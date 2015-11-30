#import <Foundation/Foundation.h>
#import "OpenCVProcessingBridge.h"

@interface OpenCVProcess_DirectPass : NSObject <OpenCVProcessDelegate>
-(instancetype) init;
-(void) opencvProcessFrom:(const cv::Mat &)fromMat To:(cv::Mat &)toMat;
@end
