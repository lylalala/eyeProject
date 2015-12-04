//
//  SampleOpenCVRoute.m
//  GazeEstimationOnCocoa
//
//  Created by JiangZhiping on 15/11/4.
//  Copyright © 2015年 JiangZhiping. All rights reserved.
//

#import "OpenCVProcess_DirectPass.h"
#import "string.h"
#import "ASMGazeTracker.h"
#import <vector>
using namespace std;
using namespace cv;

//definition
#define _DEBUG
#define left "1"
#define right "2"
#define OLDtemplate "0"
#define NEWtemplate "1"
#define upShelterRate 3
#define bottomShelterRate 3
#define stableFrame 50
#define imageWorkingWidth 150

int irisRange[]={24,35};
int maskLength=30;
int kernel[3][3]={{0,1,0},{1,1,1},{0,1,1}};
int rangeSize;
vector<cv::Mat> templateImg;
vector<float> irisRadiiLog;

//============================================================================================
@implementation OpenCVProcess_DirectPass {
    //NSString
    string userProfilePath,cameraProfilePath;
    ASM_Gaze_Tracker pupilTracker;
    int frameCount;
    Mat leftEyeRectified,rightEyeRectified,rectifiedImage;
    cv::Point_<int> leyecenterInRect,reyecenterInRect;
    int Lradius,Rradius;
    Mat im,orgin;
    int videoFrameRotation;
    float zoomRatio;
    float displayZoomRatio;
}

-(instancetype) init {
    self = [super init];
    if(self) {
        userProfilePath="/Users/ly/eyeProject/current_headOld/test/user_profile/liuyang";
        cameraProfilePath="/Users/ly/eyeProject/current_headOld/test/camera_calibration/iSight/CM_iSight.yaml";
        pupilTracker.initialize((userProfilePath + "/trackermodel.yaml"),cameraProfilePath);
        frameCount=0;
        //初始化模板
        [self generateTemplateSet:templateImg];
        videoFrameRotation = 0;
        zoomRatio = 1.0f;
        displayZoomRatio = 1.0f;
    }
    return self;
}

//============================================================================================
+(string) int2str:(const int)int_temp{
    stringstream stream;
    stream<<int_temp;
    return stream.str();
}

+(vector<float>) normFit:(vector<float>)numList{
    //返回均值和方差
    int average = 0;
    int listSum=0;
    double squareSum = 0;//平方和
    for(int i=0;i<numList.size();i++)
        listSum+=numList.at(i);
    average=listSum/(numList.size()-1);
    
    for(int i=0;i<numList.size();i++){
        int cha = abs(numList.at(i)-average);
        squareSum+=pow((double)cha,2);        //离差平方和
    }
    squareSum=squareSum/(numList.size()-1);
    vector<float> res;
    res.push_back(average);
    res.push_back(sqrt(squareSum));
    return res;
}

//============================================================================================
-(void) generateTemplateSet:(vector<cv::Mat>&)temp{
    vector<int> firstIrisRange;
    for(int i=irisRange[0];i<=irisRange[1];i++)
        firstIrisRange.push_back(i);
    rangeSize=int(firstIrisRange.size());
    [self newTemplate:temp IrisRange:firstIrisRange newOrOld:OLDtemplate];
}

-(void) regulateIrisRange:(vector<cv::Mat>&)temp{
    temp.clear();
    //vector<float> data(normFit(irisRadiiLog));
    vector<float> data([OpenCVProcess_DirectPass normFit:irisRadiiLog]);
    int small=data[0]-1.5*data[1];
    int big=data[0]+1.5*data[1];
    vector<int> secondIrisRange;
    for(int i=small;i<=big;i++)
        secondIrisRange.push_back(i);
    rangeSize=int(secondIrisRange.size());
    irisRange[0]=small;
    irisRange[1]=big;
    [self newTemplate:temp IrisRange:secondIrisRange newOrOld:NEWtemplate];
}

