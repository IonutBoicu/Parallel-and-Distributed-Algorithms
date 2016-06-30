#ifndef _HOMEWORK_HEADER
#define _HOMEWORK_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RGB 3
#define GS 1
#define GreyScale '5'
#define GaussDivision 16
FILE *ptr_read, *ptr_write;
int i, j, format, size;

typedef struct {
	char p[2];
	int width, height, maxval;
	unsigned char* map;
}image;

#endif