#include "ASMGazeTracker.h"
#include "PoseEstimatorParameterTuner.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"

//#define _DEBUG
#define left 1
#define right 2
#define imageWorkingWidth 150
#define coutImg 1
#define irisRingWidth 1
#define kernelRow 3
#define kernelCol 3
#define upShelterRate 3
#define bottomShelterRate 3
#define stableFrame 50
#define OLDtemplate 0
#define NEWtemplate 1

int irisRange[]={24,35};
int rangeSize;
int kernel[3][3]={{0,1,0},{1,1,1},{0,1,1}};
int maskLength=30;

vector<float> irisRadiiLog;

//======================================================================================================
//--------------工具函数-------------------------
string int2str(const int &int_temp)
{
    stringstream stream;
    stream<<int_temp;
    return stream.str();   //此处也可以用 stream>>string_temp
}

vector<float> normFit(vector<float> numList){
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

//--------------模板生成函数----------------------
void newTemplate(vector<Mat>& temp, vector<int> irisRange,int oldOrNew){
    //用矩阵作为核来膨胀
    Mat diskStrelKernel=getStructuringElement(MORPH_RECT, Size(3,3));
    temp.clear();
    for(int i=0;i<irisRange.size();i++){
        Mat ring=Mat::zeros(irisRange[i]*2+1, irisRange[i]*2+1, CV_8UC1);
        circle(ring, Point(irisRange[i],irisRange[i]),irisRange[i], Scalar::all(1));
        dilate(ring, ring, diskStrelKernel);
        for(int j=0;j<float(ring.rows)/upShelterRate;j++)
            ring.row(j)^=ring.row(j);
        for(int j=ring.rows*(bottomShelterRate-1)/bottomShelterRate;j<ring.rows;j++)
            ring.row(j)^=ring.row(j);
        ring.convertTo(ring,  CV_32FC1);
        //ring=ring/((irisRange[i]*2+1)*(irisRange[i]*2+1));
        temp.push_back(ring);
    }
    
#ifdef _DEBUG
    //0. 输出所有的模板
    string path="/Users/ly/eyeProject/current_headOld/debugpic/template/";
    for(int i=0;i<temp.size();i++)
        imwrite(path+int2str(oldOrNew)+"/eye"+int2str(i)+".jpg", temp[i]*255*(irisRange[i]*2+1)*(irisRange[i]*2+1));
#endif
    
}

void generateTemplateSet(vector<Mat>& temp){
    vector<int> firstIrisRange;
    for(int i=irisRange[0];i<=irisRange[1];i++)
        firstIrisRange.push_back(i);
    rangeSize=firstIrisRange.size();
    newTemplate(temp,firstIrisRange,OLDtemplate);
}

void regulateIrisRange(vector<Mat>& temp){
    temp.clear();
    vector<float> data(normFit(irisRadiiLog));
    int small=data[0]-1.5*data[1];
    int big=data[0]+1.5*data[1];
    vector<int> secondIrisRange;
    for(int i=small;i<=big;i++)
        secondIrisRange.push_back(i);
    rangeSize=secondIrisRange.size();
    irisRange[0]=small;
    irisRange[1]=big;
    newTemplate(temp,secondIrisRange,NEWtemplate);
}

//======================================================================================================
//-----------------卷积过程-------------------------------------------------------
vector<int> findMax(vector<Mat>& imgVector, Mat& imgMax, Mat& imgMaxPosition,string countStr,string numStr){
    //cout<<numStr<<endl;
    int pos=0;
    double maxNumThis=0;
    Point maxPos;
    Point maxPosThis;
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

vector<int> DaugmanIrisCore(Mat& img, vector<Mat>& temp, string framecount, string leftOrRight){
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
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/3convolve/eye"+framecount+"_"+leftOrRight+"_"+int2str(i)+".jpg", resTemp);
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
        imwrite("/Users/ly/eyeProject/current_headOld/debugpic/4diff/eye"+framecount+"_"+leftOrRight+"_"+int2str(i)+".jpg", resTemp);
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
    vector<int> res(findMax(resVector, covstackdiffmax, covstackdiffmaxPos,framecount,leftOrRight));
    return res;
}

//======================================================================================================
//----------------------------图像预处理过程------------------------------------------------------------
Mat& equalizeRGBImage(Mat &Img){
    Mat ycrcb;
    cvtColor(Img, ycrcb, COLOR_BGR2YCrCb);
    vector<Mat> src;
    split(ycrcb,src);
    equalizeHist(src[0],src[0]);
    merge(src,ycrcb);
    cvtColor(ycrcb, Img, COLOR_YCrCb2BGR);
    return Img;
}

vector<Mat> preProcessing(Mat img, string countStr, string numStr){
    //直方图均衡
    Mat imgeq=equalizeRGBImage(img);
    
    //仿射变换
    Mat imlarge;
    resize(imgeq,imlarge,Size(imageWorkingWidth,imageWorkingWidth*img.rows/img.cols),0,0,CV_INTER_LINEAR);
    //边缘滤波
    Mat imlargeFilter;
    bilateralFilter(imlarge,imlargeFilter,5,150,50);
    
#ifdef _DEBUG
    //2. 输出放大并且滤波后的图像
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/bigAndFilter/eye"+countStr+"_"+numStr+".jpg", imlargeFilter);
    //imshow("pic_"+countStr+"_"+numStr,imlargeFilter);
#endif
    
    //灰度处理
    Mat gray;
    cvtColor(imlargeFilter,gray,COLOR_BGR2GRAY);
    
    vector<Mat> res;
    res.push_back(gray);
    res.push_back(imlargeFilter);
    return res;
}

//======================================================================================================
//----------------------核心函数------------------------------------
vector<float> DaugmanIrisLocalization(Mat eyeRectified, vector<Mat> temp, int frameCount, int leftOrRight){
#ifdef _DEBUG
    string whicheye=(leftOrRight==left)?"left ":"right ";
    cout<<"DaugmanIrisLocalization: "<<"It is the "<<frameCount<<"th frame and the "<<whicheye<<"eye!"<<endl;
#endif
    
    //图片说明：
    //1.eyeRectified 表示原图
    //2.srcImg[0] 表示放大后的灰度图
    //3.srcImg[1] 表示放大后的图
    
    //先后存放灰度图和放大后的图
    vector<Mat> srcImg(preProcessing(eyeRectified,int2str(frameCount),int2str(leftOrRight)));
    
    //先后存放微分最大值和对应半径值
    vector<int> srcIris(DaugmanIrisCore(srcImg[0],temp,int2str(frameCount),int2str(leftOrRight)));
    int radiusSrc=irisRange[0]+srcIris[0];
    Point centerSrc(srcIris[1],srcIris[2]-2);
    
#ifdef _DEBUG
    Mat resImg=srcImg[1];
    circle(resImg, Point(srcIris[1],srcIris[2]), 1, Scalar(0,0,255));
    circle(resImg, Point(srcIris[1],srcIris[2]), irisRange[0]+srcIris[0], Scalar(0,0,255));
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/res/eye"+int2str(frameCount)+"_"+int2str(leftOrRight)+".jpg", resImg);
    //imshow("Before"+int2str(leftOrRight),resImg);
#endif
    
    //前50帧进行记录
    if(frameCount<=stableFrame){
        irisRadiiLog.push_back(radiusSrc);
    }
    
    //仿射变换还原
    int radius=radiusSrc*eyeRectified.cols/imageWorkingWidth;
    Point center=Point(float(centerSrc.x)*eyeRectified.cols/imageWorkingWidth,float(centerSrc.y)*eyeRectified.cols/imageWorkingWidth);
    vector<float> res;
    
#ifdef _DEBUG
    //6. 还原后的图片和眼球位置
    circle(eyeRectified, center, 1, Scalar(0,0,255));
    circle(eyeRectified, center, radius, Scalar(0,0,255));
    imwrite("/Users/ly/eyeProject/current_headOld/debugpic/6res/eye"+int2str(frameCount)+"_"+int2str(leftOrRight)+".jpg", eyeRectified);
    imshow("After"+int2str(leftOrRight), eyeRectified);
#endif
    
    res.push_back(center.x);
    res.push_back(center.y);
    res.push_back(radius);
    return res;
}


int main(int argc, const char * argv[])
{
    string windowName;
    VideoCapture cam;
    bool dumpFile = false;
    bool extractEyeImages = true;
    bool noShow = false;
    string userProfilePath,cameraProfilePath,inputFilePath;
    int videoFrameRotation = 0;
    windowName="Pupil tracking from camera!";
    cam.open(0);
    userProfilePath="/Users/ly/eyeProject/current_headOld/test/user_profile/liuyang";
    cameraProfilePath="/Users/ly/eyeProject/current_headOld/test/camera_calibration/iSight/CM_iSight.yaml";
    extractEyeImages = false;
    int frameCount = 0;
    
    //--------------------------------------------------------------------------------
    //some defination
    vector<cv::Mat> templateImg;
    generateTemplateSet(templateImg);
    //生成模板
    
    //--------------------------------------------------------------------------------
    
    if(!cam.isOpened()){
        cout<<"cannot open camera or video file."<<endl;
        return 0;
    }
    
    ASM_Gaze_Tracker pupilTracker;
    pupilTracker.initialize((userProfilePath + "/trackermodel.yaml"),cameraProfilePath);
    windowName += ( " by profile ["+userProfilePath+"]");
    
    namedWindow(windowName);
    ParameterTuner tuner(pupilTracker);
    tuner.mainWindowName = windowName;
    
    vector<Point3f> faceCrdRefVecs;
    faceCrdRefVecs.push_back(Point3f(0,0,0));
    faceCrdRefVecs.push_back(Point3f(50,0,0));
    faceCrdRefVecs.push_back(Point3f(0,50,0));
    faceCrdRefVecs.push_back(Point3f(0,0,50));
    
    Mat origin, im ;
    float zoomRatio = 1.0f;
    float displayZoomRatio = 1.0f;
    CSVFileWriter csvlogger;
    CSVFileWriter parseLogger;
    LowpassFPSTimer timer(20);
    Mat leftEyeRectified,rightEyeRectified,rectifiedImage;
    Point_<int> leyecenterInRect,reyecenterInRect;
    int Lradius,Rradius;
    
    
    while(true){
        timer.tick();
        bool success = captureImage(cam, origin);
        if (success == false) {
            break;
        }
        
        imageOrientationFix(origin,videoFrameRotation);
        imresize(origin,zoomRatio,im);
        bool succeeded = pupilTracker.featureTracking(im);
        if (succeeded) {
            pupilTracker.estimateFacePose();
            pupilTracker.extractEyeRectifiedImages();
            //            pupilTracker.estimatePupilCenterFaceCoordinates();
            //------------------------------------------------------------------------------------------------
            cout<<"frameCount:"<<frameCount<<endl;
            //均是bgr图片
            leftEyeRectified=pupilTracker.leftEyeImageRectified;
            rightEyeRectified=pupilTracker.rightEyeImageRectified;
            rectifiedImage=pupilTracker.rectifiedImage;
            
#ifdef _DEBUG
            //1. 输出原始图像
            //imshow("pupilTracker.leftEyeImageRectified.left", leftEyeRectified);
            //imshow("pupilTracker.leftEyeImageRectified.right", rightEyeRectified);
            imwrite("/Users/ly/eyeProject/current_headOld/debugpic/1EyeRectified/eye"+int2str(frameCount)+"_"+int2str(0)+".jpg", leftEyeRectified);
            imwrite("/Users/ly/eyeProject/current_headOld/debugpic/1EyeRectified/eye"+int2str(frameCount)+"_"+int2str(0)+".jpg", rightEyeRectified);
#endif
            
            if(frameCount==stableFrame){
                //
                cout<<"regulate the irisRange!"<<endl;
                regulateIrisRange(templateImg);
            }
            vector<float> leftEye(DaugmanIrisLocalization(leftEyeRectified,templateImg,frameCount,left));
            vector<float> rightEye(DaugmanIrisLocalization(rightEyeRectified,templateImg,frameCount,right));
            leyecenterInRect=Point(leftEye[0],leftEye[1]);
            reyecenterInRect=Point(rightEye[0],rightEye[1]);
            Lradius=leftEye[2];
            Rradius=rightEye[2];
            
#ifdef _DEBUG
            circle(leftEyeRectified, leyecenterInRect, 1, Scalar(0,0,255));
            circle(leftEyeRectified, leyecenterInRect, Lradius, Scalar(0,0,255));
            //circle(rightEyeRectified, reyecenterInRect, 1, Scalar(0,0,255));
            //circle(rightEyeRectified, reyecenterInRect, Rradius, Scalar(0,0,255));
            //imshow("left",leftEyeRectified);
            //imshow("right",rightEyeRectified);
#endif
            //------------------------------------------------------------------------------------------------
        }
        else{
            continue;
        }
        
        /*if (extractEyeImages) {
         fs::path basePath = inputFilePath.parent_path() / inputFilePath.stem().string();
         if (fs::exists(basePath)== false) {
         fs::create_directories(basePath);
         }
         vector<float> slot;
         slot.push_back((pupilTracker.isImageRectificationUsingHomoTransformation? 1.0f : 0.0f));
         slot.push_back(pupilTracker.leftEyeRectInRectified.tl().x);
         slot.push_back(pupilTracker.leftEyeRectInRectified.tl().y);
         Mat temp = pupilTracker.rectifyTransformationInv.reshape(1,1);
         for (int i = 0 ;i< temp.cols; i++) {
         slot.push_back((float)temp.at<double>(i));
         }
         parseLogger.addSlot(slot);
         fs::path leyePath = basePath / ("L"+boost::lexical_cast<string>(frameCount)+".jpg");
         fs::path reyePath = basePath / ("R"+boost::lexical_cast<string>(frameCount)+".jpg");
         imwrite(leyePath.string(), pupilTracker.leftEyeImageRectified);
         imwrite(reyePath.string(), pupilTracker.rightEyeImageRectified);
         }
         
         if (dumpFile)
         csvlogger.addSlot(pupilTracker.toDataSlot());
         
         printf("\b\rfps: %f, frame count: %d",1.0/timer.tock(), ++frameCount);
         fflush(stdout);*/
        
        if (noShow) {
            continue;
        }
        if (succeeded) {
            //将定位出得眼球中心和轮廓circle到每一帧中
            /*int tlx=pupilTracker.leftEyeRectInRectified.tl().x;
             int tly=pupilTracker.leftEyeRectInRectified.tl().y;
             int width=pupilTracker.rightEyeRectInRectified.width;
             int length=pupilTracker.rightEyeRectInRectified.height;
             
             vector<Point2f> srcPoint,dstPoint;
             circle(im, Point(tlx,tly), 2, Scalar(0,0,255));
             circle(im, Point(tlx+width,tly), 2, Scalar(0,0,255));
             circle(im, Point(tlx,tly+length), 2, Scalar(0,0,255));
             circle(im, Point(tlx+width,tly+length), 2, Scalar(0,0,255));
             srcPoint.push_back(Point2f(tlx,tly));
             srcPoint.push_back(Point2f(tlx+width,tly));
             srcPoint.push_back(Point2f(tlx,tly+length));
             srcPoint.push_back(Point2f(tlx+width,tly+length));
             transform(srcPoint, dstPoint, pupilTracker.rectifyTransformationInv);
             circle(im, dstPoint[0], 2, Scalar(0,255,0));
             circle(im, dstPoint[1], 2, Scalar(0,255,0));
             circle(im, dstPoint[2], 2, Scalar(0,255,0));
             circle(im, dstPoint[3], 2, Scalar(0,255,0));*/
            
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
                leyecenterInRectified2fVec[0]=Point2f(im.cols-leyecenterInRectified2fVec[0].x, leyecenterInRectified2fVec[0].y);
                reyecenterInRectified2fVec[0]=Point2f(im.cols-reyecenterInRectified2fVec[0].x,reyecenterInRectified2fVec[0].y);
                transform(leyecenterInRectified2fVec, leyecenterfinal, pupilTracker.rectifyTransformationInv);
                transform(reyecenterInRectified2fVec, reyecenterfinal, pupilTracker.rectifyTransformationInv);
            }
            
            circle(im, leyecenterfinal[0], 1, Scalar(0,0,255));
            circle(im, leyecenterfinal[0], Lradius, Scalar(0,0,255));
            circle(im, reyecenterfinal[0], 1, Scalar(0,0,255));
            circle(im, reyecenterfinal[0], Rradius, Scalar(0,0,255));
            //circle(im, Point2d(500,500), Rradius, Scalar(0,0,255));
            
            drawPoints(im, pupilTracker.allTrackedPoints2D);
            //drawPoints(im, pupilTracker.reprojectedSelectedPoints2D,Scalar(0,255,0));
            //circle(im, pupilTracker.leftEyePoint, 3, Scalar(0,255,0),-1);
            //circle(im, pupilTracker.rightEyePoint, 3, Scalar(0,255,0),-1);
            //vector<Point2f> reprjCrdRefPts;
            //pupilTracker.projectTemplatePointsBackIntoImage2DPoints(faceCrdRefVecs, reprjCrdRefPts);
            //line(im, reprjCrdRefPts[0], reprjCrdRefPts[1], Scalar(255,0,0),2);
            //line(im, reprjCrdRefPts[0], reprjCrdRefPts[2], Scalar(0,255,0),2);
            //line(im, reprjCrdRefPts[0], reprjCrdRefPts[3], Scalar(0,0,255),2);
            //drawStringAtTopLeftCorner(im, "d:" + boost::lexical_cast<string>(pupilTracker.distanceToCamera()));
        }
        imresize(im, displayZoomRatio, im);
        imshow(windowName,im);
        int c = waitKey(1)%256;
        if(c == 'q' && ! dumpFile)
            break;
        if(c == 'd')
            pupilTracker.reDetectFace();
        if(c == '-')
            displayZoomRatio -=0.1;
        if(c == '=')
            displayZoomRatio +=0.1;
        if (c == 's') {
            tuner.showTuner();
        }
        frameCount++;
    }
    
    /*if (dumpFile) {
     csvlogger.writeToFile(inputFilePath.parent_path() / (inputFilePath.stem().string() + ".test"));
     }
     
     if (extractEyeImages) {
     fs::path basePath = inputFilePath.parent_path() / inputFilePath.stem().string();
     parseLogger.writeToFile(basePath / "formatlab.result");
     }*/
    
    
    return 0;
}
