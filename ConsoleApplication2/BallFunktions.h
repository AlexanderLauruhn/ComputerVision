#pragma once
/*
 * Ball Funktions
 *
 */

#ifndef BALLFUNKTIONS_H
#define BALLFUNKTIONS_H

#include"Ball.h"
#include"Constants.h"


 // check if two balls collide
bool isCollision(Ball& ball1, Ball& ball2) {
    return (std::sqrt(std::pow(ball1.x - ball2.x, 2) + std::pow(ball1.y - ball2.y, 2)) <= (ball1.radius + ball2.radius));
}

//if two balls collide, change direction
void checkAllCollisions(Ball& ball1) {
    for (Ball& ball2 : ballList) {
        if ((ball1.x != ball2.x || ball1.y != ball2.y)) {
            if (isCollision(ball1, ball2)) {
                int shiftX1 = ball1.shiftX;
                int shiftY1 = ball1.shiftY;
                ball1.shiftX = ball2.shiftX;
                ball1.shiftY = ball2.shiftY;
                ball2.shiftX = shiftX1;
                ball2.shiftY = shiftY1;
            }
        }
    }
}

/*add a new ball to the list, fitting into the frame with columns and rows
* add to list only if no collision at start
*/
void addBall(int cols, int rows) {
    if (ballList.size() < AMOUNTOFBALLS) { //change here the number of balls
        Ball ball = Ball(cols, rows);
        bool noCollision = true;
        for (Ball& ball1 : ballList) { //check with each other ball
            if (isCollision(ball1, ball)) { noCollision = false; }
        }
        if (noCollision) { ballList.push_back(ball); } // add ball
    }
}

//remove any ball from list if it had left the frame
void removeFromList(int rows, int cols) {
    for (auto it = ballList.begin(); it != ballList.end();) {//remove if ball out of picture
        if ((it->x > rows + it->radius && it->shiftX > 0) ||
            (it->x < 0 - it->radius && it->shiftX < 0) ||
            (it->y > cols + it->radius) ||
            (it->y < -it->radius && it->shiftY < 0)
            || it->color == cv::Scalar(0, 0, 0)
            ) {
            it = ballList.erase(it);
        } // delete ball and set iterator to next ball in list
        else { ++it; }// go to next ball
    }
}


#endif  // BALLFUNKTIONS_H