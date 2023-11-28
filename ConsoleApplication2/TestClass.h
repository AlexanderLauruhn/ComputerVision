#pragma once
/*
 * TestClass.h
 *
 */

#ifndef TESTCLASS_H
#define TESTCLASS_H

#include <iostream>
#include <string>

using namespace std;

class TestClass {

public:
	TestClass();
	~TestClass();

	void printInfo();
	void setNumber(int newValue);


private:
	int number;
};

TestClass::TestClass() {
	number = 1;
}

TestClass::~TestClass() {
	// pass
}

void TestClass::printInfo() {
	std::cout << endl << "Current number: " << number << endl;
}

void TestClass::setNumber(int newValue) {
	number = newValue;
}

#endif  // TESTCLASS_H