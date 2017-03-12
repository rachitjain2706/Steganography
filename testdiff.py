import cv2
mat = cv2.imread("./newimg.bmp", 1)
for i in range(0, 4):
	print mat[0][i][0]
