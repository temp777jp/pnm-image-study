#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


//ヘッダー情報格納用構造体
typedef struct
{
	char pnm[256];
	char comment[256];
	int width, height;
	int max;
}HEADER;

//RGB格納用構造体 3バイト
typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
}RGB;

//画像データ格納用構造体
typedef struct
{
	unsigned char** gray;
	unsigned char** gray_temp1;
	unsigned char** gray_temp2;
}DATA;

//グレースケール変換処理関数　(RGBデータ格納用構造体(ポインタ), ヘッダー情報構造体(ポインタ), データ格納用構造体(ポインタ)）
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

//画像ファイル読み込み関数 (ヘッダー情報構造体(ポインタ), 入力ファイル名(ポインタ), RGBデータ格納用構造体(ポインタ), データ格納用構造体(ポインタ))
void load_image(HEADER* header, char* file_name, RGB(*rgb)[256], DATA* data)
{
	FILE* fp;

	if ((fopen_s(&fp, file_name, "rb")) != 0)
	{//ファイルを開くのに失敗した場合の処理
		fprintf(stderr, "%sのオープンに失敗しました\n", file_name);
		exit(-1);
	}

	fgets(header->pnm, 100, fp);		//PNM形式の取得
	fgets(header->comment, 100, fp);	//コメントの取得
	fscanf_s(fp, "%d %d", &header->width, &header->height);	//縦幅, 横幅の取得
	fscanf_s(fp, "%d\n", &header->max);	//最大輝度値の取得

	//グレースケール変換先と変換後データ保持先の2次元配列の作成
	data->gray = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	data->gray_temp1 = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	data->gray_temp2 = (unsigned char**)malloc(sizeof(unsigned char*) * header->height);
	for (int i = 0; i < header->height; i++)
	{
		data->gray[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
		data->gray_temp1[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
		data->gray_temp2[i] = (unsigned char*)malloc(sizeof(unsigned char) * header->width);
	}

	//P6形式かP5形式の確認（P6ならグレースケール変換, P5はデータ取得のみ）
	if (strcmp("P6\n", header->pnm) == 0)
	{//P6の場合
		fread(rgb, sizeof(RGB), header->width * header->height, fp);
		rgbtogray((&rgb)[0], header, data);
	}
	else
	{//P5の場合
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

//pgm書き出し関数 (ヘッダー情報, ファイル名, 書き出しデータ)
void write_pgm(HEADER* header, char file_name[256], unsigned char** data)
{
	FILE* output;
	fopen_s(&output, file_name, "wb");		//出力先の作成

	fprintf(output, "P5\n");										//PNM形式の書き込み
	fprintf(output, "%s", header->comment);							//コメントの書き込み
	fprintf(output, "%d %d\n", header->width, header->height);		//横幅, 縦幅の書き込み
	fprintf(output, "%d\n", header->max);							//明るさの最大値の書き込み

	//出力先に１バイトずつデータを書き込み
	for (int i = 0; i < header->height; i++)
	{
		for (int j = 0; j < header->width; j++)
		{
			fputc(data[i][j], output);
		}
	}

	fclose(output);
}

//Robertsフィルタ処理関数（ヘッダー情報構造体(ポインタ), データ格納用構造体(ポインタ))
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
			{//3x3画素のデータを取得
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
			{//0以下の場合
				temp = (int)-temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

//Prewittフィルタ処理関数（ヘッダー情報構造体(ポインタ), データ格納用構造体(ポインタ))
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
			{//3x3画素のデータを取得
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
			{//0以下の場合
				temp = -temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

//Sobelフィルタ処理関数（ヘッダー情報構造体(ポインタ), データ格納用構造体(ポインタ))
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
			{//3x3画素のデータを取得
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
			{//0以下の場合
				temp = -temp;
			}
			data->gray_temp1[i][j] = (int)temp;
			sum_w = sum_h = 0;
		}
	}
}

int main(void)
{
	HEADER header;		//ヘッダー情報格納用構造体
	DATA data;			//バイナリデータ格納用構造体
	RGB rgb[256][256];	//RGB情報保持用2次元構造体

	char file_name[256];

	printf("ファイル名を入力：");
	scanf_s("%s", &file_name, 255);

	//画像読み込み
	load_image(&header, &file_name[0], (&rgb)[0], &data);

	//Robertsフィルタの処理
	roberts(&header, &data);
	write_pgm(&header, "roberts.pgm", (&data.gray_temp1)[0]);
	
	//Prewittフィルタの処理
	prewitt(&header, &data);
	write_pgm(&header, "prewitt.pgm", (&data.gray_temp1)[0]);

	//Sobelフィルタの処理
	sobel(&header, &data);
	write_pgm(&header, "sobel.pgm", (&data.gray_temp1)[0]);

	return 0;
}