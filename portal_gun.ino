#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

enum PinAssignments {
	encoderPinA = 2,
	encoderPinB = 3,
	 projectButton = 8
};

volatile unsigned int encoderPos = 0;
unsigned int lastReportedPos = 1;

boolean A_set = false;
boolean B_set = false;

Adafruit_7segment matrix = Adafruit_7segment();

uint8_t hexcharacters[] = {
	0x77, // A
	0x7C, // B
	0x39, // C
	0x5E, // D
	0x79, // E
	0x71  // F
};

long dimensionlist[] = {
	0x35C,
	0xC500,
	0xBEEF,
	0xB00B
};

// Update the display with the home dimension, a random pick from a list, or a random dimension.
void SetDisplay(Adafruit_7segment matrix, int seed) {
	// We want each position on the rotator to give us the same (random) result.
	// So if you click right to get a new value, then click back left, you should get the previous result.
	RandomSeed(seed);

	int arraylength = 0;

	// 25% chance to show the home dimension.	
	if(Random(4) == 0) {
		matrix.print(0xC137, HEX);
	}
	// If not, 20% chance to show a random pick
	else if(Random(5) == 0) {
		// Calculate the length of the array dynamically...
		arraylength = sizeof(dimensionlist) / sizeof(long);
		matrix.print(dimensionlist[Random(0, arraylength)]);
	}
	// If still not, just create a random dimension.
	else {	
		// Calculate the length of the array again
		arraylength = sizeof(hexcharacters) / sizeof(uint8_t);

		// Write the random dimension name to the display
		matrix.writeDigitRaw(0, hexcharacters[Random(0, arraylength)]);
		matrix.writeDigitNum(1, Random(0, 10));
		matrix.writeDigitNum(3, Random(0, 10));
		matrix.writeDigitNum(4, Random(0, 10));
	}
	
	// Push whatever we decided on to the display over I2C
	matrix.writeDisplay();
}

void doEncoderA() {
	A_set = digitalRead(encoderPinA) == HIGH;
	encoderPos += (A_set != B_set) ? +1 : -1;
}

void doEncoderB() {
	B_set = digitalRead(encoderPinB) == HIGH;
	encoderPos += (A_set == B_set) ? +1 : -1;
}

// At the moment this just returns whatever you give it, but is expandable to whatever algorithm you want
// to make some random seeds. For this to work it should be deterministic, though.
int GetNewSeedFromSelector(int input) {
	return input;
}

void ProjectImage() {
	// TODO: Actually tie together the projection. Maybe this doesn't need to do anything. We'll probably
	// end up just hooking up a transistor to illuminate the LEDs.
	return;
}

void Setup() {
	// Encoder stuff
	pinMode(encoderPinA, INPUT);
	pinMode(encoderPinB, INPUT);
	pinMode( projectButton, INPUT);

	digitalWrite(encoderPinA, HIGH);
	digitalWrite(encoderPinB, HIGH);
	digitalWrite(projectButton, HIGH);

	attachInterrupt(0, doEncoderA, CHANGE);
	attachInterrupt(1, doEncoderB, CHANGE);

	// Set up our 7-seg matrix
	matrix.begin(0x70);
}

void loop() {
	// If we twiddled the knob
	if(lastReportedPos != encoderPos) {
		int seedFromSelector = GetNewSeedFromSelector(encoderPos);
		SetDisplay(matrix, seedFromSelector);
		lastReportedPos = encoderPos;
	}

	// TODO: The project button is tied to input 8, so once we've worked out what we're doing, tie it together.
	if(digitalRead(projectButton) == HIGH) {
		ProjectImage();
	}
}


