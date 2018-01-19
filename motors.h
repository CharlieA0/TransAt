// motors.h

#ifndef _MOTORS_h
#define _MOTORS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Servo.h>

class Motor {
public:
	Motor(int);
	void start(); // Initializes ESCs and sets to 0 power (Takes 5 seconds)
	void setPower(int newPower); // Sets power to value between FULLREVERSE and FULLFORWARD
	int getPower();

private:
	int pin, power;
	Servo esc;
};

// TODO: Implement this class

class MotorArray {
public:
	MotorArray(int leftPin, int rightPin); // Create array with left and right motor pins.
	MotorArray(Motor left, Motor right);
};

#endif