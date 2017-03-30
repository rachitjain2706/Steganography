#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace std;
using namespace cv;

__global__ void hideIt(int *d_blue, char *d_str, int *d_recv_blue, int tot) {
	int block = blockIdx.x;
	int sstr_len = blockDim.x;
	int id = threadIdx.x;

	int cur = (block * sstr_len) + id;
	char c = d_str[cur];

	if (cur >= tot) {
		return;
	}

	int start = 4 * cur;

	int j = 0;
	d_recv_blue[start + j] = 0;
	for (j = 0; j < 4; j++) {
		d_recv_blue[start + j] = ((d_blue[start + j] >> 2) << 2) | (c & 0x3);
		c = c >> 2;	
	}
	

}

int main() {
	int size = 4, k = 0, i, j, flag;
	int *img_blue;
	int *recv_img_blue;
	int cols, rows, sstr_len;
	char str[1000], recv_str[1000];
	int msg_len, store;
	Mat img;

	int *d_blue, *d_recv_blue;
	char *d_str;

	img = imread("..\\mani.bmp", 1);
	cols = img.cols;
	rows = img.rows;

	img_blue = new int[rows * cols];
	recv_img_blue = new int[cols * rows];

	cout << "Enter the message to be hidden: ";
	cin.getline(str, 1000);
	msg_len = strlen(str);
	sstr_len = ceil((float)msg_len / size);

	k = 0;
	flag = 0;
	store = msg_len;

	cout << store;

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

	k = 0;

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

	cudaMalloc((void **)&d_blue, rows * cols * sizeof(int));
	cudaMalloc((void **)&d_str, strlen(str) * sizeof(char));
	cudaMalloc((void **)&d_recv_blue, msg_len * 4 * sizeof(int));

	cudaMemcpy(d_blue, img_blue, rows * cols * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(d_str, str, strlen(str) * sizeof(char), cudaMemcpyHostToDevice);

	hideIt <<<size, sstr_len>>> (d_blue, d_str, d_recv_blue, msg_len);

	cudaMemcpy(recv_img_blue, d_recv_blue, msg_len * 4 * sizeof(int), cudaMemcpyDeviceToHost);

	int r = 0;
	int c = 0;
	for (i = 0; i < msg_len * 4; i++, c++) {
		if (c > cols) {
			r++;
			c = 0;
		}
		img.at<Vec3b>(r, c).val[0] = recv_img_blue[i];
	}

	imwrite("..//maniout.bmp", img);
	imshow("Output", img);
	waitKey(0);
	cout << "Done!" << endl;
}