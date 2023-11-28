#pragma once
#ifndef PERSON_H
#define PERSON_H

using namespace std;

class Person;
// list of Person elements found in Picture

std::list<cv::Vec3b> colorList;

class Person {
public:
    int size;
    int number;//x and y coordinates and direction
    int points = 0;
    int maxCol = 0, maxRow = 0, minCol = 0, minRow = 0;
    bool found = true;
    cv::Vec3b bodyColors[BODYCOLORLENGTH];
    cv::Vec3b color; //color of the ball
    Person(int number, int size, int maxCol, int minCol, int maxRow, int minRow) {
        this->size = size;
        this->number = number;
        this->maxCol = maxCol;
        this->minCol = minCol;
        this->maxRow = maxRow;
        this->minRow = minRow;
        auto it = colorList.begin();
        std::advance(it, number%8);
        color = *it;
    }
    Person() {
        this->size = 0;
        this->number = 0;
    }

    int getNumber() {return number;}
    void setColor(cv::Vec3b newColor, int i) { this->bodyColors[i] = newColor; }
private:
};

#endif  // PERSON_H