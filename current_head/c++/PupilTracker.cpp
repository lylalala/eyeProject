
//#include "asm_face/ft.hpp"
//#include "jzp_lib/jzplib_all.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "ASMGazeTracker.h"
#include "PoseEstimatorParameterTuner.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <cv.h>
#include <cvaux.h>
#include <ml.h>
#include <cxcore.h>
#include <string>
using namespace std;
using namespace cv;
using namespace gpu;
//add
#define left 0
#define right 1
#define imageWorkingWidth 150
#define coutImg 1
#define irisRingWidth 1
#define _DEBUG
int irisRange[]={24,25,26,27,28,29,30,31,32,33,34,35};
#define rangeSize 12
int kernel[3][3]={{0,1,0},{1,1,1},{0,1,1}};
#define kernelRow 3
#define kernelCol 3
#define upShelterRate 3
#define bottomShelterRate 4
#define stableFrame 50
vector<float> irisRadiiLog;
void regulateIrisRange(vector<Mat>& temp);
void newTemplate(vector<Mat>& temp, vector<int> irisRange);
vector<float> DaugmanIrisLocalization(Mat eyeRectified, vector<Mat> temp, int frameCount, int leftOrRight);


//--------------模板生成函数----------------------
void generateTemplateSet(vector<Mat>& temp){
    vector<int> firstIrisRange(irisRange,irisRange+rangeSize);
    newTemplate(temp,firstIrisRange);
}

void regulateIrisRange(vector<Mat>& temp){
    
}

void newTemplate(vector<Mat>& temp, vector<int> irisRange,int oldOrNew){
    //生成核
    Mat diskStrelKernel=Mat::ones(kernelRow,kernelRow,CV_8UC1);
    for(int i=0;i<kernelRow;i++)
        for(int j=0;j<kernelRow;j++)
            diskStrelKernel.at<uchar>(i,j)=kernel[i][j];
    temp.clear();
    for(int i=0;i<irisRange.size();i++){
        Mat ring=Mat::zeros(irisRange[i]*2+1, irisRange[i]*2+1, CV_8UC1);
        circle(ring, Point(irisRange[i],irisRange[i]),irisRange[i], Scalar::all(1));
        dilate(ring, ring, diskStrelKernel);
        for(int j=0;j<float(ring.rows)/upShelterRate;j++)
            ring.row(j)-=ring.row(j);
        for(int j=ring.rows*(bottomShelterRate-1)/bottomShelterRate;j<ring.rows;j++)
            ring.row(j)-=ring.row(j);
        temp.push_back(ring);
    }
    
#ifdef _DEBUG
    //输出所有的模板
    cout<<"temp::size()="<<temp.size()<<endl;
    for(int i=0;i<temp.size();i++)
        imwrite("\\Users\\ly\\eyeProject\\current_head\\debugpic\\templateOld\\"+string(1,oldOrNew+'0')+"eye"+string(1,i+'0')+".jpg", temp[i]);
#endif
}



