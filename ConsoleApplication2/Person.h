#pragma once
#ifndef PERSON_H
#define PERSON_H

using namespace std;

int personInitialisation = 0; //count the amount of initialised persons

class Person;
// list of Person elements found in Picture

std::list<cv::Vec3b> colorList;


class Person {
public:
    int size; //amout of pixel
    int number;//x and y coordinates and direction
    int points = 0,  id = 0;
    int maxCol = 0, maxRow = 0, minCol = 0, minRow = 0;
    bool found = false;//true if person in mainperson list
    int appearenceCounter = 0;//count how often person is found
    int hitByBall = 0; //count how many balls hit the foreground
    cv::Vec3b meanRowFeature[MEANROWFEATURELENGTH];
    cv::Vec3b color; //color of the ball
    bool doublePerson = false; //true if person behind another person
    Person(int number, int size, int maxCol, int minCol, int maxRow, int minRow) {
        this->size = size;
        this->number = number;
        this->maxCol = maxCol;
        this->minCol = minCol;
        this->maxRow = maxRow;
        this->minRow = minRow;

    }
    Person() {
        this->size = 0;
        this->number = 0;
    }

    int getNumber() {return number;}
    void setNumber(int num) { this->number = num; }
    int getID() { return id; }
    int getHalfWidth() {
        return (maxCol - minCol) / 2;
    }
    void setID(int id) { 
        this->id = id;
        auto it = colorList.begin();
        std::advance(it, id % 8);
        color = *it;
    }
    void setColor(cv::Vec3b newColor, int i) { this->meanRowFeature[i] = newColor; }
    cv::Vec3b getScoreColor() { return cv::Vec3b(color[0]+1, color[1]+1, color[2]+1); }
    /*update the mainPerson with the features of the selected person from frame*/
    void upDate(Person p) {
        if (found) {
            for (int i = 0; i < MEANROWFEATURELENGTH; i++) {
                for (int color = 0; color < 3; color++) {//refresh with 20% of new frame mean row colors
                    this->meanRowFeature[i][color] = (p.meanRowFeature[i][color] + this->meanRowFeature[i][color] * 4) / 5;
                }
            }
        }//update id and bounding box
        this->number = p.number;
        this->minCol = p.minCol;
        this->maxCol = p.maxCol;
        this->minRow = p.minRow;
        this->maxRow = p.maxRow;
        this->size = p.size;
    }
private:
};

#endif  // PERSON_H