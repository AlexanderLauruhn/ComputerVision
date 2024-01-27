#pragma once
#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
using namespace std;
/*check what the users want to select*/
int checkInput(int startValue, int endValue, string msg) {
	int selectedOption;
	do {
		cout << endl << msg << endl;
		if (cin >> selectedOption && selectedOption >= startValue && selectedOption <= endValue) {
			break;
		}
		else if (!cin.bad() && !cin.eof()) {
			cerr << "You have entered an invalid number! Try it again.\n";
			cin.clear(); // Reset error state
			// remove incorrect input
			cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		}
	} while (cin);
	
	return selectedOption;
}
/*setup the list of the persons colors*/
std::list<cv::Vec3b> createcolorList() {
	std::list<cv::Vec3b> colorList;
	colorList.push_back(cv::Vec3b(0, 250, 0));//green
	colorList.push_back(cv::Vec3b(250, 0, 0));//blue
	colorList.push_back(cv::Vec3b(0, 0, 250));//red
	colorList.push_back(cv::Vec3b(250, 250, 0));//turquisie
	colorList.push_back(cv::Vec3b(250, 0, 250));//violet
	colorList.push_back(cv::Vec3b(0, 250, 250));//yellow
	colorList.push_back(cv::Vec3b(250, 250, 250));//white
	colorList.push_back(cv::Vec3b(100, 100, 100));//grey
	return colorList;
}
#endif  // TOOLS_H
