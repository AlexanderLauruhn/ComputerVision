#pragma once
#pragma once
#ifndef TRACKING_H
#define TRACKING_H

#include <algorithm>
#include <iostream>
#include <cmath>
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
Person setupMeanRowFeature(Person person);
std::list<Person> mainPerson; // this person is what the tracking is looking for

double euclidianDistance(Person p1, Person p2);  //calculates the length between two centers of persons
int colorDifference(Person p1, Person p2);  //calculate the difference of mean row feature
Person currentPerson;

/*label each pixel in Mat to count the foreground areas = region growing*/
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
        markPosition(row + 4, col, value);
    }
}
/*count the foreground areas after background substraction*/
Mat countAreas(Mat frame) {
    counter = 0;
    cv::resize(frame, base, cv::Size(COMPWIDTH, COMPHEIGHT), cv::INTER_LINEAR); //resize linearly to avoid stack overflow
    if (frame.channels() == 3) { //if frame is in color, convert to gray
        cv::cvtColor(base, base, cv::COLOR_BGR2GRAY);
    }
    marker = Mat::zeros(base.rows, base.cols, base.type());//start with black image
    for (int row = 0; row < base.rows; row++) { //iterate all pixel
        for (int col = 0; col < base.cols; col++) {
            if (base.at<uchar>(row, col) != 0 && marker.at<uchar>(row, col) == 0){
                counter++; // count the areas
                iterationCounter = 0; //count the pixels
                markPosition(row, col, counter); //recursively label whole area
            }
        }
    }
    return marker;
}
/*determine if object has the shape of a person*/
bool isPerson(int pixelCounter, int maxCol, int minCol, int maxRow, int minRow) {
    return (pixelCounter > MINPIXELCOUNT && pixelCounter < MAXPIXELCOUNT
        && maxCol - minCol > MINPERSONWIDTH && maxRow - minRow > MINPERSONHEIGHT
        );
}
/*return an image with mainperson's color only*/
Mat removeWrongAreas(Mat input) {
    cv::Mat output(input.rows, input.cols, CV_8UC3);//black
    output.setTo(cv::Vec3b(0, 0, 0));
    for (Person p : mainPerson) {
        if (p.found && p.appearenceCounter > PERSONNOTFOUNDCOUNTER) {
            for (int row = 0; row < input.rows; row++) { //iterate all pixel
                for (int col = 0; col < input.cols; col++) {
                    if (input.at<uchar>(row, col) == p.number) {
                        output.at<cv::Vec3b>(row, col) = p.color;
                    }
                }
            }
        }
    }
  cv::Mat dilatierteImg;
  cv::dilate(output, dilatierteImg, cv::Mat(), cv::Point(-1, -1), 1);
  cv::Mat kantenBild;
  cv::absdiff(output, dilatierteImg, output);
  return output;
}
/*add the ID of each person to image*/
Mat addIDToImage(Mat frame) {
    int offSet = 20;
    for (Person p : mainPerson) {//large size ID
        if (p.found && p.appearenceCounter > PERSONNOTFOUNDCOUNTER) {
            std::stringstream strst;
            strst << p.id;
            std::string idString = strst.str();
            cv::putText(frame, idString, cv::Point(p.minCol*4, p.minRow*4), cv::FONT_HERSHEY_SIMPLEX, 0.6, p.getScoreColor(), 1);
        }
        if (p.doublePerson && p.appearenceCounter> PERSONNOTFOUNDCOUNTER) {
            std::stringstream strst;
            strst << p.id;
            std::string idString = strst.str();//small size ID of minor person
            cv::putText(frame, idString, cv::Point(p.minCol*4 + offSet, p.minRow*4), cv::FONT_HERSHEY_SIMPLEX, 0.4, p.getScoreColor(), 1);
            offSet += 20;
        }
    }
    return frame;
}
/*show the person in its color on a frame*/
Mat pictureWithPersons(Mat original) {
    cv::resize(original, original, cv::Size(640, 360));
    Mat onlyMainPerson = removeWrongAreas(marker);//get only the mainpersons color and black
    cv::resize(onlyMainPerson, onlyMainPerson, cv::Size(640, 360), 0,0,cv::INTER_NEAREST);
    return onlyMainPerson;// original;
}
//compare by amount of pixel, largest Person first
bool compareBySize(Person& a, Person& b) {
    return a.size > b.size;
}

//compare the distance to mainperson in previous frame: nearest first
bool compareByDistanceToMainPerson(Person& a, Person& b) {
    return euclidianDistance(a,currentPerson) < euclidianDistance(b, currentPerson);
}
//compare by meanrowfeature
bool compareByMeanRowFeature(Person& a, Person& b) {
    return colorDifference(a, currentPerson) < colorDifference(b, currentPerson);
}


