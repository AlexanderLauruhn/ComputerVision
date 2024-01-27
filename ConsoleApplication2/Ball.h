#pragma once
#ifndef BALL_H
#define BALL_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <random>

using namespace std;

int speed = 10;

class Ball;
// list of ball elements on picture
std::list<Ball> ballList; 

class Ball {
public:
    int radius, x,y, shiftX, shiftY; //x and y coordinates and direction
    cv::Scalar color; //color of the ball
    Ball(int row, int col) {
        std::random_device rd;
        std::mt19937 gen(rd()); // random generator
        std::uniform_int_distribution<int> distribution(row+col);
        int random = distribution(gen)%3;
        radius = (distribution(gen) % (row / 60)) + (row / 70);

        if (random % 3 == 0) {//ball coming from right
            x = -radius;
            y = distribution(gen) % col;
            shiftX = (distribution(gen) % 3) + 1;
            shiftY = (distribution(gen) % 3) - 1;
        }
        if (random % 3 == 1) { //ball coming from top
            y = -radius;
            x = distribution(gen) % row;
            shiftX = (distribution(gen) % 3) - 1;
            shiftY = (distribution(gen) % 3) + 1;
        }
        if (random % 3 == 2) {// ball coming from left
            x = row + radius;
            y = distribution(gen) % col;
            shiftX = -(distribution(gen) % 3) - 1;
            shiftY = (distribution(gen) % 3) - 1;
        }
        color = cv::Scalar(rand() % 230, rand() % 200, 250);
    }
    void shift() { //move in shiftx and shifty direction
        x += shiftX;
        y += shiftY;
    }
private:

};


#endif  // BALL_H