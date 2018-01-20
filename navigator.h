// navigator.h

#ifndef _navigator_h
#define _navigator_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Adafruit_GPS.h>


struct Waypoint {
	double lat, lon;
};

class Navigator {
public:
	Navigator(Waypoint* w, int);
	int getCourse();	// Returns course to next waypoint (compass bearings 0-360 with 0 true North)
	void update();		// Updates navigator (Call this frequently in the main loop)
	void diagnostic();


private:
	Waypoint * waypoints;	// Array of waypoints to pass through
	int pointCount, next;	// Number of waypoints, index of next waypoint
	int course;				// Last calculated course to waypoint

	bool checkWaypoint();
	void updateWaypoints();
	void startGPS();
	void updateGPS();
};


double degreesToRadians(double);
double radiansToDegrees(double);
int calculateCourse(double lat1, double lon1, double lat2, double lon2);
double calculateCoordinateDistance(double, double, double, double);
double getLatitude();
double getLongitude();

#endif

