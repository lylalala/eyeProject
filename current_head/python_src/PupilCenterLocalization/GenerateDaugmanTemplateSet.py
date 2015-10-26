__author__ = 'Jiangzp'

import cv2
import numpy as np

diskStrelKernel = np.array([[0,1,0],[1,1,1],[0,1,1]],dtype=np.uint8)

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
        
def DaugmanPatternFullCircle(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate
    
def DaugmanPatternTopHalf(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[0:round(ring.shape[0]/2),:] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate
    
def DaugmanPatternBottomHalf(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/2):,:] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate
    
def DaugmanPatternSymmetryBrace(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/5*4):,:] = 0
        ring[0:round(ring.shape[0]/5):,:] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate
    
def DaugmanPatternLeftBrace(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/5*4):,:] = 0
        ring[0:round(ring.shape[0]/5):,:] = 0
        ring[:,round(ring.shape[1]/2):] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate
    
def DaugmanPatternRightBrace(ringWidth,diskSize):
    ringTemplate =[]
    for i in xrange(0,len(diskSize)):
        ring = np.zeros((diskSize[i]*2+1,diskSize[i]*2+1),dtype=np.uint8)
        for j in xrange(1,ringWidth+1):
            ring = MidpointCircle(ring,diskSize[i]-j+1,diskSize[i],diskSize[i],1)
        ring = cv2.dilate(ring,diskStrelKernel)
        ring[round(ring.shape[0]/5*4):,:] = 0
        ring[0:round(ring.shape[0]/5):,:] = 0
        ring[:,0:round(ring.shape[1]/2)] = 0
        ring = np.float32(ring)
        ring = ring/sum(sum(ring))
        ringTemplate.append(ring)
    return ringTemplate

def GenerateDaugmanTemplateSet(ringWidth, diskSize):
    weights = [0.15,0.15,0.15,0.45,0.1,0.1];
    funcSet = [];
    funcSet.append(DaugmanPatternFullCircle)
    funcSet.append(DaugmanPatternTopHalf)
    funcSet.append(DaugmanPatternBottomHalf)
    funcSet.append(DaugmanPatternSymmetryBrace)
    funcSet.append(DaugmanPatternLeftBrace)
    funcSet.append(DaugmanPatternRightBrace)
    results = [GenerateDaugmanTemplateSet.pool.apply_async(f,(ringWidth,diskSize)) for f in funcSet]
    parallelResults = [r.get() for r in results]
    return parallelResults,weights
#    for i in range(len(parallelResults)):
#        covstack[:,:,i] = parallelResults[i][0]
    
#    templateSet = [];
#    templateSet.append(DaugmanPatternFullCircle(ringWidth,diskSize))
#    templateSet.append(DaugmanPatternTopHalf(ringWidth,diskSize))
#    templateSet.append(DaugmanPatternBottomHalf(ringWidth,diskSize))
#    templateSet.append(DaugmanPatternSymmetryBrace(ringWidth,diskSize))
#    templateSet.append(DaugmanPatternLeftBrace(ringWidth,diskSize))
#    templateSet.append(DaugmanPatternRightBrace(ringWidth,diskSize))
#    
#    return templateSet,weights