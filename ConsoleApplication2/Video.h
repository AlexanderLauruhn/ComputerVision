#pragma once
#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include <string>
#include <fstream>
#include<windows.h>

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

Mat addBallsToPicture(Mat frame) {
    addBall(frame.cols, frame.rows); //add balls, if there are not enough balls
    removeFromList(frame.cols, frame.rows); //remove balls, if ball out of bounds
    for (Ball& ball : ballList) {
        checkAllCollisions(ball);//check, if balls collide
        cv::circle(frame, cv::Point(ball.x, ball.y), ball.radius, ball.color, -1);//Fill circle in color
        cv::circle(frame, cv::Point(ball.x, ball.y), ball.radius, cv::Scalar(0, 0, 0), 1); // draw black circle around
        ball.shift(); //move balls to new position
    }
    return frame;
}
std::vector<cv::Mat> boxplot;
Mat imageProcessing(Mat frame) {
    Mat backGround = substractBackground(boxplot, frame);
    Mat imageWithPersons = findPersons(backGround, frame);
    //addBallsToPicture(frame); //add balls
    return imageWithPersons;
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
        if (cv::waitKey(5) == 'q') {   // wait 30 ms for key input, quit loop with key q
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
    VideoCapture cap("v" + fileName + ".mp4"); // cap is the object of class video capture that tries to capture Bumpy.mp4
    if (!cap.isOpened())  // isOpened() returns true if capturing has been initialized.
    {
        cout << "Cannot open the video file. \n";
        return -1;
    }
    double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video  
    Mat frame;
    boxplot = createBackgroundImages(cap);
    while (1)
    {
        if (!cap.read(frame)){ // if not success, break loop
            cout << "\n Cannot read the video file or video terminated. \n";
            break;
        }
        frame = imageProcessing(frame);
        imshow("Game", frame);
        if (waitKey(25) == 27){ // Wait for 'esc' key press to exit
            break;
        }
    }
    return 1;
}

#endif  // VIDEO_H