#pragma once
#pragma once
#ifndef TESTVIDEOCREATOR_H
#define  TESTVIDEOCREATOR_H

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"

using namespace cv;


/*mirror vertically*/
Mat mirrorImage(Mat frame) {
    for (int row = 0; row < frame.rows; row++) { //iterate all pixel
        for (int col = 0; col < frame.cols/2; col++) {
            frame.at<cv::Vec3b>(row, frame.cols - col - 1) = frame.at<cv::Vec3b>(row, col);
        }
    }
    return frame;
}




#endif  //testvideocreator