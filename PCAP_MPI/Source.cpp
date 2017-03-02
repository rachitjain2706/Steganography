#include "mpi.h"
#include <iostream>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) 
{
	int rank, size, k = 0, i, j;
	int *img_blue;
	int *recv_img_blue;
	int cols, rows;
	char str[100];
	Mat img;

	// Init MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	

	if (rank == 0)
	{
		// Read file specified in argument
		img = imread(argv[1], 1);
		namedWindow("Rachit", WINDOW_AUTOSIZE);
		imshow("Rachit", img);
		waitKey(0);

		cols = img.cols;
		rows = img.rows;

	}

	MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

	img_blue = new int[rows * cols];
	recv_img_blue = new int[cols];	

	if(rank == 0) 
	{

		// Storing in seperate matrices for each color value
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				Vec3b bgrPixel = img.at<Vec3b>(i, j);
				img_blue[k] = bgrPixel.val[0];
				k++;
			}
		}
	}

	// Scatter arrays
	MPI_Scatter(img_blue, cols, MPI_INT, recv_img_blue, cols, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		for (i = 0; i < cols; i++)
			cout << "Lowl = " << recv_img_blue[i] << endl;
	}
	else {
		for (i = 0; i < cols; i++)
			cout << "Lulz = " << recv_img_blue[i] << endl;
	}

	MPI_Finalize();
	return 0;
}