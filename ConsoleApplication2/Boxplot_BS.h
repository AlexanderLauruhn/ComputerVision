#pragma once
#pragma once
#ifndef BOXPLOT_BS_H
#define  BOXPLOT_BS_H

#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "Constants.h"
#include"TestVideoCreator.h"
using namespace std;
using namespace cv;
bool isPixelSimilar(int color, Vec3b q1, Vec3b pixel, Vec3b q3);
Mat adjustBrightness(Mat frame);

/*pretreatment to get each frame in same size and brightness*/
Mat imagePreTreatment(Mat frame){
    frame = testImage(frame);
    if (frame.size().width == 640) { //if live video , need cut off 60 pixel
        frame = frame(cv::Rect(0, BLACKAREARANGE, frame.cols, frame.rows - 2 * BLACKAREARANGE)); //remove black areas
    }
    cv::resize(frame, frame, cv::Size(COMPWIDTH, COMPHEIGHT));  //small size to decrease calculation time
    //frame = adjustBrightness(frame); //each frame in same average brightness
    return frame;
}
/*get a value from list at % position*/
int getValueFromList(std::list<int> listOfValues, float position) {
    std::list<int>::iterator i = listOfValues.begin();//start iterator at begin
    if (listOfValues.size() >= BACKGROUNDSUBSTRACTIONLENGTH * position) {
        std::advance(i, BACKGROUNDSUBSTRACTIONLENGTH * position); //shift to position of half lengh
    }
    return *i;
}

/*create a median picture pixelwise based on Mat array*/
std::vector<cv::Mat> createBackgroundImages(VideoCapture cap) {  
    cout << "taking images as background reference" << endl;
    Mat frames[BACKGROUNDSUBSTRACTIONLENGTH];
    for (int i = 0; i < BACKGROUNDSUBSTRACTIONLENGTH; i++) {
        Mat frame;
        cap >> frame; //get image and remove the upper and lower black area
        frames[i] = imagePreTreatment(frame);
    }
    cout << "background images taken - boxplot calculation started" << endl;
    std::vector<cv::Mat> boxplot(2);
    Mat quantil1Image(frames[0].size(), frames[0].type());  //the output
    Mat quantil2Image(frames[0].size(), frames[0].type());  //the output
    for (int row = 0; row < quantil1Image.rows; row++) { //iterate all pixel
        for (int col = 0; col < quantil1Image.cols; col++) {
            std::list<int> blue, green, red; //lists for all colors for each pixel
            for (int i = 0; i < BACKGROUNDSUBSTRACTIONLENGTH; i++) {
                blue.push_back(frames[i].at<cv::Vec3b>(row, col)[0]); //fill lists
                green.push_back(frames[i].at<cv::Vec3b>(row, col)[1]);
                red.push_back(frames[i].at<cv::Vec3b>(row, col)[2]);
            }
            blue.sort(); //sort to get easily the median or quartil
            green.sort();
            red.sort();
            quantil1Image.at<cv::Vec3b>(row, col) = cv::Vec3b(
                getValueFromList(blue, 0.1),
                getValueFromList(green, 0.1),
                getValueFromList(red, 0.1)); //fill medianImage with median values for each color
            quantil2Image.at<cv::Vec3b>(row, col) = cv::Vec3b(
                getValueFromList(blue, 0.9),
                getValueFromList(green, 0.9),
                getValueFromList(red, 0.9)); //fill medianImage with median values for each color
        }
    }
    boxplot[0] = quantil1Image;//lower reference
    boxplot[1] = quantil2Image; //upper reference
    cout << "Background generation finished" << endl;
    return boxplot;
}
/*opening and closing on all theree colors separately*/
Mat openingClosing(Mat substraction) {
    std::vector<cv::Mat> colorChannels;
    cv::split(substraction, colorChannels); //split in three grayscale images
    cv::Mat elementErosion = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * EROSION + 1, 2 * EROSION + 1));
    cv::Mat elementDilation = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * DILATATION + 1, 2 * DILATATION + 1));
    for (int i = 0; i < 3; ++i) {//erosion and dilataion with each size
        cv::erode(colorChannels[i], colorChannels[i], elementErosion);
        cv::dilate(colorChannels[i], colorChannels[i], elementDilation);
    }
    cv::merge(colorChannels, substraction); //merge after spliting and closing
    return substraction;

}
/*Compare all pixel after box filtering and brightness adustment with quantil images*/
Mat getComparedImage(Mat current, Mat quantil1, Mat quantil2 ){
    Mat substraction(current.size(), current.type()); // here we save if the pixel changed
    substraction.setTo(0);
    for (int row = 0; row < current.rows; row++) { //iterate all pixel
        for (int col = 0; col < current.cols; col++) {
            for (int color = 0; color < 3; color++) {
                if (!isPixelSimilar(//if difference on one of each color 
                    color,
                    quantil1.at<cv::Vec3b>(row, col),
                    current.at<cv::Vec3b>(row, col),
                    quantil2.at<cv::Vec3b>(row, col))) {
                    substraction.at<cv::Vec3b>(row, col)[color] = 255; //setblue, green or red depending on color value
                }
            }
        }
    }
    //substraction = openingClosing(substraction); //removed to get better shapes
    Mat substractionCopy;
    cv::resize(substraction, substractionCopy, cv::Size(640, 360));
    return substraction;
}