int main(int argc, const char * argv[])
{
    string windowName;
    VideoCapture cam;
    bool dumpFile = false;
    bool extractEyeImages = true;
    bool noShow = false;
    //userProfilePath="\\Users\\ly\\eyeProject\\current_head\\test\\user_profile\\liuyang\\trackermodel.yaml";
    //cameraProfilePath="\\Users\\ly\\eyeProject\\current_head\\test\\camera_calibration\\iSight\\CM_iSight.yaml";
    fs::path userProfilePath,cameraProfilePath,inputFilePath;
    int videoFrameRotation = 0;
    /*
    if (argc<3) {
        cout<<argv[0]<<" userProfileDir"<<" cameraProfile"<<" [dumping Video file]"<<" [noshow] [ExtractEyeImages]"<<endl;
        return 0;
    } else if (argc == 3) {
        windowName = "Pupil tracking from camera";
        cam.open(0);
        userProfilePath = fs::path(argv[1]);
        cameraProfilePath = fs::path(argv[2]);
        extractEyeImages = false;
    } else if(argc == 4) {
        dumpFile = true;
        userProfilePath = fs::path(argv[1]);
        cameraProfilePath = fs::path(argv[2]);
        inputFilePath = fs::path(argv[3]);
        cam.open(inputFilePath.string());
        videoFrameRotation = readRotationMetadataForVideo(inputFilePath);
        windowName = "Pupil tracking from video [" + inputFilePath.string() +"]";
    } else if(argc == 5 && boost::iequals(string(argv[4]), "noshow")) {
        dumpFile = true;
        noShow = true;
        userProfilePath = fs::path(argv[1]);
        cameraProfilePath = fs::path(argv[2]);
        inputFilePath = fs::path(argv[3]);
        cam.open(inputFilePath.string());
        videoFrameRotation = readRotationMetadataForVideo(inputFilePath);
        windowName = "Pupil tracking from video [" + inputFilePath.string() +"]";
    } else if(argc >=5 && (boost::iequals(string(argv[4]), "ExtractEyeImage") || boost::iequals(string(argv[5]), "ExtractEyeImages"))) {
        dumpFile = true;
        extractEyeImages = true;
        userProfilePath = fs::path(argv[1]);
        cameraProfilePath = fs::path(argv[2]);
        inputFilePath = fs::path(argv[3]);
        cam.open(inputFilePath.string());
        videoFrameRotation = readRotationMetadataForVideo(inputFilePath);
        windowName = "Pupil tracking from video [" + inputFilePath.string() +"]";
    }*/
//-----------------------------------输入处理--------------------------------------------
    windowName="Pupil tracking from camera!";
    cam.open(0);
    userProfilePath="/Users/ly/eyeProject/current_head/test/user_profile/liuyang/";
    cameraProfilePath="/Users/ly/eyeProject/current_head/test/camera_calibration/iSight/CM_iSight.yaml";
    extractEyeImages = false;
//--------------------------------------------------------------------------------------
    if(!cam.isOpened()){
        cout<<"cannot open camera or video file."<<endl;
        return 0;
    }
    else{
        cout<<"has opened!"<<endl;
    }

    
    

    ASM_Gaze_Tracker pupilTracker;
    pupilTracker.initialize((userProfilePath / "trackermodel.yaml").string(),cameraProfilePath.string());
    windowName += ( " by profile ["+userProfilePath.string()+"]");
    
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
    int frameCount = 0;
//--------------------------------------------------------------------------------
    //some defination
    vector<cv::Mat> templateImg;
    generateTemplateSet(templateImg);
    //生成模板
    
    
//--------------------------------------------------------------------------------
    
    /*while(true){
        timer.tick();
        bool success = captureImage(cam, origin);
        if (success == false) {
            break;
        }
        //add
        frameCount++;
        
        imageOrientationFix(origin,videoFrameRotation);
        imresize(origin,zoomRatio,im);
        bool succeeded = pupilTracker.featureTracking(im);
        if (succeeded) {
            pupilTracker.estimateFacePose();
            pupilTracker.extractEyeRectifiedImages();
//            pupilTracker.estimatePupilCenterFaceCoordinates();
//-------------------------start:2015.11.13---------------------------------------
            //均是bgr图片
            Mat leftEyeRectified=pupilTracker.leftEyeImageRectified;
            Mat rightEyeRectified=pupilTracker.rightEyeImageRectified;
            Mat rectifiedImage=pupilTracker.rectifiedImage;*/
            
            /*if(frameCount==stableFrame){
                cout<<"regulate the irisRange!"<<endl;
            }*/
            /*vector<float> leftEye(DaugmanIrisLocalization(leftEyeRectified,templateImg,frameCount,1));
            vector<float> rightEye(DaugmanIrisLocalization(rightEyeRectified,templateImg,frameCount,2));
            
//-------------------------end:---------------------------------------
        }
        
        if (extractEyeImages) {
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
        fflush(stdout);
        
        if (noShow) {
            continue;
        }
        if (succeeded) {
            drawPoints(im, pupilTracker.allTrackedPoints2D);
            drawPoints(im, pupilTracker.reprojectedSelectedPoints2D,Scalar(0,255,0));
            circle(im, pupilTracker.leftEyePoint, 3, Scalar(0,255,0),-1);
            circle(im, pupilTracker.rightEyePoint, 3, Scalar(0,255,0),-1);
            vector<Point2f> reprjCrdRefPts;
            pupilTracker.projectTemplatePointsBackIntoImage2DPoints(faceCrdRefVecs, reprjCrdRefPts);
            line(im, reprjCrdRefPts[0], reprjCrdRefPts[1], Scalar(255,0,0),2);
            line(im, reprjCrdRefPts[0], reprjCrdRefPts[2], Scalar(0,255,0),2);
            line(im, reprjCrdRefPts[0], reprjCrdRefPts[3], Scalar(0,0,255),2);
            drawStringAtTopLeftCorner(im, "d:" + boost::lexical_cast<string>(pupilTracker.distanceToCamera()));
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
    }
    
    if (dumpFile) {
        csvlogger.writeToFile(inputFilePath.parent_path() / (inputFilePath.stem().string() + ".test"));
    }
    
    if (extractEyeImages) {
        fs::path basePath = inputFilePath.parent_path() / inputFilePath.stem().string();
        parseLogger.writeToFile(basePath / "formatlab.result");
    }*/
    
    return 0;
}

