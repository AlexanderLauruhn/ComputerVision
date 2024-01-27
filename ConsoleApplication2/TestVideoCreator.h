#pragma once
#pragma once
#ifndef TESTVIDEOCREATOR_H
#define  TESTVIDEOCREATOR_H

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include"Person.h"

using namespace cv;

/*mirror vertically*/
Mat mirrorImage(Mat frame) {
    for (int row = 0; row < frame.rows; row++) { //iterate left half of frame
        for (int col = 0; col < frame.cols/2; col++) {
            frame.at<cv::Vec3b>(row, frame.cols - col - 1) = frame.at<cv::Vec3b>(row, col);
        }
    }
    return frame;
}
/*create a brightness gradient from left to right on frame*/
Mat brightnessGradientImage(Mat frame, int value) {
    for (int row = 0; row < frame.rows; row++) {
        for (int col = frame.cols/2; col < frame.cols; col++) {
            for (int color = 0; color < 3; color++) {//on each color 
                if ((frame.at<cv::Vec3b>(row, col)[color] + (col - frame.cols/2)/value) > 255) {
                    frame.at<cv::Vec3b>(row, col)[color] = 255; //avoid values above 255
                }
                else{
                    frame.at<cv::Vec3b>(row, col)[color] += (col - frame.cols/2) / value;
                }
                if ((frame.at<cv::Vec3b>(row, frame.cols  - col)[color] - (col-frame.cols/2) / value) < 0) {
                    frame.at<cv::Vec3b>(row, frame.cols  - col)[color] = 0; //avoid values below 0
                }
                else {
                    frame.at<cv::Vec3b>(row, frame.cols-col)[color] -= (col-frame.cols/2) / value;
                }
            }
        }
    }
    return frame;
}

/*create a test image with with noise*/
Mat addNoiseToImage(Mat frame, int noiseIntensity, int size) {
    std::random_device rd;
    std::mt19937 gen(rd()); // random generator
    std::uniform_int_distribution<int> distribution(frame.rows + frame.cols);
    int random = distribution(gen) % 3;
    for (int i = 0; i < noiseIntensity; i++) { //noise intensity is the amout of white circles
        int randomRow = distribution(gen) % frame.rows;
        int randomCol = distribution(gen) % frame.cols;//set white circles
        cv::circle(frame, cv::Point(randomCol,randomRow), size, cv::Scalar(255,255,255), -1);//Fill circle in color
    }
    return frame;
}
/*create an image with horizontal black lines*/
Mat addBlackLine(Mat frame, int width) {
    for (int i = 1; i < 8; i++) { //8 lines
        cv::line(frame, cv::Point(0, (frame.rows / 8) * i), cv::Point(frame.cols - 1, (frame.rows / 8) * i), cv::Scalar(0, 0, 0), width);
    }
    return frame;
}

Mat addBlackLineV(Mat frame, int width) {
    for (int i = 1; i < 8; i++) { //8 lines
        cv::line(frame, cv::Point((frame.cols / 8) * i,0), cv::Point((frame.cols / 8) * i, frame.rows-1), cv::Scalar(0, 0, 0), width);
    }
    return frame;
}
int frameCounter = 0;
Person p1;
Person p2;
int p1shift = 2;
int p2shift = -2;
bool checkAssumptions = false;

