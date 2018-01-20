// 
// Library for navigator guiding the TransAt vehicle
// 
// Note:	The navigator uses 0-360 compass bearings with 0 equivalent to true North and assumes 1 degree of precision 
//			All distances are measured in km
//			The course is the ideal angle between the vehicle and a waypoint

#include "navigator.h"
#include <math.h>

#define EARTH_RADIUS 6371 // in km
#define WAYPOINT_RADIUS 5 // in km

#include <Adafruit_GPS.h>
#define GPSSerial Serial3
Adafruit_GPS GPS(&GPSSerial);

Navigator::Navigator(Waypoint* w, int count) {
	waypoints = w; 
	pointCount = count;
	next = 0;								// start with first waypoint
	course = 90;							// Notice that initial course is due East

	Serial.println('here');

	startGPS();
}

// Returns course between 0-360 degrees, where 0 is true North
int Navigator::getCourse() {
	if (!GPS.fix)	// Without a fix, continue along last course.
		return course;

	// Get coordinates (In decimal degrees)
	double waypointLon = waypoints->lon;
	double waypointLat = waypoints->lat;
	double currentLon = getLongitude();
	double currentLat = getLatitude();

	Serial.print("\nCurrent Loc: "); Serial.print(currentLat);
	Serial.print(", "); Serial.println(currentLon);
	Serial.print("Waypoint Loc: "); Serial.print(waypointLat);
	Serial.print(", "); Serial.println(waypointLon);

	// Update course
	course = calculateCourse(currentLat, currentLon, waypointLat, waypointLon);
	return course;
}

// Passed coordinates in decimal degrees
// Returns course in integer compass bearings
int  calculateCourse(double lat1, double lon1, double lat2, double lon2) {
	// From https://www.movable-type.co.uk/scripts/latlong.html
	
	// Convert to radians
	lat1 = degreesToRadians(lat1);
	lon1 = degreesToRadians(lon1);
	lat2 = degreesToRadians(lat2);
	lon2 = degreesToRadians(lon2);

	double y = sin(lon2 - lon1) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1);
	int course = ((int)radiansToDegrees(atan2(y, x)) + 360) % 360;				// We convert the -pi to pi result of atan2 to 360 degree compass bearings
	
	return course;
}

// Call this from the main loop to keep navigation up to date
void Navigator::update() {
	updateGPS();
	updateWaypoints();
}

void Navigator::updateWaypoints() {
	if (checkWaypoint()) {			// If we've reached a waypoint

		if (next + 1 < pointCount)	// If there are more waypoints,
			next++;					// Move to the next waypoint (Else, keep seeking the last waypoint)
	}
}

// Returns true if we've reached the next waypoint, else false
bool Navigator::checkWaypoint() {
	if (!GPS.fix)		// Without a fix, assume we haven't reached a waypoint
		return false;

	// Get coordinates
	double waypointLon = (waypoints + next)->lon;
	double waypointLat = (waypoints + next)->lat;
	double currentLon = getLongitude();
	double currentLat = getLatitude();

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

// Provides decimal degree latitude with .01 degree of precision
double getLatitude() {
	double lat = GPS.latitude_fixed;
	
	if (GPS.lat == 'S') {	// Note we're doing a character comparison here so the == is appropriate
		lat = -lat;			// Southern latitudes are negative
	}

	return lat / 10000000.0;
}

// Provides decimal degree latitude with .01 degree of precision
double getLongitude() {
	double lon = GPS.longitude_fixed;

	if (GPS.lon == 'W') {	// Note we're doing a character comparison here so the == is appropriate
		lon = -lon;			// Western longitudes are negative
	}

	return lon / 10000000.0;
}

// Configures and begins reading GPS
void Navigator::startGPS() {
	GPS.begin(9600);

	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);	// Turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);		// 1 Hz update rate	(In final version, these rates should be lowered and the interupt used less frequently to conserve clock cycles)
    delay(1000);

	// Create interrupt
	OCR0A = 0xAF;			// We'll use comparison register A (OCR0A) for the ATMega's ~1Hz Timer0 (When Timer0 == OCR0A == 0xAF, the interupt will fire)
	TIMSK0 |= _BV(OCIE0A);	// enable compare A interrupts 
							// Note: TIMSK0 is a macro for the 'Timer Interrupt Mask Register' and OCIE0A is the bit mask specifing 'Timer/Counter Output Compare Match A Interrupt'

	// Wait for GPS to get fix
	// Actually we can't do that
	// Trying to loop here produces weird behavior where nothing is printed to Serial or we never get any data from the gps
	// Possibly try to figure out what is going on later?
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
	GPS.read();	// read any new GPS data (We still have to parse completed sentences in the main loop)
}

// Parses new NMEA sentences
void Navigator::updateGPS() {
	if (GPS.newNMEAreceived())
		GPS.parse(GPS.lastNMEA());
}

void Navigator::diagnostic() {

	Serial.println("\n\nNavigation Diagnostic");
	Serial.print("Time: ");
	Serial.print(GPS.hour, DEC); Serial.print(':');
	Serial.print(GPS.minute, DEC); Serial.print(':');
	Serial.print(GPS.seconds, DEC); Serial.print('.');
	Serial.println(GPS.milliseconds);
	Serial.print("Date: ");
	Serial.print(GPS.day, DEC); Serial.print('/');
	Serial.print(GPS.month, DEC); Serial.print("/20");
	Serial.println(GPS.year, DEC);
	Serial.print("Fix: "); Serial.print((int)GPS.fix);
	Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
	if (GPS.fix) {
		Serial.print("Location: ");
		Serial.print(getLatitude()); Serial.print(GPS.lat);	// Decimal Degrees (1/10,000,000 of a degree)
		Serial.print(", ");
		Serial.print(getLongitude()); Serial.println(GPS.lon); // Decimal Degrees (1/10,000,000 of a degree)
		Serial.print("Speed (knots): "); Serial.println(GPS.speed);
		Serial.print("Angle: "); Serial.println(GPS.angle);
		Serial.print("Altitude: "); Serial.println(GPS.altitude);
		Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
	}
	Serial.print("Waypoint Number: "); Serial.println(next);

}
