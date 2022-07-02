#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


//�w�b�_�[���i�[�p�\����
typedef struct
{
	char pnm[256];
	char comment[256];
	int width, height;
	int max;
}HEADER;

//RGB�i�[�p�\���� 3�o�C�g
typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
}RGB;

//�摜�f�[�^�i�[�p�\����
typedef struct
{
	unsigned char** gray;
	unsigned char** gray_temp1;
	unsigned char** gray_temp2;
}DATA;

//�O���[�X�P�[���ϊ������֐��@(RGB�f�[�^�i�[�p�\����(�|�C���^), �w�b�_�[���\����(�|�C���^), �f�[�^�i�[�p�\����(�|�C���^)�j
void rgbtogray(RGB(*rgb)[256], HEADER* header, DATA* data)
{
	for (int i = 0; i < header->height; i++)
	{
		for (int j = 0; j < header->width; j++)
		{
			data->gray[i][j] = data->gray_temp1[i][j] = data->gray_temp2[i][j] = (unsigned char)(int)(rgb[i][j].red * 0.299 + rgb[i][j].green * 0.587 + rgb[i][j].blue * 0.114);
		}
	}
}

//�摜�t�@�C���ǂݍ��݊֐� (�w�b�_�[���\����(�|�C���^), ���̓t�@�C����(�|�C���^), RGB�f�[�^�i�[�p�\����(�|�C���^), �f�[�^�i�[�p�\����(�|�C���^))
void load_image(HEADER* header, char* file_name, RGB(*rgb)[256], DATA* data)
{
	FILE* fp;

	if ((fopen_s(&fp, file_name, "rb")) != 0)
	{//�t�@�C�����J���̂Ɏ��s�����ꍇ�̏���
		fprintf(stderr, "%s�̃I�[�v���Ɏ��s���܂���\n", file_name);
		exit(-1);
	}

	fgets(header->pnm, 100, fp);		//PNM�`���̎擾
	fgets(header->comment, 100, fp);	//�R�����g�̎擾
	fscanf_s(fp, "%d %d", &header->width, &header->height);	//�c��, �����̎擾
	fscanf_s(fp, "%d\n", &header->max);	//�ő�P�x�l�̎擾

	//�O���[�X�P�[���ϊ���ƕϊ���f�[�^�ێ����2�����z��̍쐬
	data->gray = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	data->gray_temp1 = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	data->gray_temp2 = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	for (int i = 0; i < header->height; i++)
	{
		data->gray[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
		data->gray_temp1[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
		data->gray_temp2[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
	}

	//P6�`����P5�`���̊m�F�iP6�Ȃ�O���[�X�P�[���ϊ�, P5�̓f�[�^�擾�̂݁j
	if (strcmp("P6\n", header->pnm) == 0)
	{//P6�̏ꍇ
		fread(rgb, sizeof(RGB), header->width * header->height, fp);
		rgbtogray((&rgb)[0], header, data);
	}
	else
	{//P5�̏ꍇ
		for (int i = 0; i < header->width; i++)
		{
			for (int j = 0; j < header->height; j++)
			{
				data->gray[i][j] = data->gray_temp1[i][j] = data->gray_temp2[i][j] = fgetc(fp);;
			}
		}
	}
	fclose(fp);
}

//pgm�����o���֐� (�w�b�_�[���, �t�@�C����, �����o���f�[�^)
void write_pgm(HEADER* header, char file_name[256], unsigned char** data)
{
	FILE* output;
	fopen_s(&output, file_name, "wb");		//�o�͐�̍쐬

	fprintf(output, "P5\n");										//PNM�`���̏�������
	fprintf(output, "%s", header->comment);							//�R�����g�̏�������
	fprintf(output, "%d %d\n", header->width, header->height);		//����, �c���̏�������
	fprintf(output, "%d\n", header->max);							//���邳�̍ő�l�̏�������

	//�o�͐�ɂP�o�C�g���f�[�^����������
	for (int i = 0; i < header->height; i++)
	{
		for (int j = 0; j < header->width; j++)
		{
			fputc(data[i][j], output);
		}
	}

	fclose(output);
}

//Roberts�t�B���^�����֐��i�w�b�_�[���\����(�|�C���^), �f�[�^�i�[�p�\����(�|�C���^))
void roberts(HEADER* header, DATA* data)
{
	double sum_w = 0, sum_h = 0, temp;
	int width[3][3] = { { 0, 0, 0},
						{ 0, 1, 0},
						{ 0, 0,-1} };
	int height[3][3] = { { 0, 0, 0},
						 { 0, 0, 1},
						 { 0,-1, 0} };

	for (int i = 1; i < header->height - 1; i++)
	{
		for (int j = 1; j < header->width - 1; j++)
		{
			for (int k = -1; k < 2; k++)
			{//3x3��f�̃f�[�^���擾
				for (int l = -1; l < 2; l++)
				{
					sum_w += data->gray[i + k][j + l] * width[k + 1][l + 1];
					sum_h += data->gray[i + k][j + l] * height[k + 1][l + 1];
				}
			}
			temp = sqrt(pow(sum_w, 2.0) + pow(sum_h, 2.0));

			if (temp > 255)
			{
				temp = 255;
			}
			else if (temp < 0)
			{//0�ȉ��̏ꍇ
				temp = (int)-temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

//Prewitt�t�B���^�����֐��i�w�b�_�[���\����(�|�C���^), �f�[�^�i�[�p�\����(�|�C���^))
void prewitt(HEADER* header, DATA* data)
{
	double sum_w = 0, sum_h = 0, temp;
	int width[3][3] = { {-1, 0, 1},
						{-1, 0, 1},
						{-1, 0, 1} };
	int height[3][3] = { {-1,-1,-1},
						 { 0, 0, 0},
						 { 1, 1, 1} };

	for (int i = 1; i < header->height - 1; i++)
	{
		for (int j = 1; j < header->width - 1; j++)
		{
			for (int k = -1; k < 2; k++)
			{//3x3��f�̃f�[�^���擾
				for (int l = -1; l < 2; l++)
				{
					sum_w += data->gray[i + k][j + l] * width[k + 1][l + 1];
					sum_h += data->gray[i + k][j + l] * height[k + 1][l + 1];
				}
			}
			temp = sqrt(pow(sum_w, 2.0) + pow(sum_h, 2.0));

			if (temp > 255)
			{
				temp = 255;
			}
			else if (temp < 0)
			{//0�ȉ��̏ꍇ
				temp = -temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

//Sobel�t�B���^�����֐��i�w�b�_�[���\����(�|�C���^), �f�[�^�i�[�p�\����(�|�C���^))
void sobel(HEADER* header, DATA* data)
{
	double sum_w = 0, sum_h = 0, temp;
	int width[3][3] = { {-1, 0, 1},
						{-2, 0, 2},
						{-1, 0, 1} };
	int height[3][3] = { {-1,-2,-1},
						 { 0, 0, 0},
						 { 1, 2, 1} };

	for (int i = 1; i < header->height - 1; i++)
	{
		for (int j = 1; j < header->width - 1; j++)
		{
			for (int k = -1; k < 2; k++)
			{//3x3��f�̃f�[�^���擾
				for (int l = -1; l < 2; l++)
				{
					sum_w += data->gray[i + k][j + l] * width[k + 1][l + 1];
					sum_h += data->gray[i + k][j + l] * height[k + 1][l + 1];
				}
			}
			temp = sqrt(pow(sum_w, 2.0) + pow(sum_h, 2.0));

			if (temp > 255)
			{
				temp = 255;
			}
			else if (temp < 0)
			{//0�ȉ��̏ꍇ
				temp = -temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

int main(void)
{
	HEADER header;		//�w�b�_�[���i�[�p�\����
	DATA data;			//�o�C�i���f�[�^�i�[�p�\����
	RGB rgb[256][256];	//RGB���ێ��p2�����\����

	char file_name[256];

	printf("�t�@�C��������́F");
	scanf_s("%s", &file_name, 255);

	//�摜�ǂݍ���
	load_image(&header, &file_name[0], (&rgb)[0], &data);

	//Roberts�t�B���^�̏���
	roberts(&header, &data);
	write_pgm(&header, "roberts.pgm", (&data.gray_temp1)[0]);
	
	//Prewitt�t�B���^�̏���
	prewitt(&header, &data);
	write_pgm(&header, "prewitt.pgm", (&data.gray_temp1)[0]);

	//Sobel�t�B���^�̏���
	sobel(&header, &data);
	write_pgm(&header, "sobel.pgm", (&data.gray_temp1)[0]);

	return 0;
}