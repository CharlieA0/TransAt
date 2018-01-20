#include "motors.h"
#include "navigator.h"

Waypoint w = { 43.36, -75.1 };	// These objects must be accessible in both setup() and loop(), but we can't use the new keyword
Navigator* nav;					// That means we have to do a lot of the heavy lifting here

     
uint32_t timer = millis();

void setup() {
	Serial.begin(115200);
	Serial.println("MIT TransAt Vehicle Debugging");
	nav = new Navigator(&w, 1);	// And yet, it only works if I use the new keyword... ask some who knows more C++ about this.
}

void loop() 
{ 
	nav->update();

	if (timer > millis()) timer = millis();
   
	if (millis() - timer > 2000) {

		timer = millis(); // reset the timer
		nav->diagnostic();

		int course = nav->getCourse();
		Serial.println("\nCourse: "); Serial.println(course);
	}

}
