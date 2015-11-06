#master
import numpy as np
import cv2

def mat2gray(mat):
    maxv  = mat.max()
    minv  = mat.min()
    gap = maxv - minv
    if gap==0:
        gap=1
    return (mat-minv)/gap

if __name__=="__main__":
    img=np.zeros((31,31),dtype=np.uint8)
    cv2.imshow('img',img)
    cv2.waitKey(2000)
    cv2.circle(img,(15,15),15,1)
    cv2.imshow('img',img*255)
    print img
    cv2.waitKey()
    img[round(img.shape[0]/10*7):,:]=0
    cv2.imshow('img',img*255)
    cv2.waitKey()
    img[0:round(img.shape[0]/10*3):,:] = 0
    cv2.imshow('img',img*255)
    cv2.waitKey()

