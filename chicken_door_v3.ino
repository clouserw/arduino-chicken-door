// Pro Mini Atmega 328P

#include "LowPower.h"

const int DEBUG = 0;         // DEBUG; if set to 1, will write values back via serial
const int DOORDELAY = 15000; // How long it takes to manipulate the door in ms

const int photocellPin = A0;
const int potPin = A2;
const int doorMotorPin1 = 2;
const int doorMotorPin2 = 3;

int doorstate = -1;   // -1 = unknown, 0 = closed, 1 = open
int photocellVal = 0; // the analog reading from the sensor photocell
int potVal = 0;       // analog reading from potentiometer
int sleeper = 0;      // Just a variable to track sleep cycles

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(photocellPin, INPUT);
  pinMode(doorMotorPin1, OUTPUT);
  pinMode(doorMotorPin2, OUTPUT);

  if (DEBUG) {
    Serial.begin(9600);
  }

  // We need to start in a known state.  Let's close the door.
  digitalWrite(doorMotorPin1, LOW);
  digitalWrite(doorMotorPin2, HIGH);
  doorstate = 0;
  delay(DOORDELAY);

}

void loop() {

  // If we're in debug mode (1) Sleep for 1s instead of 8s, (2) Don't disable the serial monitor
  if (DEBUG) {
    Serial.println("Skipping low power state because DEBUG is on.");
    delay(1000);
    sleeper = sleeper + 100;
  } else {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    sleeper = sleeper + 1;
  }


  // LowPower.h can only sleep for a max of 8s.  So, we wake up every 8s but only do work periodically, 8 * this value.  256 = about 4 minutes
  if (sleeper >= 1) {
    sleeper = 0;

    // The console is broken with power management so this is really our only indicator it works with DEBUG off
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    potVal =  map(analogRead(potPin), 0, 1023, 100, 0);  // This is wired backwards so fixing in software...
    photocellVal = map(analogRead(photocellPin), 0, 1023, 0, 100);  // I'm mapping this to 200 because that will stretch out the dawn/dusk time which I'm interested in.

    if ((potVal > (photocellVal+3)) && doorstate == 1) {
      // door close
      digitalWrite(doorMotorPin1, LOW);
      digitalWrite(doorMotorPin2, HIGH);
      doorstate = 0;
      delay(DOORDELAY);
    } else if (potVal <= (photocellVal-3) && doorstate == 0) {
      // door open
      digitalWrite(doorMotorPin1, HIGH);
      digitalWrite(doorMotorPin2, LOW);
      doorstate = 1;
      delay(DOORDELAY);
    }

    if (DEBUG) {
      Serial.print("Pot: ");
      Serial.print(potVal);
      Serial.print("   Photocell: ");
      Serial.print(photocellVal);
      Serial.print("   Door state: ");
      Serial.print(doorstate);
      Serial.println();
    }

  } // end of sleeper function
}
