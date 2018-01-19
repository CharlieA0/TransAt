// 
// Library for TransAt vechicle's navigation
// 

#include "navigation.h"
#include <math.h>

#define EARTH_RADIUS 6371 // in km
#define WAYPOINT_RADIUS 5 // in km

Adafruit_GPS* gpsPointer; 

// Configures and begins reading GPS
void startGPS(Adafruit_GPS* gps) {

	gpsPointer = gps;

	(*gps).begin(9600);
	
	(*gps).sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);	// Turn on RMC (recommended minimum) and GGA (fix data) including altitude
	(*gps).sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);		// 1 Hz update rate	(In final version, these rates should be lowered and the interupt used less frequently to conserve clock cycles)
	(*gps).sendCommand(PMTK_API_SET_FIX_CTL_1HZ);		// 1 Hz fix update rate

	// Create interrupt
	OCR0A = 0xAF;			// We'll use comparison register A (OCR0A) for the ATMega's ~1Hz Timer0 (When Timer0 == OCR0A == 0xAF, the interupt will fire)
	TIMSK0 |= _BV(OCIE0A);	// enable compare A interrupts 
							// Note: TIMSK0 is a macro for the 'Timer Interrupt Mask Register' and OCIE0A is the bit mask specifing 'Timer/Counter Output Compare Match A Interrupt'
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
	(*gpsPointer).read();	// read any new GPS data (We still have to parse completed sentences in the main loop)
}

// Parses new NMEA sentences
void updateGPS() {
	if ((*gpsPointer).newNMEAreceived())
		(*gpsPointer).parse((*gpsPointer).lastNMEA());
}

Navagator::Navagator(Waypoint* w, int count) {
	waypoints = w;
	pointCount = count;
	next = 0;
	bearing = 0;	// default to straight forward
}

// Returns bearing to next waypoint in degrees

int Navagator::getBearing() {
	if (!(*gpsPointer).fix)	// Without a fix, continue along last bearing.
		return bearing;

	// Get coordinates to find bearing between (in radians)
	double waypointLon = degreesToRadians((waypoints + next)->lon);
	double waypointLat = degreesToRadians((waypoints + next)->lat);
	double currentLon = degreesToRadians((*gpsPointer).longitude);
	double currentLat = degreesToRadians((*gpsPointer).latitude);

	bearing = radiansToDegrees(calculateBearing(currentLat, currentLon, waypointLat, waypointLon))

	return bearing;
}

void Navagator::updateWaypoints() {
	if (checkWaypoint()) {	// If we've reached a waypoint
		if (next + 1 < pointCount)
			next++;	// Move to the next waypoint
		else {
			// Keep seeking the last waypoint
		}
	}
}

// Returns true if we've reached the next waypoint, else false
bool Navagator::checkWaypoint() {
	if (!(*gpsPointer).fix)	// Without a fix, assume we haven't reached a waypoint
		return false;

	// Get coordinates in radians
	double waypointLon = degreesToRadians((waypoints + next)->lon);
	double waypointLat = degreesToRadians((waypoints + next)->lat);
	double currentLon = degreesToRadians((*gpsPointer).longitude);
	double currentLat = degreesToRadians((*gpsPointer).latitude);

	//Find distance between  them
	return calculateCoordinateDistance(currentLat, currentLon, waypointLat, waypointLon) < WAYPOINT_RADIUS;
}


// Takes lattitudes and longitudes in radians
// Returns bearing in radians
double calculateBearing(double lat1, double lon1, double lat2, double lon2) {
	// From https://www.movable-type.co.uk/scripts/latlong.html
	double y = sin(lon2 - lon1) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1);
	return atan2(y, x);
}

// Takes coordinates in radians
// Returns distance between in km
double calculateCoordinateDistance(double lat1, double lon1, double lat2, double lon2) {
	// Halversine Formula https://www.movable-type.co.uk/scripts/latlong.html
	double a = pow(sin((lat2 - lat1) / 2), 2) + cos(lon1) * cos(lon2) * pow(sin((lon2 - lon1) / 2), 2);	// For right triangle with sides a, b, and c
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return EARTH_RADIUS * c;
}

double degreesToRadians(double degrees) {
	return degrees * M_PI / 180;
}

double radiansToDegrees(double radians) {
	return radians * 180 / M_PI;
}
