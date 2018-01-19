// navigation.h

#ifndef _NAVIGATION_h
#define _NAVIGATION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Adafruit_GPS.h>

void startGPS(Adafruit_GPS* gps);
void updateGPS();

struct Waypoint {
	double lat, lon;
};

class Navagator {
public:
	Navagator(Waypoint*, int);
	int getBearing();	// Returns angle to next waypoint (between 180 port and -180 starboard) 
	void updateWaypoints();

private:
	Waypoint* waypoints;
	int pointCount, next;
	int bearing;

	bool checkWaypoint();
};



#endif