-(void) newTemplate:(vector<Mat>&)temp IrisRange:(vector<int>) theIrisRange newOrOld:(string) oldOrNew{
#ifdef _DEBUG
    //0. 输出所有的模板
    string path="/Users/ly/eyeProject/current_headOld/debugpic/template/";
#endif
    //用矩阵作为核来膨胀
    Mat diskStrelKernel=getStructuringElement(MORPH_RECT, cv::Size(3,3));
    temp.clear();
    for(int i=0;i<theIrisRange.size();i++){
        Mat ring=Mat::zeros(theIrisRange[i]*2+1, theIrisRange[i]*2+1, CV_8UC1);
        circle(ring, cv::Point(theIrisRange[i],theIrisRange[i]),theIrisRange[i], Scalar::all(1));
        dilate(ring, ring, diskStrelKernel);
        for(int j=0;j<float(ring.rows)/upShelterRate;j++)
            ring.row(j)^=ring.row(j);
        for(int j=ring.rows*(bottomShelterRate-1)/bottomShelterRate;j<ring.rows;j++)
            ring.row(j)^=ring.row(j);
        ring.convertTo(ring,  CV_32FC1);
#ifdef _DEBUG
        //0. 输出所有的模板
        string iStr=[OpenCVProcess_DirectPass int2str:i];
        imwrite(path+oldOrNew+"/eye"+iStr+".jpg", ring*255*(theIrisRange[i]*2+1)*(theIrisRange[i]*2+1));
#endif
        temp.push_back(ring);
    }

}

//============================================================================================
-(vector<int>) findMax:(vector<cv::Mat>&)imgVector maxDiff:(cv::Mat&)imgMax maxPosition:(cv::Mat&)imgMaxPosition whichFrame:(string)countStr whichEye:(string)numStr{
    //cout<<numStr<<endl;
    int pos=0;
    double maxNumThis=0;
    cv::Point maxPos;
    cv::Point maxPosThis;
    double maxNum=maxNumThis;
    minMaxLoc(imgVector[0], NULL, &maxNumThis, NULL, &maxPosThis);
    //cout<<"0:"<<maxPosThis<<" "<<"maxNumThis"<<maxNumThis<<" maxNum"<<endl;
    for(int i=1;i<imgVector.size();i++){
        
        minMaxLoc(imgVector[i], NULL, &maxNumThis, NULL, &maxPosThis);
        //cout<<i<<":"<<maxPosThis<<" "<<"maxNumThis"<<maxNumThis<<" maxNum"<<endl;
        if(maxNumThis>=maxNum){
            maxNum=maxNumThis;
            pos=i;
            maxPos=maxPosThis;
        }
    }
    //cout<<"maxPos:"<<maxPos<<endl;
    
#ifdef _DEBUG
    //5. 输出响应图像滤波后的值
    //imwrite("/Users/ly/eyeProject/current_headOld/debugpic/5Guass/eye"+countStr+"_"+numStr+".jpg", dstmatArray);
#endif
    
    vector<int> res;
    res.push_back(pos);
    res.push_back(maxPos.x);
    res.push_back(maxPos.y);
    return res;
}

-(vector<int>) DaugmanIrisCore:(cv::Mat&)img templateCircle:(vector<cv::Mat>)temp whichFrame:(string)frameCountStr whichEye:(string)leftOrRight{
    //img=imread("/Users/ly/eyeProject/current_headOld/debugpic/lalala/eye.jpg",0);
    vector<Mat> resVector;
    //进行卷积
    Mat resTempConvolve;
    img.convertTo(img, CV_32FC1);
    for(int i=0;i<temp.size();i++){
        filter2D(img, resTempConvolve, -1, temp[i]);
        
        //3. 卷积后的图片
        double maxVal=0.0;
        double *maxValue=&maxVal;
        minMaxIdx(resTempConvolve, NULL, maxValue);
        Mat resTemp=(resTempConvolve/maxVal)*255;
#ifdef _DEBUG
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/3convolve/eye"+frameCountStr+"_"+leftOrRight+"_"+leftOrRight+".jpg", resTemp);
#endif
        
        resVector.push_back(resTemp);
    }
    //生成掩码
    Mat mask=Mat::ones(resVector[0].rows, resVector[0].cols, CV_32FC1);
    for(int i=0;i<maskLength;i++)
        mask.row(i)^=mask.row(i);
    for(int i=mask.rows-maskLength;i<mask.rows;i++)
        mask.row(i)^=mask.row(i);
    for(int i=0;i<maskLength;i++)
        mask.col(i)^=mask.col(i);
    for(int i=mask.cols-maskLength;i<mask.cols;i++)
        mask.col(i)^=mask.col(i);
    
    //进行微分
    for(int i=0;i<temp.size()-1;i++){
        resVector[i]=abs(resVector[i+1]-resVector[i]);
        resVector[i]=resVector[i].mul(mask);
        //GaussianBlur(resVector[i], resVector[i], Size(0,0), 6.0);
        
#ifdef _DEBUG
        //4. 微分后的图片
        double maxVal=0.0;
        double *maxValue=&maxVal;
        minMaxIdx(resVector[i], NULL, maxValue);
        Mat resTemp=(resVector[i]/maxVal)*255;
        string iStr=[OpenCVProcess_DirectPass int2str:i];
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/4diff/eye"+frameCountStr+"_"+leftOrRight+"_"+iStr+".jpg", resTemp);
#endif
        
    }
    resVector.pop_back();
    //均值滤波
    float filterArray[]={0.2,0.2,0.2,0.2,0.2};
    Mat filterTemp=Mat(1,5,CV_32FC1,filterArray);
    for(int i=0;i<resVector.size();i++){
        filter2D(resVector[i], resVector[i], -1, filterTemp);
    }
    //求取微分最大值和位置
    Mat covstackdiffmax(resVector[0]);
    Mat covstackdiffmaxPos(resVector[0]);
    vector<int> res([self findMax:resVector maxDiff:covstackdiffmax maxPosition:covstackdiffmaxPos whichFrame:frameCountStr whichEye:leftOrRight]);
    return res;
}