/*initialise a person, which comes into the frame*/
int idValue = 0;
void initialisePerson(std::list<Person> personList) {
    if (!personList.empty()) {//verify at least one object is in list
        for (Person candidate: personList){
            if (!candidate.found &&
                candidate.minRow > marker.rows * 0.02 && candidate.minCol > marker.cols * 0.02 //mainPerson not at frame border
                && candidate.maxRow < marker.rows * 0.95 && candidate.maxCol < marker.cols * 0.98
                && (candidate.maxRow - candidate.minRow)>(1.2 * MINPERSONHEIGHT)) {
                personInitialisation++;//only on time per video, this person will be tracked
                cout << "Person Initialised" << endl;
                candidate.setID(++idValue);//new ID to identify person
                mainPerson.push_back(candidate);
            }
        }
    }
}
/*find all Person objects on a frame*/
Mat findPersons(Mat marker, Mat original) {
    cv::resize(original, originalCopy, cv::Size(COMPWIDTH, COMPHEIGHT));
    marker = countAreas(marker); 
    std::list<Person> personList; //list of persons in the current Frame
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
        if (isPerson(pixelCounter, maxCol, minCol, maxRow, minRow)){
            Person person = Person(i, pixelCounter, maxCol, minCol, maxRow, minRow);
            person = setupMeanRowFeature(person); //calculate colors of mean rows
            personList.push_back(person);
        }
    }
    for (Person& p1 : mainPerson) {
        p1.found = false;
        p1.doublePerson = false;
        currentPerson = p1;
        personList.sort(compareByMeanRowFeature);
        for (Person& p2 : personList) { //compare with all persons from frame
            if (// colorDifference(p1, p2) < COLORSIMILARITY &&
                !p1.found &&
                !p2.found &&
                euclidianDistance(p1,p2) <= EUCLIDIANDISTANCEOFFSET + p1.getHalfWidth() + p2.getHalfWidth()) {
                p1.found = true;
                p2.found = true;
                p1.appearenceCounter++;
                p1.upDate(p2); //update person with new boxinformation
            }
            if (!p1.found &&//person might be behind other person
                p2.found &&
                euclidianDistance(p1, p2) <= EUCLIDIANDISTANCEOFFSET + p1.getHalfWidth() + p2.getHalfWidth()) {
                p1.upDate(p2); //update overlapped person
                p1.doublePerson = true;
            }
        }
        if (!p1.found) {
            p1.setNumber(-1);//besser doch entfernen
        }
    }
    /*for (auto it = mainPerson.begin(); it != mainPerson.end();) {//remove if ball out of picture
        if ( !(it->found) &&
            (it->appearenceCounter> PERSONNOTFOUNDCOUNTER
                //|| it->minCol == 0 || it->maxCol == 160)
                )) {
            it = mainPerson.erase(it);
        } // delete person and set iterator to next ball in list
        else { ++it; }// go to next person
    }*/
    initialisePerson(personList); //add person to list
    original = pictureWithPersons(original);// add person to image
    return original;
}
/*compares, how similar the mean row feature is: small value = very similar*/
int colorDifference(Person p1, Person p2) {
    int difference = 0;
    for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
        for (int color = 0; color < 3; color++) {
            difference += std::abs(p1.meanRowFeature[i][color] - p2.meanRowFeature[i][color]);
        }
    }
    return difference / MEANROWFEATURELENGTH; //independent from length
}

/*similar to erosion; use only inner pixels for mean row feature*/
bool isPixelnotatBorder(int row, int col, Person person) {
    if (row == 0 || row == marker.rows-1 || col == 0 || col == marker.cols-1){
        return false;
    }
    else if (marker.at<uchar>(row, col) != person.getNumber()
        || marker.at<uchar>(row-1, col) != person.getNumber()
        || marker.at<uchar>(row + 1, col) != person.getNumber()
        || marker.at<uchar>(row, col + 1) != person.getNumber()
        || marker.at<uchar>(row, col -1 ) != person.getNumber()) {
        return false; //necessary for meanrowfeature
    }
    else { return true;}
}
/*calculate mean row feature of person*/
Person setupMeanRowFeature(Person person) {
     int distanceRow = (person.maxRow - person.minRow) / MEANROWFEATURELENGTH;//the height of each color block
     Mat adjustedCopy = originalCopy;// adjustBrightness(originalCopy);
     for (int i = 1; i <= MEANROWFEATURELENGTH; i++) {
        int blue = 0, green = 0, red = 0;//start black
        int pixelCounter = 1;// to avoid diviation by zero
        for (int row = person.minRow + (i - 1) * distanceRow; row <= person.minRow + i * distanceRow; row++) {
            for (int col = person.minCol; col <= person.maxCol; col++) {
                if (isPixelnotatBorder(row, col, person)) {
                    pixelCounter++;
                    blue += adjustedCopy.at<cv::Vec3b>(row, col)[0];
                    green += adjustedCopy.at<cv::Vec3b>(row, col)[1];
                    red += adjustedCopy.at<cv::Vec3b>(row, col)[2];
                }
            }
        }
        blue /= pixelCounter;//mean of each color
        green /= pixelCounter;
        red /= pixelCounter;
        person.setColor(cv::Vec3b(blue, green, red), i - 1);
     }
     return person;
 }

/*calculate the distance of the middle of two persons*/
double euclidianDistance(Person p1, Person p2) {
    int p1CenterRow = p1.maxRow - (p1.maxRow - p1.minRow) / 2;//middle of row
    int p2CenterRow = p2.maxRow - (p2.maxRow - p2.minRow) / 2;
    int p1CenterCol = p1.maxCol - (p1.maxCol - p1.minCol) / 2;//middle of col
    int p2CenterCol = p2.maxCol - (p2.maxCol - p2.minCol) / 2;
    return std::sqrt(std::pow((p1CenterRow - p2CenterRow), 2) + std::pow((p1CenterCol - p2CenterCol), 2));
}

#endif  //tracking.h
#pragma once
