#pragma once
#ifndef AVERAGEBS_H
#define AVERAGEBS_H

#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
using namespace std;
using namespace cv;


static Mat referenceFrame;
static Mat frameBS;
static Mat frameBSlastFrames;
static Mat frameResult;
static Mat previousFrame;
static queue<Mat> previousFrames; 

// contains frames for further determination of the reference frame for Averege BS
static vector<Mat> setOfFrames;
static const int startFrame = 5;
static const int endFrame = 55;
static const int delta = 8; 
static const int referenceBrightness = 100; 



// adapts the brightness in the current frame
Mat adaptBrightness(Mat inputFrame) {
    int rows = inputFrame.rows;
    int cols = inputFrame.cols;

    int averageI = 0;
    int sumI = 0;
    // calculate average brightness in the current frame
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            sumI += (int)inputFrame.at<unsigned char>(r, c);
        }
    }
    averageI = sumI / (rows * cols);
    
    // adjusting the brightness with a fixed reference value
    int deltaAverage = referenceBrightness - averageI;
    int newValuePixel = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            newValuePixel = (int)inputFrame.at<unsigned char>(r, c) + delta;

            if (newValuePixel >= 255) {
                inputFrame.at<unsigned char>(r, c) = 255;
            }
            else if (newValuePixel <= 0) {
                inputFrame.at<unsigned char>(r, c) = 0;
            }
            else {
                inputFrame.at<unsigned char>(r, c) = newValuePixel;
            }
        }
    }
    return inputFrame; 
}

// applies opening operation from openCV
Mat opening(Mat inputFrame, int kernelSize) {
    // Create a structuring element (kernel) for the opening operation
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
    // Perform morphological opening operation
    cv::Mat outputFrame;
    cv::morphologyEx(inputFrame, outputFrame, cv::MORPH_OPEN, kernel);

    return outputFrame;
}


// applies closing operation from openCV
Mat closing(Mat inputFrame, int kernelSize) {
    // Create a structuring element (kernel) for the closing operation
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
    // Perform morphological closing operation
    cv::Mat outputFrame;
    cv::morphologyEx(inputFrame, outputFrame, cv::MORPH_CLOSE, kernel);

    return outputFrame;
}


// fills up objects in the Frame with white pixels
Mat fillUpBody(Mat inputFrame) {
    Mat outputFrame = Mat::zeros(inputFrame.size(), inputFrame.type());
    // Frame for counting four four passages
    Mat addFrame = Mat::zeros(inputFrame.size(), inputFrame.type());
    int rows = inputFrame.rows;
    int cols = inputFrame.cols;
    
    // apply closing
    inputFrame = closing(inputFrame, 5);

    // fills up from the left to the right and from the right to the left borders
    for (int r = 0; r < rows; r++) {
        bool metPixelLeftRight = false;
        bool metPixelRightLeft = false;
        for (int c = 0; c < cols; c++) {

            // left->right
            if ((int)inputFrame.at<unsigned char>(r, c) == 255) {
                addFrame.at<unsigned char>(r, c) += 1;
                metPixelLeftRight = true;
            }
            if (metPixelLeftRight == true) {
                addFrame.at<unsigned char>(r, c) += 1;
            }

            // right->left
            if ((int)inputFrame.at<unsigned char>((rows - 1 - r), c) == 255) {
                addFrame.at<unsigned char>((rows - 1 - r), c) += 1;
                metPixelRightLeft = true;
            }

            if (metPixelRightLeft == true) {
                addFrame.at<unsigned char>((rows - 1 - r), c) += 1;
            }
        }
    }

    //  fills up from the top to the bottom and from the bottom to the top borders 
    for (int c = 0; c < cols; c++) {
        bool metPixelTopBottom = false;
        bool metPixelBottomTop = false;
        for (int r = 0; r < rows; r++) {
            // top - > bottom
            if ((int)inputFrame.at<unsigned char>(r, c) == 255) {
                addFrame.at<unsigned char>(r, c) += 1;
                metPixelTopBottom = true;
            }

            if (metPixelTopBottom == true) {
                addFrame.at<unsigned char>(r, c) += 1;
            }

            // bottom -> top
            if ((int)inputFrame.at<unsigned char>(r, (cols - 1 - c)) == 255) {
                addFrame.at<unsigned char>(r, (cols - 1 - c)) += 1;
                metPixelBottomTop = true;
            }
            if (metPixelBottomTop == true) {
                addFrame.at<unsigned char>(r, (cols - 1 - c)) += 1;
            }
        }
    }

    // create results frame 
    // if the pixel is equal to four, then it will be colored white
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((int)addFrame.at<unsigned char>(r, c) >= 4) {
                outputFrame.at<unsigned char>(r, c) = 255;
            }
        }
    }
    return outputFrame;
}


