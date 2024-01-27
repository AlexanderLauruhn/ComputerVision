#pragma once
#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include"Ball.h"
#include"BallFunktions.h"
#include"Constants.h"
#include"Boxplot_BS.h"
#include"Tracking.h"
#include"TestVideoCreator.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
using namespace std;
using namespace cv;

std::vector<cv::Mat> boxplot;
int framecounter = 0;
/*count the collision of balls and persons*/
void countBallHits(Mat frame) {
    for (Person& p: mainPerson){
        for (Ball& ball : ballList) {//detect on 8 points on circle
            for (double angle = 0.0; angle < 2 * PI; angle += PI / 4) {// 8 positions on the ball´s line
                int x = ball.x + ball.radius * cos(angle);
                int y = ball.y + ball.radius * sin(angle);
                if (x >= 0 && y >= 0 && x < frame.cols && y < frame.rows) { // if position in frame
                    if (frame.at<cv::Vec3b>(y, x) == p.color && ball.color != cv::Scalar(0, 0, 0)) {
                        PlaySound(TEXT("plopp.wav"), NULL, SND_FILENAME | SND_ASYNC);
                        ball.color = cv::Scalar(0, 0, 0); // dye ball blue
                        p.hitByBall++; //count hits
                        }
                    }
                }
            }
        }
}
/*add the hibByBall counter to image*/
Mat addCounterToImage(Mat cFrame) {
    cv::Mat output(cFrame.rows, cFrame.cols+50, cFrame.type(), cv::Scalar(0, 0, 0));
    // Kopiere den Inhalt des Eingabebildes in das erweiterte Bild
    cFrame.copyTo(output(cv::Rect(50, 0, cFrame.cols+0, cFrame.rows)));
    int offset = 50;
    for (Person p : mainPerson) {
        if (p.found || p.doublePerson) {
        std::stringstream strst;
        strst << p.hitByBall;
        std::string hitByBallString = strst.str();
        cv::putText(output, hitByBallString, cv::Point(5, offset), cv::FONT_HERSHEY_SIMPLEX, 1, p.color, 2);
        offset += 25;
    }
    }
    return output;
}

Mat addBallsToPicture(Mat frame) {  
    addBall(frame.cols, frame.rows); //add balls, if there are not enough balls
    removeFromList(frame.cols, frame.rows); //remove balls, if ball out of bounds
    countBallHits(frame);
    for (Ball& ball : ballList) {
        checkAllCollisions(ball);//check, if balls collide
        cv::circle(frame, cv::Point(ball.x, ball.y), ball.radius, ball.color, -1);//Fill circle in color
        cv::circle(frame, cv::Point(ball.x, ball.y), ball.radius, cv::Scalar(0, 0, 0), 1); // draw black circle around
        ball.shift(); //move balls to new position
    }
    return frame;
}

Mat combineImages(Mat backGround, Mat foreGround){
    if (backGround.size().width == 640) { //if live video , need cut off 60 pixel
        backGround = backGround(cv::Rect(0, BLACKAREARANGE, backGround.cols, backGround.rows - 2 * BLACKAREARANGE)); //remove black areas
    }
    cv::resize(backGround, backGround, cv::Size(640,360));
    cv::resize(foreGround, foreGround, cv::Size(640, 360));
    for (int row = 0; row < backGround.rows; row++) { //iterate all pixel
        for (int col = 0; col < backGround.cols; col++) {
            if (foreGround.at<cv::Vec3b>(row, col)!= cv::Vec3b(0,0,0)){
                backGround.at<cv::Vec3b>(row, col) = foreGround.at<cv::Vec3b>(row, col);
            }
        }
    }
    return backGround;
}

int expectationLeft = 0;
int expectationRight = 0;
double totalSwitches = 0;
double correctSwitches = 1;

void executeAssumptions(Mat frame) {
    int expL = 0;
    int expR = 0;
    for (Person p : mainPerson) {
        if (p.found) {
            if (p.minCol <40) {
                if (p.getID() == expectationLeft) {
                    correctSwitches++;
                }
                expR = p.getID();
                totalSwitches++;
            }
            if (p.maxCol > 120) {
                if (p.getID() == expectationRight) {
                    correctSwitches++;
                }
                expL = p.getID();
                totalSwitches++;
            }
        }
    }
    expectationRight = expR;
    expectationLeft = expL;
    if (totalSwitches == 100 || totalSwitches == 300) {
        cout.precision(3);
        cout << "ratio:" << correctSwitches / totalSwitches << endl;
        PlaySound(TEXT("plopp.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
}
Mat imageProcessing(Mat frame) {
    checkAssumptions = false;
    framecounter++;
   // frame = adjustBrightness(frame);
    frame = testImage(frame);
    Mat backGround = substractBackground(boxplot, frame);
    Mat imageWithPersons = findPersons(backGround, frame);
    
    // calls method to check the results of the tracking algorithm
    if (checkAssumptions) {
        executeAssumptions(frame);
    }

    imageWithPersons = addBallsToPicture(imageWithPersons); //add balls

    Mat result = combineImages(frame, imageWithPersons);
    result = addIDToImage(result);
    result = addCounterToImage(result);
    cv::resize(result, result, cv::Size(1216, 684));
    
    return  result;
}

// runs Live Video
int liveVideo() {
    VideoCapture cap(0); // 0 is for first camera (use 1 for second camera, if available)
    if (!cap.isOpened()) {// verify is camera is accessed correctely
        std::cerr << "Error opening camera!" << std::endl;
        return -1;
    }
    Mat frame;
    for (int i = 0; i < 50; i++) {
        cap >> frame; //warm up for camera
    }
    boxplot = createBackgroundImages(cap);
    while (true) {
        cap >> frame;  // get picture from camera loopwise
        if (frame.empty()) { // verify if picture is not empty
            std::cerr << "Frame is empty!" << std::endl;
            break;
        }
        frame = imageProcessing(frame);

        imshow("Game", frame);
        if (waitKey(5) == 27) { // Wait for 'esc' key press to exit
            break;
        }
    }
    cap.release(); // close camera and all windows
    cv::destroyAllWindows();
    return 1;
}

// runs Video
int videoLoop() {
    cout << "Enter file name: \n";
    string fileName;
    cin >> fileName;
    VideoCapture cap(fileName + ".mp4"); // cap is the object of class video capture that tries to capture Bumpy.mp4
    if (!cap.isOpened())  // isOpened() returns true if capturing has been initialized.
    {
        cout << "Cannot open the video file. \n";
        return -1;
    }
    double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video  
    Mat frame;
    boxplot = createBackgroundImages(cap);
    int waitTime = 1;
    while (1)
    {
        if (!cap.read(frame)){ // if not success, break loop
            cout << "\n Cannot read the video file or video terminated. \n";
            break;
        }
        frame = imageProcessing(frame);
        imshow("Game", frame);
        if (waitKey(waitTime) == 27) { // Wait for 'esc' key press to exit
            break;
        }
    }
    return 1;
}

#endif  // VIDEO_H