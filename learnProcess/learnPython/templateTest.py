import numpy as np
import cv2
import copy as cp


def allKindsOfPattern(ringWidth,diskSize):
    global splitRate
    splitRate=[2,3,4,5,6,7,8]
    global splitNum
    splitNum=20
    ringTemplate =[]
    index=[]
    for i in xrange(0,len(diskSize)):
        ring=np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        #for j in xrange(1,ringWidth+1):
            #ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        cv2.circle(ring,(np.int32(diskSize[i]),np.int32(diskSize[i])),np.int32(diskSize[i]),1)
        ringTemplateTop=[]
        #print 'i=%d'%i
        #print ring
        for j in xrange(0,len(splitRate)):
            ringTop=cp.deepcopy(ring)
            ringTop[0:round(ringTop.shape[0]*splitRate[j]/splitNum):,:] = 0
            ringTemplateBottom=[]
            print 'i=%d,j=%d'%(i,j)
            #print ringTop
            #print ring
            #cv2.imshow('test',ringTop*255)
            #cv2.waitKey(3000)
            for k in xrange(0,len(splitRate)):
                ringBottom=cp.deepcopy(ringTop)
                ringBottom[round(ringBottom.shape[0]*(splitNum-splitRate[k])/20):,:]=0
                print 'i=%d,j=%d,k=%d'%(i,j,k)
                #cv2.imshow('test',ringBottom*255)
                #cv2.waitKey(1000)
                ringTemplate.append(ringBottom)
                index.append([i,j,k])
        #       ringTemplateBottom.append(ringBottom)
#       ringTemplateTop.append(ringTemplateBottom)
#   ringTemplate.append(ringTemplateTop)
    return ringTemplate,index

if __name__=="__main__":
    width=1;
    disk=[24.]
    temp,index=allKindsOfPattern(width,disk)
    for i in xrange(0,len(temp)):
        cv2.imshow('ts',temp[i]*255)
        cv2.waitKey(1000)
        print 'i=%d'%i
        print index[i]