// counts the number of white pixels in frame
int countNumberOfPixels(Mat inputFrame) {
    int rows = inputFrame.rows;
    int cols = inputFrame.cols;
    int counter = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((int)inputFrame.at<unsigned char>(r, c) == 255) {
                counter++;
            }
        }
    }
    return counter;
}


// creates the average reference frame
void createReferenceFrameForAverageBS(Mat frameGray, int numberOfFrames) {
    // add frame
    setOfFrames.push_back(frameGray);

    if (setOfFrames.size() == numberOfFrames) {
        
        // create a reference frame for the background subtraction
        referenceFrame = Mat::zeros(frameGray.size(), frameGray.type());  
        cout << "\n *** The reference frame has been created ***";

        int rows = frameGray.rows;
        int cols = frameGray.cols;
        int PixelAverageValue = 0;

        // build the reference frame
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                for (int i = 0; i < numberOfFrames; i++) {
                    PixelAverageValue += (int)(setOfFrames.at(i)).at<unsigned char>(r, c);
                }
                PixelAverageValue = (unsigned char)(PixelAverageValue / numberOfFrames);  
                referenceFrame.at<unsigned char>(r, c) = PixelAverageValue;
                PixelAverageValue = (unsigned char)0;
            }
        }
    }
}


// applies background subtraction using the average reference frame
Mat createBS(Mat currentFrame, int counterFrame) {
    // convert BGR to Gray
    cvtColor(currentFrame, currentFrame, CV_BGR2GRAY);

    // resize current Frame
    //cv::resize(currentFrame, currentFrame, cv::Size(320, 180)); 

    // define the size of the box filter
    cv::Size ksize(5, 5);
    // apply the box filter
    cv::boxFilter(currentFrame, currentFrame, -1, ksize);

    // adapt brightness
    currentFrame = adaptBrightness(currentFrame);

    if (counterFrame == 0) {
        frameBS = Mat::zeros(currentFrame.size(), currentFrame.type());
    }
    else if (counterFrame >= startFrame && counterFrame < endFrame) {
        // create reference frame based on the initial frames 
        createReferenceFrameForAverageBS(currentFrame, endFrame - startFrame);
    }
    else if (counterFrame > endFrame) {
        int rows = currentFrame.rows;
        int cols = currentFrame.cols;
        int numerator = 0;

        // determining the background using absolute and relative errors
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                numerator = abs((int)currentFrame.at<unsigned char>(r, c) - (int)referenceFrame.at<unsigned char>(r, c));
                // change recognized
                if ((numerator >= 25) && (((float)numerator / (float)(int)referenceFrame.at<unsigned char>(r, c)) * 100) > 15) {
                    frameBS.at<unsigned char>(r, c) = (unsigned char)255;
                }
                // change not recognized 
                else { 
                    frameBS.at<unsigned char>(r, c) = (unsigned char)0;
                }
            }
        }

        // apply closing
        frameBS = closing(frameBS, 3);

        // size of the structuring element for the dilatation
        int dilationSize = 3;  
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1));
        // apply dilatation 
        cv::dilate(frameBS, frameBS, element);
    }
    return frameBS;
}


