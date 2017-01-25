//#include <iostream>

#include <ctype.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <setjmp.h>
#include <png.h>
#include <assert.h>
#include "imageUtils.h"

#define PPM_MAGIC_NUMBER "P6"

bool savePNGImage(const char *fileName, const Image *img, png_byte *rows[]);
void readToken(FILE *fp, char *token);


bool pngRead(const char *fileName, Image *img)
{
	png_byte **pngImage = NULL;
	png_structp png;
	png_infop   info;
	FILE *fp;
	png_byte header[20];
	png_uint_32 w, h;
	int i;
	int bitDepth, colorType, interlaceType;
	int compressionType, filterMethod;

	assert(img);
	img->imgData = NULL;
	img->width = 0;
	img->height = 0;

	fp = fopen(fileName, "rb");
	if (!fp)
	{
		fprintf(stderr, "Could not open PNG file %s.\n", fileName);
		return false;
	}
	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8))
		return false;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info = png_create_info_struct(png);

	/* Default error handling */
	if (setjmp(png_jmpbuf(png))) {
		fclose(fp);
		png_destroy_write_struct(&png, &info);
		return false;
	}

	png_init_io(png, fp);
	// we have already read 8 bytes of the header
	png_set_sig_bytes(png, 8);
	png_read_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
	pngImage = png_get_rows(png, info);
	png_get_IHDR(png, info, &w, &h, &bitDepth, &colorType,
		&interlaceType, &compressionType, &filterMethod);

	if (!png_get_gAMA(png, info, &img->gamma))
		img->gamma = 1.0;

	img->width = (int)w;
	img->height = (int)h;

	if (bitDepth != 8)
	{
		fprintf(stderr, "pngRead: currently only 8 Bit per channel are "
			"supported.\n");
		fclose(fp);
		return false;
	}

	switch (colorType)
	{
	case PNG_COLOR_TYPE_GRAY:
		img->channel = 1;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		img->channel = 2;
		break;
	case PNG_COLOR_TYPE_RGB:
		img->channel = 3;
		break;
	case PNG_COLOR_TYPE_RGBA:
		img->channel = 4;
		break;
	default:
		fprintf(stderr, "pngRead: invalid color type"
			"(not grayscale, grayscale with alpha, RGB or RGBA)\n");
		fclose(fp);
		return false;
	}

	//*data = (unsigned char*) malloc((int)w*(int)h* *channel);
	img->imgData = new unsigned char[img->channel * img->width * img->height];

	for (i = 0; i<(int)h; ++i)
	{
		memcpy((img->imgData + i*(int)w* img->channel), pngImage[i], (int)w* img->channel);
		free(pngImage[i]);
		pngImage[i] = NULL;
	}

	fclose(fp);
	free(pngImage);

	return true;
}


bool pngWrite(const char *fileName, const Image *img, bool invert)
{
	bool retVal;
	int i;
	png_byte **pngImage = NULL;

	if ((img->channel < 1) || (img->channel > 4))
		return false;

	pngImage = (png_byte**)malloc(img->height * sizeof(png_byte*));
	//pngImage = new png_byte*[img->height];

	if (invert)
		for (i = 0; i<img->height; ++i)
			pngImage[i] =
			(png_byte *)(img->imgData + (img->height - 1 - i) * img->width*img->channel);
	else
		for (i = 0; i<img->height; ++i)
			pngImage[i] = (png_byte *)(img->imgData + i*img->width * img->channel);

	retVal = savePNGImage(fileName, img, pngImage);

	free(pngImage);
	//delete [] pngImage;

	return retVal;
}