//============================================================================================

-(Mat&) equalizeRGBImage:(Mat&) Img{
    Mat ycrcb;
    cvtColor(Img, ycrcb, COLOR_BGR2YCrCb);
    vector<Mat> src;
    split(ycrcb,src);
    equalizeHist(src[0],src[0]);
    merge(src,ycrcb);
    cvtColor(ycrcb, Img, COLOR_YCrCb2BGR);
    return Img;
}

-(vector<Mat>) preProcessing:(Mat)img whichFrame:(string)frameCountStr whichEye:(string)leftOrRight{
    //直方图均衡
    Mat imgeq=[self equalizeRGBImage:img];
    
    //仿射变换
    Mat imlarge;
    resize(imgeq,imlarge,cv::Size(imageWorkingWidth,imageWorkingWidth*img.rows/img.cols),0,0,CV_INTER_LINEAR);
    //边缘滤波
    Mat imlargeFilter;
    bilateralFilter(imlarge,imlargeFilter,5,150,50);
    
#ifdef _DEBUG
    //2. 输出放大并且滤波后的图像
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/2bigAndFilter/eye"+frameCountStr+"_"+leftOrRight+".jpg", imlargeFilter);
#endif
    
    //灰度处理
    Mat gray;
    cvtColor(imlargeFilter,gray,COLOR_BGR2GRAY);
    
    vector<Mat> res;
    res.push_back(gray);
    res.push_back(imlargeFilter);
    return res;
}


//============================================================================================

-(vector<float>)DaugmanIrisLocalization:(cv::Mat)eyeRectified templateCircle:(vector<Mat>)temp whichFrame:(string)frameCountStr whichEye:(string)leftOrRight{
#ifdef _DEBUG
    string whicheye=(leftOrRight==left)?"left ":"right ";
    cout<<"DaugmanIrisLocalization: "<<"It is the "<<frameCountStr<<"th frame and the "<<whicheye<<"eye!"<<endl;
#endif
    
    //图片说明：
    //1.eyeRectified 表示原图
    //2.srcImg[0] 表示放大后的灰度图
    //3.srcImg[1] 表示放大后的图
    
    //先后存放灰度图和放大后的图
    vector<Mat> srcImg([self preProcessing:eyeRectified whichFrame:frameCountStr whichEye:leftOrRight]);
    
    
    //先后存放微分最大值和对应半径值
    vector<int> srcIris([self DaugmanIrisCore:srcImg[0] templateCircle:temp whichFrame:frameCountStr whichEye:leftOrRight]);
    int radiusSrc=irisRange[0]+srcIris[0];
    
    cv::Point centerSrc(srcIris[1],srcIris[2]-2);
    
#ifdef _DEBUG
    Mat resImg=srcImg[1];
    circle(resImg, cv::Point(srcIris[1],srcIris[2]), 1, Scalar(0,0,255));
    circle(resImg, cv::Point(srcIris[1],srcIris[2]), irisRange[0]+srcIris[0], Scalar(0,0,255));
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/res/eye"+frameCountStr+"_"+leftOrRight+".jpg", resImg);
#endif
    
    //前50帧进行记录
    if(frameCount<=stableFrame){
        irisRadiiLog.push_back(radiusSrc);
    }
    
    //仿射变换还原
    int radius=radiusSrc*eyeRectified.cols/imageWorkingWidth;
    cv::Point center=cv::Point(float(centerSrc.x)*eyeRectified.cols/imageWorkingWidth,float(centerSrc.y)*eyeRectified.cols/imageWorkingWidth);
    
#ifdef _DEBUG
    //6. 还原后的图片和眼球位置
    circle(eyeRectified, center, 1, Scalar(0,0,255));
    circle(eyeRectified, center, radius, Scalar(0,0,255));
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/6res/eye"+frameCountStr+"_"+leftOrRight+".jpg", eyeRectified);
#endif
    
    
    vector<float> res;
    res.push_back(center.x);
    res.push_back(center.y-2);
    res.push_back(radius);
    return res;

}