// applies background subtraction using the average reference frame and last three frames
Mat createBSCurrent(Mat currentFrame, int counterFrame) {

    // convert BGR to Gray
    cvtColor(currentFrame, currentFrame, CV_BGR2GRAY);

    // resize current Frame
    //cv::resize(currentFrame, currentFrame, cv::Size(320, 180)); 

    // define the size of the box filter
    cv::Size ksize(5, 5);
    // apply the box filter
    cv::boxFilter(currentFrame, currentFrame, -1, ksize);

    // adapt brightness
    currentFrame = adaptBrightness(currentFrame);

    if (counterFrame == 0) {
        frameBS = Mat::zeros(currentFrame.size(), currentFrame.type());
        frameBSlastFrames = Mat::zeros(currentFrame.size(), currentFrame.type());
        frameResult = Mat::zeros(currentFrame.size(), currentFrame.type());
    }
    else if (counterFrame >= startFrame && counterFrame < endFrame) {
        // create reference frame based on the initial frames
        createReferenceFrameForAverageBS(currentFrame, endFrame - startFrame);
        // initialize last three frames
        if (counterFrame == (endFrame - 1)) {
            previousFrames.push(currentFrame);
        }
        else if (counterFrame == (endFrame - 2)) {
            previousFrames.push(currentFrame);
        }
        else if (counterFrame == (endFrame - 3)) {
            previousFrames.push(currentFrame);
        }
    }
    else if (counterFrame > endFrame) {
        int rows = currentFrame.rows;
        int cols = currentFrame.cols;
        int numerator = 0;
        int numerator1 = 0;
        int numerator2 = 0;
        int numerator3 = 0;
        Mat puffer1, puffer2, puffer3;
        puffer1 = previousFrames.front();
        previousFrames.pop();
        puffer2 = previousFrames.front();
        previousFrames.pop();
        puffer3 = previousFrames.front();
        previousFrames.pop();

        // determining the background using the average reference frame and last three frames
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                // calculate absolute errors
                numerator = abs((int)currentFrame.at<unsigned char>(r, c) - (int)referenceFrame.at<unsigned char>(r, c));
                numerator1 = abs((int)currentFrame.at<unsigned char>(r, c) - (int)(puffer1).at<unsigned char>(r, c));
                numerator2 = abs((int)currentFrame.at<unsigned char>(r, c) - (int)(puffer2).at<unsigned char>(r, c));
                numerator3 = abs((int)currentFrame.at<unsigned char>(r, c) - (int)(puffer3).at<unsigned char>(r, c));

                // determining the background using absolute and relative errors and initial reference frame
                // change recognized
                if (((numerator >= 25) || (((float)numerator / (float)(int)referenceFrame.at<unsigned char>(r, c)) * 100) > 15)) {
                    frameBS.at<unsigned char>(r, c) = (unsigned char)255;
                }
                // change not recognized 
                else { 
                    frameBS.at<unsigned char>(r, c) = (unsigned char)0;
                }

                // determining the background using  last three frames
                // change recognized
                if (numerator1 >= 25 || numerator2 >= 25 || numerator3 >= 25) { 
                    frameBSlastFrames.at<unsigned char>(r, c) = (unsigned char)255;
                }
                // change not recognized 
                else { 
                    frameBSlastFrames.at<unsigned char>(r, c) = (unsigned char)0;
                }

            }
        }

        // apply closing
        frameBS = closing(frameBS, 3);

        // size of the structuring element for the dilatation
        int dilationSize = 3;  
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1));
        // apply dilatation
        cv::dilate(frameBS, frameBS, element);

        // add last three frames to the queue
        previousFrames.push(puffer1);
        previousFrames.push(puffer2);
        previousFrames.push(puffer3);
        // delete first frame in the queue
        previousFrames.pop();
        // add a new frame to the queue
        previousFrames.push(currentFrame);

        // apply closing
        frameBSlastFrames = closing(frameBSlastFrames, 3);
        // apply dilatation
        cv::dilate(frameBSlastFrames, frameBSlastFrames, element);

        // fill up the body 
        frameBSlastFrames = fillUpBody(frameBSlastFrames);

        // create results frame
        // no movement in the frame - background subtraction using the average reference frame
        if (countNumberOfPixels(frameBSlastFrames) < 1000) { 
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    if ((int)frameBS.at<unsigned char>(r, c) == 255) {
                        frameResult.at<unsigned char>(r, c) = (unsigned char)255;
                    }
                    else
                    {
                        frameResult.at<unsigned char>(r, c) = (unsigned char)0;
                    }
                }
            }
        }
        // enough movement in the frame - background subtraction using the average reference frame and last three frames
        else {
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    if ((int)frameBS.at<unsigned char>(r, c) == 255 && (int)frameBSlastFrames.at<unsigned char>(r, c) == 255) {
                        frameResult.at<unsigned char>(r, c) = (unsigned char)255;
                    }
                    else
                    {
                        frameResult.at<unsigned char>(r, c) = (unsigned char)0;
                    }
                }
            }
        }
    }

    return frameResult;
}


// Selection of the background Subtraction function
Mat createBSImage(Mat currentFrame, int counterFrame, int type) {
    // the background subtraction function with reference frame formed at the beginning
    if (type == 1) {
        return createBS(currentFrame, counterFrame);
    }
    // the background subtraction function with reference frames based on the initial frames and the last three frames
    return createBSCurrent(currentFrame, counterFrame);
}



#endif  // AVERAGEBS_H