#compare normal and encoded image

import cv2
matnew = cv2.imread("./newimg.bmp", 1)
mat = cv2.imread("./testimg.bmp", 1)

# print matnew - mat
for i in range(len(matnew)):
	for j in range(len(matnew[i])):
		if(mat[i][j][0] > matnew[i][j][0]): 
			print i, j, mat[i][j][0] - matnew[i][j][0]
		elif mat[i][j][0] < matnew[i][j][0]:
			print i, j, matnew[i][j][0] - mat[i][j][0]