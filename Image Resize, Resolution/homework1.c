#include "homework1.h"

int num_threads;
int resolution;

void initialize(image *im) {
	// set number of threads
	omp_set_num_threads(num_threads);
	int i, j;
	// initialize matrix
	strcpy(im -> p, GreyScale);
	im -> width = resolution;
	im -> height = resolution;
	im -> maxval = MAX;
	im -> map = (unsigned char***) malloc (sizeof(unsigned char **) * resolution);
	#pragma omp parallel for private(j)
	for(i = 0; i < resolution; i++) {
		im -> map[i] = (unsigned char**) malloc (sizeof(unsigned char*) * resolution);
		for(j = 0; j < resolution; j++) 
			im -> map[i][j] = (unsigned char*) malloc (sizeof(unsigned char));
	}
}
// equation used for distance from a point to a line
int equation(int x, int y) {
	double eq = abs(-1 * x + 2 * y + 0)/sqrt(1 + 4);
	if(-THICK <= eq && eq <= THICK)
		return 0;
	return MAX;
}

void render(image *im) {
	// set number of threads
	omp_set_num_threads(num_threads);
	int i, j;
	// division is used for scaling the picture to 100 logical pixels
	int division = Logic < resolution ? resolution/Logic : Logic/resolution;
	if(resolution > Logic){
		#pragma omp parallel for private(j)
		for(i = 0; i < resolution; i++)
			for(j = 0; j < resolution; j++)
				// calculate distance from each pixel and color it
				im -> map[i][j][0] = equation(i / division, j / division);
	}
	else {
		#pragma omp parallel for private(j)
		for(i = 0; i < resolution; i++)
			for(j = 0; j < resolution; j++)
				// calculate distance from each pixel and color it
				im -> map[i][j][0] = equation(i * division, j * division);
	}

}

void writeData(const char * fileName, image *img) {
	// set number of threads
	omp_set_num_threads(num_threads);
	int i, j;
	ptr_write = fopen(fileName, "wb");
	// write type, size, value
	fprintf(ptr_write, "%s\n%d %d\n%d\n", img -> p, img -> width, img -> height, img -> maxval);
	// initialize buffer and write pixels
	buffer = (unsigned char*) malloc (sizeof(unsigned char) * img -> height * img -> width);
	#pragma omp parallel for private(i)
	for(j = resolution - 1; j >= 0; j--)
		for(i = 0; i < resolution; i++) 
		{
			buffer[(i+ (resolution -1 - j)* resolution)] = img -> map[i][j][0];
		}
	fwrite(buffer, sizeof(unsigned char), resolution * resolution, ptr_write);
	fclose(ptr_write);
}

