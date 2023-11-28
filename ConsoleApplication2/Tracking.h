#pragma once
#pragma once
#ifndef TRACKING_H
#define TRACKING_H

#include <algorithm>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "Constants.h"
#include "Person.h"

using namespace std;
using namespace cv;

int counter = 0; //count the amount of areas
Mat marker; //if count  areas, label here the pixel
Mat base; //if count areas, count on this Mat
Mat originalCopy; //original, but resized COMPWIDTH/COMPHEIGHT
int iterationCounter = 0; //count iterations of recursion to avoid stack overflow
Person setupbodycolors(Person person, int maxCol, int minCol, int maxRow, int minRow);
Person mainPerson; // this person is what the tracking is looking for
bool personInitialised = false; //true, if mainPerson is initialised

/*label each pixel in Mat to count the foreground areas*/
void markPosition(int row, int col, int value) {
    if (row >= 0 && col >= 0 && row < marker.rows && col < marker.cols
        && marker.at<uchar>(row, col) != value &&
        base.at<uchar>(row, col) != 0
        && iterationCounter < 3000
        ) { // if position in frame
        marker.at<uchar>(row, col) = value; //label pixel
        iterationCounter++; //count recursion iterations = count pixel
        markPosition(row + 1, col, value); //recursively label the whole area
        markPosition(row - 1, col, value);
        markPosition(row, col + 1, value);
        markPosition(row, col - 1, value);
    }
}
/*count the foreground areas after background substraction*/
Mat countAreas(Mat frame) {
    counter = 0;
    cv::resize(frame, base, cv::Size(COMPWIDTH, COMPHEIGHT), cv::INTER_LINEAR); //resize linearly to avoid stack overflow
    if (frame.channels() == 3) { //if frame is in color, convert to gray
        cv::cvtColor(base, base, cv::COLOR_BGR2GRAY);
    }
    marker = Mat::zeros(base.rows, base.cols, base.type());
    marker.setTo(0);
    for (int row = 0; row < base.rows; row++) { //iterate all pixel
        for (int col = 0; col < base.cols; col++) {
            if (base.at<uchar>(row, col) != 0 && marker.at<uchar>(row, col) == 0){
                counter++; // count the areas
                iterationCounter = 0;
                markPosition(row, col, counter); //recursively label whole area
            }
        }
    }
    return marker;
}
/*determine if object has the shape of a person*/
bool isNewPerson(int pixelCounter, int maxCol, int minCol, int maxRow, int minRow) {
    return (pixelCounter > MINPIXELCOUNT && pixelCounter < MAXPIXELCOUNT
        && maxCol - minCol > MINPERSONWIDTH && maxRow - minRow > MINPERSONHEIGHT);
}
/*show the person in its color on a frame*/
Mat pictureWithPersons(std::list<Person> personList, Mat original) {
    cv::resize(original, original, cv::Size(640, 360));
    for (auto it = personList.begin(); it != personList.end(); ++it) {//for all person objects
        for (int row = 0; row < marker.rows; row++) { //iterate all pixel
            for (int col = 0; col < marker.cols; col++) {
                if (marker.at<uchar>(row, col) == it->number) {
                    for (int i = 0; i < 4; i++) {//two loops to increase capacity
                        for (int j = 0; j < 4; j++) {
                            original.at<cv::Vec3b>(row * 4 + i, col * 4 + j) = it->color;
                        }
                    }
                }
            }
        }
    }
    return original;
}
//largest Person first
bool compareBySize(Person& a, Person& b) {
    return a.size > b.size;
}
/**/
void initialisePerson(std::list<Person> personList) {
    if (!personInitialised && !personList.empty()) {//verify at least one object is in list
        personList.sort(compareBySize);
        Person candidate =  personList.front(); //largest person, because sort by Size
        if (candidate.minCol>0 && candidate.minCol > 0 //mainPerson not at frame border
            && candidate.maxRow < marker.rows && candidate.maxCol < marker.cols) {
                personInitialised = true;
                cout << "Person Initialised" << endl;
                mainPerson = candidate;
        }
    }
}
/*find all Person objects on a frame*/
Mat findPersons(Mat marker, Mat original) {
    cv::resize(original, originalCopy, cv::Size(COMPWIDTH, COMPHEIGHT));
    marker = countAreas(marker); 
    std::list<Person> personList;
    for (int i = 1; i <= counter; i++) {
        int minCol = marker.cols;//bounding box of person
        int maxCol = 0;
        int minRow = marker.rows;
        int maxRow = 0;
        int pixelCounter = 0;
        for (int row = 0; row < marker.rows; row++) { //iterate all pixel
            for (int col = 0; col < marker.cols; col++) {
                if (marker.at<uchar>(row, col) == i) {
                    pixelCounter++;// find maxima and minima
                    if (minCol > col) { minCol = col; }
                    if (maxCol < col) { maxCol = col; }
                    if (minRow > row) { minRow = row; }
                    if (maxRow < row) { maxRow = row; }
                }
            }
        }
        if (isNewPerson(pixelCounter, maxCol, minCol, maxRow, minRow)){
            Person person = Person(i, pixelCounter, maxCol, minCol, maxRow, minRow);
            person = setupbodycolors(person, maxCol, minCol, maxRow, minRow);
            personList.push_back(person);
        }
    }
    initialisePerson(personList);
    original = pictureWithPersons(personList, original);
    return original;
}
/*calculate mean row feature of person*/
Person setupbodycolors(Person person, int maxCol, int minCol, int maxRow, int minRow) {
     int distanceRow = (maxRow - minRow) / BODYCOLORLENGTH;//the height of each color block
     //originalCopy = adjustBrightness(originalCopy);
     for (int i = 1; i <= BODYCOLORLENGTH; i++) {
         int blue = 0, green = 0, red = 0;
         int pixelCounter = 1;// to avoid diviation by zero
         for (int row = minRow + (i-1) * distanceRow; row <= minRow + i * distanceRow; row++) {
             for (int col = minCol; col <= maxCol; col++) {
                 if (marker.at<uchar>(row, col) == person.getNumber()) {
                     pixelCounter++;
                     blue += originalCopy.at<cv::Vec3b>(row, col)[0];
                     green += originalCopy.at<cv::Vec3b>(row, col)[1];
                     red += originalCopy.at<cv::Vec3b>(row, col)[2];
                 }
             }
         }
         blue /= pixelCounter;//mean of each color
         green /= pixelCounter;
         red /= pixelCounter;
        // cout << blue <<"  "<<green<<"  "<<red << endl;
        // cv::rectangle(originalCopy, cv::Point(minCol, minRow + (i - 1) * distanceRow), cv::Point(maxCol, minRow + i * distanceRow), cv::Vec3b(blue, green, red), 2);
     }
    // cout << endl;
     //cv::resize(originalCopy, originalCopy, cv::Size(320, 180));
     //imshow("rectangle",originalCopy);
     return person;
 }
/*calculate the distance of the middle of two persons*/
double euclidianDistance(Person p1, Person p2) {
    int p1CenterRow = p1.maxRow - (p1.maxRow - p1.minRow) / 2;
    int p2CenterRow = p2.maxRow - (p2.maxRow - p2.minRow) / 2;
    int p1CenterCol = p1.maxCol - (p1.maxCol - p1.minCol) / 2;
    int p2CenterCol = p2.maxCol - (p2.maxCol - p2.minCol) / 2;
    return (p1CenterRow - p2CenterRow) + (p1CenterCol - p2CenterCol);// und das jeweil quadrieren und davon die wurzel

}

#endif  //tracking.h
#pragma once
