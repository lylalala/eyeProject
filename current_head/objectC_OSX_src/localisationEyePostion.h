//
//  SampleOpenCVRoute.h
//  GazeEstimationOnCocoa
//
//  Created by JiangZhiping on 15/11/4.
//  Copyright © 2015年 JiangZhiping. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OpenCVProcessingBridge.h"
#import <string>
#import <vector>
using namespace std;
using namespace cv;


@interface OpenCVProcess_DirectPass : NSObject <OpenCVProcessDelegate>

-(instancetype) init;

//main function
-(void) opencvProcessFrom:(const cv::Mat &)fromMat To:(cv::Mat &)toMat;

@end
