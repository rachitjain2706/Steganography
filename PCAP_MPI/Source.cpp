#include "mpi.h"
#include <iostream>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
	int rank, size;
	
	// Read file specified in argument
	Mat img;
	cout << argv[1] << endl;
	img = imread(argv[1], 1);
	namedWindow("Rachit", WINDOW_AUTOSIZE);
	imshow("Rachit", img);
	waitKey(0);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("Rank: %s\n", argv[1]);
	MPI_Finalize();
}