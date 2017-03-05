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
	int *final_blue;
	int cols, rows, sstr_len;
	char str[1000], recv_str[1000];
	int msg_len, store;
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
	final_blue = new int[rows * cols];

	if(rank == 0) 
	{
		cout << "Enter the message to be hidden: ";
		cin >> str;
		msg_len = strlen(str);
		sstr_len = msg_len / size;

		// Storing blue values in a single dimension array.
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				Vec3b bgrPixel = img.at<Vec3b>(i, j);
				img_blue[k] = bgrPixel.val[0];
				k++;
			}
		}

		k = 0;
		flag = 0;
		store = msg_len;
		// Burn the size of the message to be written at the end of the image. 
		// This is used to extract how many bytes were written (say n) - from the encoded image.
		// Using this, we can decode the first n/size values from each row
		for (i = rows - 1; i >= 0 && !flag; i--) {
			for (j = cols - 1; j >= 0 && !flag; j--) {
				img.at<Vec3b>(i, j).val[0] = ((img.at<Vec3b>(i, j).val[0] >> 2) << 2) | (store & 0x3);
				store = store >> 2;
				k += 2;
				if (k == 16) {
					flag = 1;
				}
			}
		}
	}
	MPI_Bcast(&sstr_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Scatter blue value array - 4 * number of bytes to be written by each process
	MPI_Scatter(img_blue, sstr_len * 4, MPI_INT, recv_img_blue, sstr_len * 4, MPI_INT, 0, MPI_COMM_WORLD);

	// Scatter string - number of bytes written by each process
	MPI_Scatter(str, sstr_len, MPI_CHAR, recv_str, sstr_len, MPI_CHAR, 0, MPI_COMM_WORLD);

	for (i = 0; i < sstr_len && recv_str[i] != '\0'; i++) {
		char curr = recv_str[i];
		for (j = 0; j < sizeof(char) * 4; j++) {
			int pos = (4 * i) + j;
			recv_img_blue[pos] = ((recv_img_blue[pos] >> 2) << 2) | (curr & 0x3);
			curr = curr >> 2;
		}
	}

	MPI_Gather(recv_img_blue, sstr_len * 4, MPI_INT, final_blue, sstr_len * 4, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0) {

		int off = 0;
		k = 0;
		for (i = 0; i < msg_len * 4; i++, off++) {
			if (off >= cols) {
				off = 0;
				k++;
			}
			img.at<Vec3b>(k, off) = final_blue[i];
		}
		namedWindow("Saved File", WINDOW_AUTOSIZE);
		imshow("Saved File", img);
		imwrite(argv[2], img);
		waitKey(0);
	}

	MPI_Finalize();
	return 0;
}