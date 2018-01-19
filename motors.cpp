// 
// Library for controlling TransAt vehicle's motors
// 

#include "motors.h"
#include "Arduino.h"

#define FULLFORWARD 1000
#define FULLBACK -1000
#define MAXSIGNAL 2000
#define MINSIGNAL 1000

Motor::Motor(int motorPin) {
	pin = motorPin;
}

void Motor::start() {
	esc.attach(pin);
	setPower(0);
	delay(5000);
}

void Motor::setPower(int newPower) {
	int signal = map(newPower, FULLBACK, FULLFORWARD, MINSIGNAL, MAXSIGNAL);
	esc.writeMicroseconds(signal);
	power = newPower;
}

int Motor::getPower() {
	return power;
}