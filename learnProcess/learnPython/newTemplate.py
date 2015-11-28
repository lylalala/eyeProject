import numpy as np
import cv2
import copy as cp

def MidpointCircle(i, radius, xc, yc, value):
    
    x = 0;
    y = (radius);
    d = (1 - radius);
    i[xc, yc+y] = value;
    i[xc, yc-y] = value;
    i[xc+y, yc] = value;
    i[xc-y, yc] = value;
    
    while ( x < y - 1 ):
        x = x + 1;
        if ( d < 0 ):
            d = d + x + x + 1;
        else :
            y = y - 1;
            a = x - y + 1;
            d = d + a + a;
        i[ x+xc,  y+yc] = value;
        i[ y+xc,  x+yc] = value;
        i[ y+xc, -x+yc] = value;
        i[ x+xc, -y+yc] = value;
        i[-x+xc, -y+yc] = value;
        i[-y+xc, -x+yc] = value;
        i[-y+xc,  x+yc] = value;
        i[-x+xc,  y+yc] = value;
    return i

diskStrelKernel = np.array([[0,1,0],[1,1,1],[0,1,1]],dtype=np.uint8)

def newTemplate(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        cv2.circle(ring,(np.int32(diskSize[i]),np.int32(diskSize[i])),np.int32(diskSize[i]),1)
        #for j in xrange(1,ringWidth+1):
        #ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/4*3):,:] = 0
        #ring[0:round(ring.shape[0]/5):,:] = 0
        ring[0:round(ring.shape[0]/3):,:]=0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate

def DaugmanPatternSymmetryBrace(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        #for j in xrange(1,ringWidth+1):
        #ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        cv2.circle(ring,(np.int32(diskSize[i]),np.int32(diskSize[i])),np.int32(diskSize[i]),1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/5*4):,:] = 0
        ring[0:round(ring.shape[0]/5):,:] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate

if __name__=="__main__":
    width=1;
    disk=[24.,25.,26.,27.,28.,29.,30.,31.,32.,33.,34.,35.,36.]
    temp=newTemplate(width,disk)
    for i in xrange(0,len(temp)):
        cv2.imshow('ts',temp[i]*255)
        cv2.waitKey()
        print 'i=%d'%i
        print temp[i].shape
    cv2.imwrite('/Users/ly/MyGithub/eyeProject/learnProcess/learnPython/ring/ring.jpg',temp[1]*255)