bool ppmRead(const char *fileName, Image *img)
{
	char token[100];
	int dataSize;
	FILE *fp;

	if (!(fp = fopen(fileName, "rb"))) {
		fprintf(stderr, "Could not open PPM file \"%s\".\n", fileName);
		return false;
	}

	/* Read magic number */
	readToken(fp, token);
	if (strcmp(PPM_MAGIC_NUMBER, token)) {
		fprintf(stderr, "PPM file \"%s\":\ninvalid magic number: %s\n",
			fileName, token);
		return false;
	}

	/* Read dimensions */
	readToken(fp, token);
	if (sscanf(token, "%i", &img->width) != 1) {
		fprintf(stderr, "PPM file \"%s\":\ninvalid width: %s\n",
			fileName, token);
		return false;
	}
	readToken(fp, token);
	if (sscanf(token, "%i", &img->height) != 1) {
		fprintf(stderr, "PPM file \"%s\":\ninvalid height: %s\n",
			fileName, token);
		return false;
	}

	/* Allocate memory for image data */
	dataSize = img->width * img->height * 3;
	img->imgData = new unsigned char[dataSize];
	//if (! (ppmFile->data = (unsigned char *)malloc(dataSize))) {
	if (!img->imgData)
	{
		fprintf(stderr, "PPM file \"%s\":\nnot enough memory for image data\n",
			fileName);
		return false;
	}

	/* Read maximum value */
	readToken(fp, token);
	if (sscanf(token, "%i", &img->maxVal) != 1) {
		fprintf(stderr, "PPM file \"%s\":\ninvalid maximum value: %s\n",
			fileName, token);
		return false;
	}

	/* Read image data */
	if (fread(img->imgData, dataSize, 1, fp) != 1) {
		fprintf(stderr, "PPM file \"%s\":\nreading image data failed\n",
			fileName);
		return false;
	}

	fclose(fp);
	return true;
}


bool ppmWrite(const char *fileName, const Image *img)
{
	int i, dataSize, maxVal;
	FILE *fp;

	fp = fopen(fileName, "wb");
	if (!fp)
	{
		fprintf(stderr, "PPM file \"%s\":\nopening image file failed\n",
			fileName);
		return false;
	}

	/* Write magic number */
	fprintf(fp, "%s\n", PPM_MAGIC_NUMBER);

	/* Write dimensions */
	fprintf(fp, "%i %i\n", img->width, img->height);

	/* Write maximum value */
	dataSize = img->width * img->height * 3;
	maxVal = 0;
	for (i = 0; i < dataSize; i++) {
		if (img->imgData[i] > maxVal) {
			maxVal = img->imgData[i];
		}
	}
	fprintf(fp, "%i\n", maxVal);

	/* Write image data */
	if (fwrite(img->imgData, dataSize, 1, fp) != 1) {
		fprintf(stderr, "PPM file \"%s\":\nwriting image data failed\n",
			fileName);
		return false;
	}

	fclose(fp);
	return true;
}


bool savePNGImage(const char *fileName, const Image *img, png_byte *rows[])
{
	FILE *fp;
	int imgType;
	png_structp png;
	png_infop   info;

	fp = fopen(fileName, "wb");
	if (!fp)
		return false;

	switch (img->channel)
	{
	case 1:
		imgType = PNG_COLOR_TYPE_GRAY;
		break;
	case 2:
		imgType = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		imgType = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
		imgType = PNG_COLOR_TYPE_RGBA;
		break;
	default:
		fclose(fp);
		return false;
	}

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png)
	{
		fclose(fp);
		return false;
	}
	info = png_create_info_struct(png);
	if (!info)
	{
		fclose(fp);
		png_destroy_write_struct(&png, NULL);
		return false;
	}

	/* Default error handling */
	if (setjmp(png_jmpbuf(png))) {
		fclose(fp);
		png_destroy_write_struct(&png, &info);
		return false;
	}
	png_init_io(png, fp);
	png_set_IHDR(png, info, img->width, img->height, 8, imgType,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);
	png_set_gAMA(png, info, img->gamma);
	png_write_info(png, info);
	png_write_image(png, (png_byte **)rows);
	png_write_end(png, info);
	png_destroy_write_struct(&png, &info);

	fclose(fp);
	return true;
}


void readToken(FILE *fp, char *token)
{
	int comment = 0;
	char c;

	do
	{
		if (fread(&c, 1, 1, fp) != 1)
		{
			perror("reading token failed");
			exit(1);
		}
		if (c == '\n')
			comment = 0;
		else if (c == '#')
			comment = 1;
	} while (isspace(c) || comment);

	*token++ = c;

	do
	{
		if (fread(&c, 1, 1, fp) != 1)
		{
			perror("reading token failed");
			exit(1);
		}
		*token++ = c;
	} while (!isspace(c));

	*--token = '\0';
}
