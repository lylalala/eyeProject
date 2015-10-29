import numpy as np
import copy as cp
import cv2

#def ColorImageKmeansBinarySegmentation(colorImg,efficient):
#    tempColorImg=cp.deepcopy(colorImg)
#    tempColorImg[:,:,0]=tempColorImg[:,:,0]*efficient[0]
#    tempColorImg[:,:,1]=tempColorImg[:,:,1]*efficient[1]
#    tempColorImg[:,:,2]=tempColorImg[:,:,2]*efficient[2]

def fillHoleInBinary(Img,binaryImg):
    print '-'*40
    contours,hierarchy=cv2.findContours(binaryImg,cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)
    print type(contours)    #list
    print len(contours[0])     #3
    print contours
    print type(hierarchy)   #numpy.ndarray
    print hierarchy.shape   #(1,3,4)
    print '-'*40
    #cv2.drawContours(Img,contours,-1,(0,0,255),3)
    #print '-'*40
    #print type(contours)
    #cv2.imshow('img',Img)
    #cv2.waitKey()
    if len(contours)!=0:
        for i in xrange(0,len(contours)):
            if len(contours[i])<100:
                cv2.drawContours(Img,contours,i,(0,0,0),-1)
                #cv2.imshow('img',Img)
                #cv2.waitKey()
    else:
        print 'no edge is detected!'
    return Img


if __name__=="__main__":
    #colorImg=cv2.imread()
    #grayImg=cv2.cvtColor(colorImg,cv2.COLOR_BGR2GRAY)
    #ret,threshImg=cv2.threshold(grayImg,127,255,cv2.THRESH_BINARY)
    #fillHoleInBinary(threshImg)
    Img=cv2.imread('/Users/ly/Code/picAfter/eye48_2.jpg')
    cv2.imshow('Img',Img)
    print Img.shape
    cv2.waitKey()
    grayImg=cv2.cvtColor(Img,cv2.COLOR_BGR2GRAY)
    #Img=cv2.imread('/Users/ly/Code/aaa.png')
    #grayImg=cv2.cvtColor(Img,cv2.COLOR_BGR2GRAY)
    cv2.imshow('grayImg',grayImg)
    print grayImg.shape
    cv2.waitKey()
    ret,binaryImg=cv2.threshold(grayImg,70,255,cv2.THRESH_BINARY)
    cv2.imshow('binaryImg',binaryImg)
    print binaryImg.shape
    cv2.waitKey()
    #print Img.shape
    noHoleImg=fillHoleInBinary(Img,binaryImg)
    cv2.imshow('noHoleImg',noHoleImg)
    cv2.waitKey()