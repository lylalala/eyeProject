import numpy as np
import copy as cp
from sklearn.cluster import KMeans
import cv2
#coding=utf-8

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
    efficient=np.array([1,0.2,0.2,0.9])
    Img=cv2.imread('/Users/ly/Code/picAfter/eye12_1.jpg')
    cv2.imshow('Img',Img)
    cv2.waitKey()
    grayImg=cv2.cvtColor(Img,cv2.COLOR_BGR2GRAY)
    ret,binaryImg=cv2.threshold(grayImg,70,255,cv2.THRESH_BINARY)
    cv2.imshow('binaryImg',binaryImg)
    cv2.waitKey()
    noHoleImg=fillHoleInBinary(Img,binaryImg)
    cv2.imshow('noHoleImg',noHoleImg)
    cv2.waitKey()
    l1=np.reshape(noHoleImg[:,:,0].T,(Img.shape[0]*Img.shape[1],1))
    l2=np.reshape(noHoleImg[:,:,1].T,(Img.shape[0]*Img.shape[1],1))
    l3=np.reshape(noHoleImg[:,:,2].T,(Img.shape[0]*Img.shape[1],1))
    x=np.reshape(np.tile(np.arange(Img.shape[0]),Img.shape[1]),(l1.shape[0],1))
    y=np.reshape(np.arange(Img.shape[1]).repeat(Img.shape[0]),(l1.shape[0],1))
    data=np.hstack((l1*efficient[0],l2*efficient[1],l3*efficient[2],x*efficient[3],y*efficient[3]))
    #the first time to operate kmeans
    kmeans=KMeans(n_clusters=2,random_state=0).fit(data)
    labels=kmeans.predict(data)
    #find the XXX
    if np.mean(data[np.where(labels==1),0:3])>np.mean(data[np.where(labels==0),0:3]):
        ti=0
    else:
        ti=1
    labels[np.where(labels!=ti)]=2
    newdata=data[np.where(labels==ti)]
    newdataIndex=np.where(labels==ti)[0]
    #the second time to operate kmeans
    kmeans=KMeans(n_clusters=2,random_state=0).fit(newdata)
    newlabels=kmeans.predict(newdata)
    if np.mean(newdata[np.where(newlabels==1),0:3])>np.mean(newdata[np.where(newlabels==0),0:3]):
        labels[newdataIndex[np.where(newlabels==1)[0]]]=0
        labels[newdataIndex[np.where(newlabels==0)[0]]]=1
    else:
        labels[newdataIndex[np.where(newlabels==0)[0]]]=0
        labels[newdataIndex[np.where(newlabels==1)[0]]]=1
    labels=np.uint8(255-np.reshape(labels,(Img.shape[1],Img.shape[0])).T*127)
    cv2.imshow('kmenas',labels)
    cv2.waitKey()











