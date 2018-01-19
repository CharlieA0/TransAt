// 
// Library for navagator guiding the TransAt vehicle
// 
// Note:	The navagator uses 0-360 compass bearings with 0 equivalent to true North and assumes 1 degree of precision 
//			All distances are measured in km
//			The course is the ideal angle between the vehicle and a waypoint

#include "navagator.h"
#include <Adafruit_GPS.h>
#include <math.h>

#define EARTH_RADIUS 6371 // in km
#define WAYPOINT_RADIUS 5 // in km

HardwareSerial GPSSerial = Serial3;
Adafruit_GPS GPS(&GPSSerial);

Navagator::Navagator(Waypoint* w, int count) {
	waypoints = w; pointCount = count;
	next = 0;		// start with first waypoint
	course = 90;	// Notice that initial course is due East

	startGPS();
}

// Returns course between 0-360 degrees, where 0 is true North
int Navagator::getCourse() {
	if (!GPS.fix)	// Without a fix, continue along last course.
		return course;

	// Get coordinates (In decimal degrees)
	double waypointLon = (waypoints + next)->lon;
	double waypointLat =(waypoints + next)->lat;
	double currentLon = GPS.longitude;
	double currentLat = GPS.latitude;

	// Update course
	course = calculateCourse(currentLat, currentLon, waypointLat, waypointLon);

	return course;
}

// Passed coordinates in decimal degrees
// Returns course in integer compass bearings
double calculateCourse(double lat1, double lon1, double lat2, double lon2) {
	// From https://www.movable-type.co.uk/scripts/latlong.html
	
	// Convert to radians
	lat1 = degreesToRadians(lat1);
	lon1 = degreesToRadians(lon1);
	lat2 = degreesToRadians(lat2);
	lon2 = degreesToRadians(lon2);

	double y = sin(lon2 - lon1) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1);
	double course = ((int)radiansToDegrees(atan2(y, x)) + 360) % 360;				// We convert the -pi to pi result of atan2 to 360 degree compass bearings
	
	return course;
}

// Call this from the main loop to keep navigation up to date
void Navagator::update() {
	updateGPS();
	updateWaypoints();
}

void Navagator::updateWaypoints() {
	if (checkWaypoint()) {			// If we've reached a waypoint

		if (next + 1 < pointCount)	// If there are more waypoints,
			next++;					// Move to the next waypoint (Else, keep seeking the last waypoint)
	}
}

// Returns true if we've reached the next waypoint, else false
bool Navagator::checkWaypoint() {
	if (!GPS.fix)		// Without a fix, assume we haven't reached a waypoint
		return false;

	// Get coordinates
	double waypointLon = (waypoints + next)->lon;
	double waypointLat = (waypoints + next)->lat;
	double currentLon = GPS.longitude;
	double currentLat = GPS.latitude;

	// Return if distance between them is less than the waypoint 
	return calculateCoordinateDistance(currentLat, currentLon, waypointLat, waypointLon) < WAYPOINT_RADIUS;
}




// Takes coordinates in decimal degrees
// Returns distance between in km
double calculateCoordinateDistance(double lat1, double lon1, double lat2, double lon2) {
	// Halversine Formula https://www.movable-type.co.uk/scripts/latlong.html

	// Convert to radians
	lat1 = degreesToRadians(lat1);
	lon1 = degreesToRadians(lon1);
	lat2 = degreesToRadians(lat2);
	lon2 = degreesToRadians(lon2);

	// Find distance on unit sphere
	double a = pow(sin((lat2 - lat1) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((lon2 - lon1) / 2), 2);	// For right triangle with sides a, b, and c
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return EARTH_RADIUS * c;
}

double degreesToRadians(double degrees) {
	return degrees * M_PI / 180;
}

double radiansToDegrees(double radians) {
	return radians * 180 / M_PI;
}

// Configures and begins reading GPS
void Navagator::startGPS() {

	GPS.begin(9600);

	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);	// Turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);		// 1 Hz update rate	(In final version, these rates should be lowered and the interupt used less frequently to conserve clock cycles)
	GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);		// 1 Hz fix update rate

	// Create interrupt
	OCR0A = 0xAF;			// We'll use comparison register A (OCR0A) for the ATMega's ~1Hz Timer0 (When Timer0 == OCR0A == 0xAF, the interupt will fire)
	TIMSK0 |= _BV(OCIE0A);	// enable compare A interrupts 
							// Note: TIMSK0 is a macro for the 'Timer Interrupt Mask Register' and OCIE0A is the bit mask specifing 'Timer/Counter Output Compare Match A Interrupt'

	// Wait for GPS to get fix
	while(!GPS.fix) 
		update();
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
	GPS.read();	// read any new GPS data (We still have to parse completed sentences in the main loop)
}

// Parses new NMEA sentences
void updateGPS() {
	if (GPS.newNMEAreceived())
		GPS.parse(GPS.lastNMEA());
}
