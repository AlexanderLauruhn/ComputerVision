#pragma once
#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
using namespace std;

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

#endif  // TOOLS_H
