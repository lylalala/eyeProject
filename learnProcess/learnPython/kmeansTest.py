import numpy as np
import copy as cp
#from scipy.cluster.vq import vq, kmeans, whiten
from sklearn.cluster import KMeans
import cv2
#coding=utf-8

#def ColorImageKmeansBinarySegmentation(colorImg,efficient):
#    tempColorImg=cp.deepcopy(colorImg)
#    tempColorImg[:,:,0]=tempColorImg[:,:,0]*efficient[0]
#    tempColorImg[:,:,1]=tempColorImg[:,:,1]*efficient[1]
#    tempColorImg[:,:,2]=tempColorImg[:,:,2]*efficient[2]
def fillHoleInBinary(Img,binaryImg):
    contours,hierarchy=cv2.findContours(binaryImg,cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)
    if len(contours)!=0:
        for i in xrange(0,len(contours)):
            if len(contours[i])<100:
                cv2.drawContours(Img,contours,i,(0,0,0),-1)
    else:
        print 'no edge is detected!'
    return Img

if __name__=="__main__":
    Img=cv2.imread('/Users/ly/Code/picAfter/eye48_2.jpg')
    print Img
    print Img.shape
    print type(Img)
    cv2.imshow('Img',Img)
    cv2.waitKey()
    grayImg=cv2.cvtColor(Img,cv2.COLOR_BGR2GRAY)
    cv2.imshow('grayImg',grayImg)
    print type(grayImg)
    print type(grayImg[0][0])
    print grayImg.shape
    print '-'*40
    print grayImg
    cv2.waitKey()
    ret,binaryImg=cv2.threshold(grayImg,70,255,cv2.THRESH_BINARY)
    cv2.imshow('binaryImg',binaryImg)
    cv2.waitKey()
    noHoleImg=fillHoleInBinary(Img,binaryImg)
    cv2.imshow('noHoleImg',noHoleImg)
    cv2.waitKey()
    #whitened=whiten(noHoleImg)
    #cv2.imshow('whitened',whitened)
    #cv2.waitKey()
    #l1=whitened[:,:,0]
    #l2=whitened[:,:,1]
    #l3=whitened[:,:,2]
    l1=noHoleImg[:,:,0]
    l2=noHoleImg[:,:,1]
    l3=noHoleImg[:,:,2]
    l1=np.reshape(l1.T,(l1.shape[0]*l1.shape[1],1))
    l2=np.reshape(l2.T,(l2.shape[0]*l2.shape[1],1))
    l3=np.reshape(l3.T,(l3.shape[0]*l3.shape[1],1))
    x=np.arange(Img.shape[0])
    y=np.arange(Img.shape[1])
    x=np.tile(x,Img.shape[1])
    y=y.repeat(Img.shape[0])
    x=np.reshape(x,(x.shape[0],1))
    y=np.reshape(y,(y.shape[0],1))
    data=np.hstack((l1,l2,l3,x,y))
    print data.shape
    kmeans=KMeans(n_clusters=2,random_state=0).fit(data)
    labels=kmeans.predict(data)
    #print type(labels)
    print labels.shape
    #print labels
    labels=np.reshape(labels,(Img.shape[1],Img.shape[0])).T
    #print labels.shape
    #labels=labels*255
    #labels=255-labels
    #labels=np.uint8(labels)
    #cv2.imshow('kmenas',labels)
    #cv2.waitKey()
    #print type(labels)
    #print type(labels[0][0])
    #print labels