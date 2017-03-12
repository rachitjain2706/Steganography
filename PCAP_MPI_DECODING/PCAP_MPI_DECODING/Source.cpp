#include "mpi.h"
#include <iostream>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	int rank, size, k = 0, i, j, flag;
	int *img_blue;
	int *recv_img_blue;
	int cols, rows, sstr_len;
	int store;
	int val;
	char str_hidden[1000], final_msg[1000];
	Mat img;

	// Init MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		// Read file specified in argument
		img = imread(argv[1], 1);
		cols = img.cols;
		rows = img.rows;
	}

	MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

	img_blue = new int[rows * cols];
	recv_img_blue = new int[cols * rows];

	if (rank == 0)
	{
		// Storing blue values in a single dimension array.
		for (i = 0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
				Vec3b bgrPixel = img.at<Vec3b>(i, j);
				img_blue[k] = bgrPixel.val[0];
				k++;
			}
		}

		k = 0;
		flag = 0;
		store = 0;

		// Extract size of image
		for (i = rows - 1; i >= 0 && !flag; i--) {
			for (j = cols - 1; j >= 0 && !flag; j--) {
				val = img.at<Vec3b>(i, j).val[0] & 0x3;
				store |= val << k;
				k += 2;
				if (k == 16) {
					flag = 1;
				}
			}
		}
		sstr_len = ceil((float)store / size);
	}
	MPI_Bcast(&sstr_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Scatter blue array to 'size' processes
	MPI_Scatter(img_blue, sstr_len  * 4, MPI_INT, recv_img_blue, sstr_len * 4, MPI_INT, 0, MPI_COMM_WORLD);


	// Extract message
	for (i = 0; i < sstr_len; i++) {
		str_hidden[i] = 0;
		for (j = 0; j < sizeof(char) * 4; j++) {
			int pos = (4 * i) + j;
			char img_val = recv_img_blue[pos] & 0x3;
			str_hidden[i] |= img_val << (j*2);
		}
	}

	MPI_Gather(str_hidden, sstr_len, MPI_CHAR, final_msg, sstr_len, MPI_CHAR, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		final_msg[store] = '\0';
		cout << "Final message = " << final_msg << endl;
	}

	MPI_Finalize();
	return 0;
}