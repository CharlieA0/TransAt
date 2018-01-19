// navagator.h

#ifndef _NAVAGATOR_h
#define _NAVAGATOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

struct Waypoint {
	double lat, lon;
};

class Navagator {
public:
	Navagator(Waypoint*, int);
	int getCourse();	// Returns course to next waypoint (compass bearings 0-360 with 0 true North)
	void update();		// Updates navagator (Call this frequently in the main loop)

private:
	Waypoint* waypoints;	// Array of waypoints to pass through
	int pointCount, next;	// Number of waypoints, index of next waypoint
	int course;				// Last calculated course to waypoint

	bool checkWaypoint();
	void updateWaypoints();
	void startGPS();
};
#endif

