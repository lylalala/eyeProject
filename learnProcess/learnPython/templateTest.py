import numpy as np
import cv2
import copy as cp

diskStrelKernel = np.array([[0,1,0],[1,1,1],[0,1,1]],dtype=np.uint8)

def allKindsOfPattern(ringWidth,diskSize):
    global splitRate
    #splitRate=[2,3,4,5,6,7,8]
    global splitNum
    splitNum=20
    ringTemplate =[]
    index=[]
    for i in xrange(0,len(diskSize)):
        ring=np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        #for j in xrange(1,ringWidth+1):
            #ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        cv2.circle(ring,(np.int32(diskSize[i]),np.int32(diskSize[i])),np.int32(diskSize[i]),1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ringTemplateTop=[]
        indexTop=[]
        splitRate=np.arange(0,np.int64(diskSize[i]))
        #for j in xrange(0,len(splitRate)):
        for j in splitRate:
            ringTop=cp.deepcopy(ring)
            ringTop[0:j,:] = 0
            ringTemplateBottom=[]
            indexBottom=[]
            print 'i=%d,j=%d'%(i,j)
            #for k in xrange(0,len(splitRate)):
            for k in splitRate:
                ringBottom=cp.deepcopy(ringTop)
                ringBottom[2*np.int64(diskSize[i])+1-k:2*np.int64(diskSize[i])+1,:]=0
                print 'i=%d,j=%d,k=%d'%(i,j,k)
                #ringTemplate.append(ringBottom)
                indexBottom.append([i,j,k])
                ringTemplateBottom.append(ringBottom)
            indexTop.append(indexBottom)
            ringTemplateTop.append(ringTemplateBottom)
        index.append(indexTop)
        ringTemplate.append(ringTemplateTop)
    return ringTemplate,index

if __name__=="__main__":
    width=1;
    disk=[24.,25.,26.,27.,28.,29.,30.,31.,32.,33.,34.,35.,36.]
    temp,index=allKindsOfPattern(width,disk)
    #for i in xrange(0,len(temp)):
        #cv2.imshow('ts',temp[i]*255)
        #cv2.waitKey(1000)
        #print 'i=%d'%i
        #print index[i]
    for i in xrange(0,len(temp)):
        for j in xrange(0,len(temp[i])):
            for k in xrange(0,len(temp[i][j])):
                print 'i=%d,j=%d,k=%d'%(i,j,k)
                cv2.imshow('template',temp[i][j][k]*255)
                cv2.waitKey()
