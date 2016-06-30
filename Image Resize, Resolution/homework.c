#include "homework.h"

int num_threads;
int resize_factor;
const int GaussianKernel[3][3] = {{1, 2, 1},{2, 4, 2},{1, 2, 1}};

void readInput(const char * fileName, image *img) {
	ptr_read = fopen(fileName, "rb");
	// read type, size, value
	fscanf(ptr_read, "%s\n%d %d\n%d\n", img -> p, &img -> width, &img -> height, &img -> maxval);
	format = img -> p[1] == GreyScale ? GS : RGB;
	size = img -> height * img -> width;
	// initialize and read pixel buffer
	img -> map = (unsigned char*) malloc (sizeof(unsigned char) * size * format);
	fread(img -> map, sizeof(unsigned char), size * format, ptr_read);
	// close file
	fclose(ptr_read);
}

void writeData(const char * fileName, image *img) {
	ptr_write = fopen(fileName, "wb");
	// write type, size, value
	fprintf(ptr_write, "%s\n%d %d\n%d\n", img -> p, img -> width, img -> height, img -> maxval);
	// write pixels
	fwrite(img -> map, sizeof(unsigned char), size * format, ptr_write);
	// close file
	fclose(ptr_write);
}

void resize(image *in, image * out) {
	// set number of threads
	omp_set_num_threads(num_threads);
	int k, l, auxR, auxG, auxB, index;
	int divide = resize_factor * resize_factor;
	// initialize image out
	strcpy(out -> p, in -> p);
	out -> width = in -> width / resize_factor;
	out -> height = in -> height / resize_factor;
	out -> maxval = in -> maxval;

	size = out -> width * out -> height;
	out -> map = (unsigned char*) malloc (sizeof(unsigned char) * size * format);
	// Greyscale format
	if(format == GS) {
		// even resize factor
		if(resize_factor % 2 == 0 && resize_factor != 0) {
			#pragma omp parallel for private (i,j,k,l,auxR,index)
			for(i = 0; i < out -> height; i++) {
				for(j = 0; j < out -> width; j++) {
					auxR = auxB = auxG = 0;
					for(k = 0; k < resize_factor; k++)
						for(l = 0; l < resize_factor; l++) {
							// 1 pixel sum for a certain index in buffer
							index = (i * in -> width * resize_factor + k* in->width + j * resize_factor + l);
							auxR += in -> map[index];
						}
					// 1 pixel arithmetic mean
					out -> map[(i * out ->width + j)] = auxR / divide;
				}
			}
		}
		// resize_factor is 3
		if(resize_factor == 3) {
			#pragma omp parallel for private (i,j,k,l,auxR,index)
			for(i = 0; i < out -> height; i++) {
				for(j = 0; j < out -> width; j++) {
					auxR = auxB = auxG = 0;
					for(k = 0; k < resize_factor; k++)
						for(l = 0; l < resize_factor; l++) {
							// 1 pixel sum for a certain index in buffer
							index = (i * in -> width * resize_factor+ k* in->width + j * resize_factor + l);
							auxR += in -> map[index] * GaussianKernel[k][l];
						}
					// 1 pixel arithmetic mean
					out -> map[(i * out ->width + j)] = auxR / GaussDivision;
				}
			}
		}
	}
	// RGB format
	else {
		// resize_factor is even
		if(resize_factor % 2 == 0 && resize_factor != 0) {
			#pragma omp parallel for private (i,j,k,l,auxR,auxG,auxB,index)
			for(i = 0; i < out -> height; i++) {
				for(j = 0; j < out -> width; j++) {
					auxR = auxB = auxG = 0;
					for(k = 0; k < resize_factor; k++)
						for(l = 0; l < resize_factor; l++) {
							// RGB sum for a certain index in buffer
							index = (i * in -> width * resize_factor + k* in->width + j * resize_factor + l) * format;
							auxR += in -> map[index];
							auxG += in -> map[index + 1];
							auxB += in -> map[index + 2];
						}
					// RGB arithmetic mean
					out -> map[(i * out ->width + j) * format] = auxR / divide;
					out -> map[(i * out ->width + j) * format + 1] = auxG / divide;
					out -> map[(i * out ->width + j) * format + 2] = auxB / divide;
				}
			}
		}
		// resize_factor is 3
		if(resize_factor == 3) {
			#pragma omp parallel for private (i,j,k,l,auxR,auxG,auxB,index)
			for(i = 0; i < out -> height; i++) {
				for(j = 0; j < out -> width; j++) {
					auxR = auxB = auxG = 0;
					for(k = 0; k < resize_factor; k++)
						for(l = 0; l < resize_factor; l++) {
							// RGB sum for a certain index in buffer
							index = (i * in -> width * resize_factor+ k* in->width + j * resize_factor + l) * format;
							auxR += in -> map[index] * GaussianKernel[k][l];
							auxG += in -> map[index + 1] * GaussianKernel[k][l];
							auxB += in -> map[index + 2] * GaussianKernel[k][l];	
						}
					// RGB arithmetic mean
					out -> map[(i * out ->width + j) * format] = auxR / GaussDivision;
					out -> map[(i * out ->width + j) * format + 1] = auxG / GaussDivision;
					out -> map[(i * out ->width + j) * format + 2] = auxB / GaussDivision;
				}
			}
		}
	}

}