//==============================================================================================


/*
//--------------图像预处理阶段--------------------
Mat& equalizeRGBImage(Mat &Img){
    Mat ycrcb;
    cvtColor(Img, ycrcb, COLOR_BGR2YCrCb);
    vector<Mat> src;
    split(src,ycrcb);
    equalizeHist(src[0],src[0]);
    merge(src,ycrcb);
    cvtColor(ycrcb, Img, COLOR_YCrCb2BGR);
    return Img;
}

vector<Mat> preProcessing(Mat &img, string countStr, string numStr){
    //直方图均衡
    Mat imgeq=equalizeRGBImage(img);
    //仿射变换
    Mat imlarge;
    resize(imgeq,imlarge,Size(imageWorkingWidth,imageWorkingWidth/img.rows*img.cols),0,0,CV_INTER_LINEAR);
    //边缘滤波
    bilateralFilter(imlarge,imlarge,5,imageWorkingWidth,50);
    
    #ifdef _DEBUG
    imwrite("\\Users\\ly\\eyeProject\\current_head\\debugpic\\eyepic\\eye"+countStr+"_"+numStr+".jpg", imlarge);
    cout<<"write"<<endl;
    #endif
    
    //灰度处理
    Mat gray;
    cvtColor(imlarge,gray,COLOR_BGR2GRAY);
    vector<Mat> res;
    res.push_back(gray);
    res.push_back(imlarge);
    return res;
}

//-------------卷积过程------------------------
void convolveDFT(Mat& A, Mat& B, Mat& C)
{
    // reallocate the output array if needed
    C.create(abs(A.rows - B.rows)+1, abs(A.cols - B.cols)+1, A.type());
    Size dftSize;
    // compute the size of DFT transform
    dftSize.width = getOptimalDFTSize(A.cols + B.cols - 1);
    dftSize.height = getOptimalDFTSize(A.rows + B.rows - 1);
    
    // allocate temporary buffers and initialize them with 0's
    Mat tempA(dftSize, A.type(), Scalar::all(0));
    Mat tempB(dftSize, B.type(), Scalar::all(0));
    
    // copy A and B to the top-left corners of tempA and tempB, respectively
    Mat roiA(tempA, Rect(0,0,A.cols,A.rows));
    A.copyTo(roiA);
    Mat roiB(tempB, Rect(0,0,B.cols,B.rows));
    B.copyTo(roiB);
    
    // now transform the padded A & B in-place;
    // use "nonzeroRows" hint for faster processing
    dft(tempA, tempA, 0, A.rows);
    dft(tempB, tempB, 0, B.rows);
    
    // multiply the spectrums;
    // the function handles packed spectrum representations well
    mulSpectrums(tempA, tempB, tempA, DFT_COMPLEX_OUTPUT);
    
    // transform the product back from the frequency domain.
    // Even though all the result rows will be non-zero,
    // you need only the first C.rows of them, and thus you
    // pass nonzeroRows == C.rows
    dft(tempA, tempA, DFT_INVERSE + DFT_SCALE, C.rows);
    
    // now copy the result back to C.
    tempA(Rect(0, 0, C.cols, C.rows)).copyTo(C);
    
    // all the temporary buffers will be deallocated automatically
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//这是这个project中暂时写的最low的函数
void findMax(vector<Mat>& imgVector, Mat& imgMax, Mat& imgMaxPosition){
//void findMax(vector<Mat>& imgVector, Mat& imgMax, int** imgMaxPosition){
    for(int i=1;i<imgVector.size();i++){
        for(int j=0;j<imgMax.rows;j++){
            for(int k=0;k<imgMax.cols;k++){
                if(imgVector[i].at<uchar>(j,k)>=imgMax.at<uchar>(j,k)){
                    imgMax.at<uchar>(j,k)=imgVector[i].at<uchar>(j,k);
                    imgMaxPosition.at<uchar>(j,k)=i;
                }
            }
        }
    }
}

vector<Mat> DaugmanIrisCore(Mat& img, vector<Mat>& temp){
    vector<Mat> resVector;
    //进行卷积
    Mat resTempConvolve;
    for(int i=0;i<temp.size();i++){
        convolveDFT(img, temp[i], resTempConvolve);
        //convolve(&img, &temp[i], &resTempConvolve);
        //filter2D(img, resTempConvolve, -1, temp[i]);
        resVector.push_back(resTempConvolve);
    }
    //进行微分
    Mat resDiff;
    for(int i=0;i<temp.size()-1;i++){
        absdiff(resVector[i], resVector[i+1], resDiff);
        resVector[i]=resDiff;
    }
    resVector.pop_back();
    //均值滤波
    Mat filterTemp=Mat::ones(1, 5, CV_32FC1);
    for(int i=0;i<5;i++){
        filterTemp.at<uchar>(i)=0.2;
    }
    for(int i=0;i<resVector.size();i++){
        filter2D(resVector[i], resVector[i], -1, filterTemp);
    }
    //求取微分最大值和位置
    Mat covstackdiffmax(resVector[0]);
    /*int** covstackdiffmaxRadius=new int*[resVector[0].rows];
    for(int i=0;i<resVector[0].rows;i++){
        covstackdiffmaxRadius[i]=new int[resVector[0].cols];
        for(int j=0;j<resVector[0].cols;j++)
            covstackdiffmaxRadius[i][j]=0;
    }*/
    /*Mat covstackdiffmaxRadius(resVector[0]);
    findMax(resVector, covstackdiffmax, covstackdiffmaxRadius);
    vector<Mat> res;
    res.push_back(covstackdiffmax);
    res.push_back(covstackdiffmaxRadius);
    return res;
}

//--------------------------------------------
vector<float> DaugmanIrisLocalization(Mat eyeRectified, vector<Mat> temp, int frameCount, int leftOrRight){
    string whicheye=(leftOrRight==left)?"left ":"right ";
    cout<<"DaugmanIrisLocalization: "<<"It is the "<<frameCount<<"th frame and the "<<whicheye<<"eye!"<<endl;
    string countStr=boost::lexical_cast<string>(frameCount);
    string numStr=boost::lexical_cast<string>(leftOrRight);
    //先后存放灰度图和放大后的图
    vector<Mat> srcImg(preProcessing(eyeRectified,countStr,numStr));
    //先后存放微分最大值和对应半径值
    vector<Mat> srcIris(DaugmanIrisCore(srcImg[0],temp));
    //高斯滤波
    GaussianBlur(srcIris[0], srcIris[0], Size(0,0), 6.0);
    #ifdef _DEBUG
    imwrite("\\Users\\ly\\eyeProject\\current_head\\debugpic\\map\\eye"+countStr+"_"+numStr+".jpg", srcIris[0]);
    #endif
    //找出最大值的点和坐标
    Point maxPoint;
    minMaxLoc(srcIris[0], NULL, NULL, NULL, &maxPoint);
    float radius=srcIris[1].at<uchar>(maxPoint);
    if(frameCount<=stableFrame){
        irisRadiiLog.push_back(radius/imageWorkingWidth);
    }
    radius=radius*srcIris[0].rows/imageWorkingWidth;
    Point center=Point(float(maxPoint.x)*srcIris[0].rows/imageWorkingWidth,float(maxPoint.y)*srcIris[0].rows/imageWorkingWidth);
    vector<float> res;
    res.push_back(center.x);
    res.push_back(center.y);
    res.push_back(radius);
    return res;
}*/