Mat addPersons(Mat frame, int quaking){
    frameCounter++;
    std::random_device rd;
    std::mt19937 gen(rd()); // random generator
    std::uniform_int_distribution<int> distribution(255);
    if (frameCounter == BACKGROUNDSUBSTRACTIONLENGTH) {
        distribution(gen);
        p1 = Person(0, 0, 3 * frame.cols / 10, 2 * frame.cols / 10, 2 * frame.rows / 3, frame.rows / 3);
        for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
            p1.setColor(cv::Vec3b(distribution(gen), distribution(gen), distribution(gen)), i);
        }
    }

    if (frameCounter == BACKGROUNDSUBSTRACTIONLENGTH+1) {
        distribution(gen);
        p2 = Person(0, 0, 8 * frame.cols / 10, 7 * frame.cols / 10, 2 * frame.rows / 3, frame.rows / 3);
        for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
            p2.setColor(cv::Vec3b(distribution(gen), distribution(gen), distribution(gen)), i);
        }
    }

    if (frameCounter>BACKGROUNDSUBSTRACTIONLENGTH+1) {
        +5 - distribution(gen) % 10;
        int slideRow1 = (p1.maxRow - p1.minRow) / MEANROWFEATURELENGTH;
        int slideRow2 = (p2.maxRow - p2.minRow) / MEANROWFEATURELENGTH;
        for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
            cv::rectangle(frame, cv::Point(p1.minCol - distribution(gen) % quaking,
                p1.minRow + i * slideRow1 - distribution(gen) % quaking),
                cv::Point(p1.maxCol + distribution(gen) % quaking,
                p1.minRow + (i + 1) * slideRow1 + distribution(gen) % quaking),
                p1.meanRowFeature[i], CV_FILLED);
            for (int color = 0; color < 3; color++) {
                int colorshift = distribution(gen) % quaking - distribution(gen) % quaking;
                if (p1.meanRowFeature[i][color] + colorshift <= 0 || p1.meanRowFeature[i][color] + colorshift >= 255) { 
                    colorshift = 0; 
                }
                p1.meanRowFeature[i][color] += colorshift;
            }
        }
        for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
            cv::rectangle(frame, cv::Point(p2.minCol - distribution(gen) % quaking,
                p2.minRow + i * slideRow2 - distribution(gen) % quaking),
                cv::Point(p2.maxCol + distribution(gen) % quaking,
                p2.minRow + (i + 1) * slideRow2 + distribution(gen) % quaking),
                p2.meanRowFeature[i], CV_FILLED);
            for (int color = 0; color < 3; color++) {
                int colorshift = distribution(gen) % quaking - distribution(gen) % quaking;
                if (p2.meanRowFeature[i][color] + colorshift <= 0 || p2.meanRowFeature[i][color] + colorshift >= 255) { 
                    colorshift = 0; 
                }
                p2.meanRowFeature[i][color] += colorshift;
            }
        }
        if (p1.minCol <0 || p1.maxCol > frame.cols ||
            p2.minCol <0 || p2.maxCol > frame.cols) {
            p1shift *= -1;
            p2shift *= -1;
            checkAssumptions = true;
        }
        p1.maxCol += p1shift;
        p1.minCol += p1shift;
        p2.maxCol += p2shift;
        p2.minCol += p2shift;
    }
    return frame;
}

Mat randomBrightness(Mat frame, float factor) {
   //create random value
    float randomValue = (std::rand() % 200 - 100) / 100.0f;
    cv::Mat adjustedImage = frame.clone();  // Copy

    if (randomValue > 0) {//brighter
        cv::add(adjustedImage, cv::Scalar(randomValue * factor), adjustedImage);
    }
    else {//darker
        cv::subtract(adjustedImage, cv::Scalar(-randomValue * factor), adjustedImage);
    }
    // only 0-25
    adjustedImage.setTo(cv::Scalar(0), adjustedImage < 0);
    adjustedImage.setTo(cv::Scalar(255), adjustedImage > 255);
    return adjustedImage;
}

Mat applyBoxBlur(Mat frame, int kernelSize) {
    // Box-Filter-Kernel 
    cv::Mat kernel = cv::Mat::ones(kernelSize, kernelSize, CV_32F) / (float)(kernelSize * kernelSize);
    // apply
    cv::filter2D(frame, frame, -1, kernel);
    //cv::resize(frame, frame, cv::Size(160, 90));
    //cv::resize(frame, frame, cv::Size(640, 360));

    return frame;
}

/*create a testImage based on given method*/
Mat testImage(Mat frame) {
    //return  addBlackLineV(frame,6);
    //return randomBrightness(frame, 0);
    //return addPersons(frame, 20);
    return frame;
   // return brightnessGradientImage(frame, 25);
    //return mirrorImage(frame);
    //return addNoiseToImage(frame, 150, 5);
    //return applyBoxBlur(frame, 40);
}


#endif  //testvideocreator