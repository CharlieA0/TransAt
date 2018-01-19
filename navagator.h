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
	int getBearing();	// Returns angle to next waypoint (between 180 port and -180 starboard) 
	void update();		// Updates navagator (Call this frequently in the main loop)

private:
	Waypoint* waypoints;	// Array of waypoints to pass through
	int pointCount, next;	// Number of waypoints, index of next waypoint
	int bearing;			// Last calculated bearing to waypoint

	bool checkWaypoint();
	void updateWaypoints();
};
#endif