//============================================================================================

-(void) opencvProcessFrom:(const cv::Mat &)fromMat To:(cv::Mat &)toMat {
    string frameStr=[OpenCVProcess_DirectPass int2str:frameCount];
    orgin=fromMat;
    imageOrientationFix(orgin, videoFrameRotation);
    imresize(orgin,zoomRatio,im);
    bool succeeded = pupilTracker.featureTracking(im);
    //bool succeeded=true;
    if(succeeded){
        pupilTracker.estimateFacePose();
        pupilTracker.extractEyeRectifiedImages();
        cout<<"frameCount:"<<frameCount<<endl;
        leftEyeRectified=pupilTracker.leftEyeImageRectified;
        rightEyeRectified=pupilTracker.rightEyeImageRectified;
        rectifiedImage=pupilTracker.rectifiedImage;
    
#ifdef _DEBUG
        //1. 输出原始图像
        //imshow("pupilTracker.leftEyeImageRectified.left", leftEyeRectified);
        //imshow("pupilTracker.leftEyeImageRectified.right", rightEyeRectified);
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/1EyeRectified/eye_"+frameStr+"_"+left+".jpg", leftEyeRectified);
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/1EyeRectified/eye_"+frameStr+"_"+right+".jpg", rightEyeRectified);
#endif
        
        if(frameCount==stableFrame){
            cout<<"regulate the irisRange!"<<endl;
            //regulateIrisRange(templateImg);
            [self regulateIrisRange:templateImg];
        }
        
        string frameCountStr=[OpenCVProcess_DirectPass int2str:frameCount];
        vector<float> leftEye([self DaugmanIrisLocalization:leftEyeRectified templateCircle:templateImg whichFrame:frameCountStr whichEye:left]);
        vector<float> rightEye([self DaugmanIrisLocalization:rightEyeRectified templateCircle:templateImg whichFrame:frameCountStr whichEye:right]);
        leyecenterInRect=cv::Point(leftEye[0],leftEye[1]);
        reyecenterInRect=cv::Point(rightEye[0],rightEye[1]);
        Lradius=leftEye[2];
        Rradius=rightEye[2];
        
        Point_<int> leyecenterInRectified=leyecenterInRect+pupilTracker.leftEyeRectInRectified.tl();
        Point_<int> reyecenterInRectified=reyecenterInRect+pupilTracker.rightEyeRectInRectified.tl();
        Point2f leyecenterInRectified2f(leyecenterInRectified.x,leyecenterInRectified.y);
        Point2f reyecenterInRectified2f(reyecenterInRectified.x,reyecenterInRectified.y);
        vector<Point2f> leyecenterfinal,reyecenterfinal;
        vector<Point2f> leyecenterInRectified2fVec,reyecenterInRectified2fVec;
        leyecenterInRectified2fVec.push_back(leyecenterInRectified2f);
        reyecenterInRectified2fVec.push_back(reyecenterInRectified2f);
        if(pupilTracker.isImageRectificationUsingHomoTransformation){
            perspectiveTransform(leyecenterInRectified2fVec, leyecenterfinal, pupilTracker.rectifyTransformationInv);
            perspectiveTransform(reyecenterInRectified2fVec, reyecenterfinal, pupilTracker.rectifyTransformationInv);
        }else{
            leyecenterInRectified2fVec[0]=Point2f(fromMat.cols-leyecenterInRectified2fVec[0].x, leyecenterInRectified2fVec[0].y);
            reyecenterInRectified2fVec[0]=Point2f(fromMat.cols-reyecenterInRectified2fVec[0].x,reyecenterInRectified2fVec[0].y);
            transform(leyecenterInRectified2fVec, leyecenterfinal, pupilTracker.rectifyTransformationInv);
            transform(reyecenterInRectified2fVec, reyecenterfinal, pupilTracker.rectifyTransformationInv);
        }
        
        cv::circle(im, leyecenterfinal[0], 1, Scalar(0,0,255));
        cv::circle(im, leyecenterfinal[0], Lradius, Scalar(0,0,255));
        cv::circle(im, reyecenterfinal[0], 1, Scalar(0,0,255));
        cv::circle(im, reyecenterfinal[0], Rradius, Scalar(0,0,255));
        imresize(im, displayZoomRatio, im);
        toMat=im;
    }else{
        toMat=fromMat;
        return;
    }
    frameCount++;
}
@end