/*substract background from current frame to make differences visible*/
Mat substractBackground(std::vector<cv::Mat> boxplot, Mat currentFrame) {
    currentFrame = imagePreTreatment(currentFrame);
    if (currentFrame.size().width == 640) { //if live video , need cut off 60 black pixel without resizing
        currentFrame = currentFrame(cv::Rect(0, BLACKAREARANGE, currentFrame.cols, currentFrame.rows - 2 * BLACKAREARANGE)); //remove black areas
    }
    Mat substraction = getComparedImage(currentFrame, boxplot[0], boxplot[1]);
   // currentFrame = addCounterToImage(currentFrame, substraction);
    return  substraction;
}

/*check if a pixel in the current frame is similar to
the quartil images*/
bool isPixelSimilar(int color, Vec3b q1, Vec3b pixel, Vec3b q3) {
    if( (q1[color]  > pixel[color] * LOWER_THRESHOLD + LOWER_DELTA ||
        pixel[color] > q3[color] * UPPER_THRESHOLD + UPPER_DELTA)){
        return false;
    }
    return true;
}


/*Automatic Exposure Correction: set the average brightness to given value
this makes dark images brighter and bright images darker*/
Mat adjustBrightness(Mat frame) {
    int average[3]; // one average brightness values for each color
    average[0] = 0;
    average[1] = 0;
    average[2] = 0;
    for (int row = 0; row < frame.rows; row++) {// for each row
        for (int col = 0; col < frame.cols; col++) {//for each column
            for (int color = 0; color < 3; color++) { //for blue, green and red
                average[color] += frame.at<cv::Vec3b>(row, col)[color]; //sum up all values of each color
            }
        }
    }
    for (int color = 0; color < 3; color++) {
        average[color] /= (frame.cols * frame.rows); //calculate average
    }
    for (int row = 0; row < frame.rows; row++) {
        for (int col = 0; col < frame.cols; col++) {
            for (int color = 0; color < 3; color++) { //for blue, green and red
                if (frame.at<cv::Vec3b>(row, col)[color] + BRIGHTNESS - average[color] > 255) {
                    frame.at<cv::Vec3b>(row, col)[color] = 255; //not above 255 allowed
                }
                else if (frame.at<cv::Vec3b>(row, col)[color] + BRIGHTNESS - average[color] < 0) {
                    frame.at<cv::Vec3b>(row, col)[color] = 0; //no negativ values allowed
                }
                else { 
                    frame.at<cv::Vec3b>(row, col)[color] += (BRIGHTNESS - average[color]); //adjust average brightness to constant value
                }
            }
        }
    }
    return frame;
}

#endif  //boxplot_bs