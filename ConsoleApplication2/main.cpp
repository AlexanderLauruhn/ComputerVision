
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
    colorList = createcolorList();//setup list of colors for persons
    int selectedOption = checkInput(1, 2, "Choose the option: \n Live Video -> Press <1> \n Video      -> Press <2> \n");
    if (selectedOption == 1) {
        liveVideo();//camera
    }
    else {
        videoLoop();//video from storage
    }
    return 0;
}





