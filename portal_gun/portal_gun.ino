#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Digital pins 2 and 3 are the only hardware interrupt pins on the UNO. For the micro this will need to changed since
// the Micro uses 2 and 3 for SDA/SCL for I2C (for driving the 7-seg matrix)
// For the Uno this isn't a problem since SDA/SCL are on A4 and A5 respectively.
enum PinAssignments {
	encoderPinA = 0,
	encoderPinB = 1,
	projectButton = 8,
        projectGatePin = 7
};

volatile byte encoderPos = 0;
volatile byte lastReportedPos = 0;

volatile byte aFlag = 0;
volatile byte bFlag = 0;

volatile byte reading = 0;

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
	0x035C,
	0xC500,
	0xBEEF,
	0xB00B
};

// Update the display with the home dimension, a random pick from a list, or a random dimension.
void SetDisplay(Adafruit_7segment matrix, int seed) {
	// We want each position on the rotator to give us the same (random) result.
	// So if you click right to get a new value, then click back left, you should get the previous result.
	randomSeed(seed);

	int arraylength = 0;

	// 25% chance to show the home dimension.	
	if(random(4) == 0) {
		matrix.print(0xC137, HEX);
	}
	// If not, 20% chance to show a random pick
	else if(random(5) == 0) {
		// Calculate the length of the array dynamically...
		arraylength = sizeof(dimensionlist) / sizeof(long);
		matrix.print(dimensionlist[random(0, arraylength)], HEX);
	}
	// If still not, just create a random dimension.
	else {	
		// Calculate the length of the array again
		arraylength = sizeof(hexcharacters) / sizeof(uint8_t);

		// Write the random dimension name to the display
		matrix.writeDigitRaw(0, hexcharacters[random(0, arraylength)]);
		matrix.writeDigitNum(1, random(0, 10));
		matrix.writeDigitNum(3, random(0, 10));
		matrix.writeDigitNum(4, random(0, 10));
	}
	
	// Push whatever we decided on to the display over I2C
	matrix.writeDisplay();
}

void doEncoderA() {
  cli();
  reading = PIND & 0xC;
  if (reading == B00001100 && aFlag) {
    encoderPos--;
    bFlag = 0;
    aFlag = 0;
  } 
  else if (reading == B00000100) {
    bFlag = 1;
  }
  sei();
}

void doEncoderB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
    sei(); //restart interrupts
  }

// At the moment this just returns whatever you give it, but is expandable to whatever algorithm you want
// to make some random seeds. For this to work it should be deterministic, though.
int GetNewSeedFromSelector(int input) {
	return 1000 + input;
}

void ProjectImage() {
	digitalWrite(projectGatePin, HIGH);
	return;
}

void StopProjectImage() {
        digitalWrite(projectGatePin, LOW);
}

void setup() {
	// Encoder stuff
	pinMode(encoderPinA, INPUT_PULLUP);
	pinMode(encoderPinB, INPUT_PULLUP);
	pinMode(projectButton, INPUT);
        pinMode(projectGatePin, OUTPUT);
        digitalWrite(projectGatePin, LOW);
        
	attachInterrupt(digitalPinToInterrupt(encoderPinA), doEncoderA, RISING);
	attachInterrupt(digitalPinToInterrupt(encoderPinB), doEncoderB, RISING);

	// Set up our 7-seg matrix
	matrix.begin(0x70);
        matrix.print(0xC137, HEX);
        matrix.writeDisplay();
        
        // Debug
        Serial.begin(9600);
}

void loop() {
	// If we twiddled the knob
	if(lastReportedPos != encoderPos) {
		int seedFromSelector = GetNewSeedFromSelector(encoderPos);
		SetDisplay(matrix, seedFromSelector);
		lastReportedPos = encoderPos;
	}

	// TODO: The project button is tied to a digital pin, so once we've worked out what we're doing, tie it together.
	if(digitalRead(projectButton) == HIGH) {
                Serial.println("HIGH");
		ProjectImage();
	}
        else {
                StopProjectImage();
                Serial.println("LOW");
        }
}


