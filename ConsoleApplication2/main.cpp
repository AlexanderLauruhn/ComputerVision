
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <string>

#include"TestClass.h"
#include"Tools.h"
#include"Video.h"


using namespace cv;
using namespace std;

int main(){
      colorList.push_back(cv::Vec3b(0, 255, 0));
      colorList.push_back(cv::Vec3b(255, 0, 0));
      colorList.push_back(cv::Vec3b(0, 0, 255));
      colorList.push_back(cv::Vec3b(255, 255, 0));
      colorList.push_back(cv::Vec3b(255, 0, 255));
      colorList.push_back(cv::Vec3b(0, 255, 255));
      colorList.push_back(cv::Vec3b(255, 255, 255));
      colorList.push_back(cv::Vec3b(0, 0, 0));
    
    int selectedOption = checkInput(1, 2, "Choose the option: \n Live Video -> Press <1> \n Video      -> Press <2> \n");
    if (selectedOption == 1) {
        liveVideo();
    }
    else {
        videoLoop();
    }
    return 0;
}





