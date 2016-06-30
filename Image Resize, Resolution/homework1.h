#ifndef _HOMEWORK1_HEADER
#define _HOMEWORK1_HEADER

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define MAX 255
#define GreyScale "P5"
#define Logic 100
#define THICK 3
FILE *ptr_write;
unsigned char* buffer;

typedef struct {
	char p[2];
	int width, height, maxval;
	unsigned char*** map;
}image;

#endif