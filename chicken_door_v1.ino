#include <SimpleTimer.h>

const boolean SerialDisplay = true;

int photocellPin = 0;     // photocell is connected to a0
int photocellReading;     // the analog reading from the sensor photocell
int photocellReadingThreshold = 350; // What level do you consider dark?
//int photocellReadingThreshold = 10; // What level do you consider dark?

int photocellReadingLevel = 1; // 0=dark, 1=light. Default to light out.
int photocellDebounceCount = 0; // How many times the photocell has been read
// 300 = ~5min w/ a 1sec loop
int photocellDebounceCountDelay = 300; // How many times to see a dark/light switch before agreeing

// motor pins
const int enableDoorMotorPin = 44;
const int directionCloseDoorMotorPin = 40;
const int directionOpenDoorMotorPin = 42;

// top switch
const int topSwitchPin = 51;
int topSwitchPinVal;                   // top switch var for reading the pin status
int topSwitchPinVal2;                  // top switch var for reading the pin delay/debounce status
int topSwitchState;                    // top switch var for to hold the switch state

// bottom switch
const int bottomSwitchPin = 50;
int bottomSwitchPinVal;                // bottom switch var for reading the pin status
int bottomSwitchPinVal2;               // bottom switch var for reading the pin delay/debounce status
int bottomSwitchState;                 // bottom switch var for to hold the switch state

// debounce delay
long lastDebounceTime = 0;
long debounceDelay = 10;

const int doorOpenLEDPin = 30;              // led set to digital pin 40
const int doorClosedLEDPin = 28;            // led set to digital pin 41
const int interiorLightsPin = 29;

int internalLEDState = LOW;

unsigned long coopDoorLedBlinkTime = 0;
const int coopDoorLedBlinkDelay = 500;
int coopDoorLedBlinkState = LOW;

int coopPhotoCellTimerCount = 0;

int doorCloseDelay = 0;

// times are in ms
SimpleTimer internalLEDTimer(2000);
SimpleTimer photocellTimer(1000);
SimpleTimer interiorLightsTimer(36000000);

void setup() {

  Serial.begin(9600);

  if (SerialDisplay) {
    Serial.println("Hello!  Serial console open...");
  }

  pinMode(LED_BUILTIN, OUTPUT);

  // door motor
  pinMode (enableDoorMotorPin, OUTPUT);           // enable motor pin = output
  pinMode (directionCloseDoorMotorPin, OUTPUT);   // motor close direction pin = output
  pinMode (directionOpenDoorMotorPin, OUTPUT);    // motor open direction pin = output

  // bottom switch
  pinMode(bottomSwitchPin, INPUT);                  // set bottom switch pin as input
  digitalWrite(bottomSwitchPin, HIGH);              // activate bottom switch resistor

  // top switch
  pinMode(topSwitchPin, INPUT);                     // set top switch pin as input
  digitalWrite(topSwitchPin, HIGH);                 // activate top switch resistor

  // lights
  pinMode(doorOpenLEDPin, OUTPUT);
  pinMode(doorClosedLEDPin, OUTPUT);
  pinMode(interiorLightsPin, OUTPUT);
}

void debounceBottomReedSwitch() {
  bottomSwitchPinVal = digitalRead(bottomSwitchPin);
  if ((millis() - lastDebounceTime) > debounceDelay) {    // delay 10ms for consistent readings
    bottomSwitchPinVal2 = digitalRead(bottomSwitchPin);    // read input value again to check or bounce
    if (bottomSwitchPinVal == bottomSwitchPinVal2) {       // make sure we have 2 consistant readings
      if (bottomSwitchPinVal != bottomSwitchState) {       // the switch state has changed!
        bottomSwitchState = bottomSwitchPinVal;
      }
    }
  }
}

void debounceTopReedSwitch() {
  topSwitchPinVal = digitalRead(topSwitchPin);             // read input value and store it in val
  if ((millis() - lastDebounceTime) > debounceDelay) {     // delay 10ms for consistent readings
    topSwitchPinVal2 = digitalRead(topSwitchPin);          // read input value again to check or bounce
    if (topSwitchPinVal == topSwitchPinVal2) {             // make sure we have 2 consistant readings
      if (topSwitchPinVal != topSwitchState) {             // the button state has changed!
        topSwitchState = topSwitchPinVal;
      }
    }
  }
}

void stopDoorMotor() {
  // A cheesy software hack for a hardware problem.  The bottom magnet is a
  // little high so the door closes but doesn't lock.  This delays so the locks
  // engage.
  if (doorCloseDelay) {
    delay(doorCloseDelay);
    doorCloseDelay = 0;
  }
  digitalWrite (directionCloseDoorMotorPin, LOW);      // turn off motor close direction
  digitalWrite (directionOpenDoorMotorPin, LOW);       // turn on motor open direction
  analogWrite (enableDoorMotorPin, 0);                 // enable motor, 0 speed
}

void startDoorMotor(int direction) {
  if (direction) { // open
    digitalWrite(directionCloseDoorMotorPin, LOW);       // turn off motor close direction
    digitalWrite(directionOpenDoorMotorPin, HIGH);       // turn on motor open direction
    analogWrite(enableDoorMotorPin, 255);                // enable motor, full speed

  } else { // close
    digitalWrite (directionCloseDoorMotorPin, HIGH);     // turn on motor close direction
    digitalWrite (directionOpenDoorMotorPin, LOW);       // turn off motor open direction
    analogWrite (enableDoorMotorPin, 255);               // enable motor, full speed
  }
}

void doCoopDoor() {
  if (photocellReadingLevel == 0) {   // if it's dark
    if (bottomSwitchState != 0) {    // if the door isn't closed
      Serial.println(" The coop door is open and we'd like to close it.");
      startDoorMotor(0);
      doorCloseDelay = 5000;
    } else {
      stopDoorMotor();
    }
  } else if (photocellReadingLevel == 1) {
    if (topSwitchState != 0) {    // if the door isn't closed
      Serial.println(" The coop door is closed and we'd like to open it.");
      startDoorMotor(1);
    } else {
      stopDoorMotor();
    }
  }
}

//  coop door status: red if open, green if closed, blinking red if stuck
void doCoopDoorLed() {
  if (bottomSwitchState == 0) {                 // if bottom reed switch circuit is closed
    digitalWrite (doorClosedLEDPin, HIGH);          // turns on doorClosedLEDPin (red)
    digitalWrite (doorOpenLEDPin, LOW);             // turns off doorOpenLEDPin (green)
  } else if (topSwitchState == 0) {             // if top reed switch circuit is closed
    digitalWrite (doorClosedLEDPin, LOW);           // turns off doorClosedLEDPin (red)
    digitalWrite (doorOpenLEDPin, HIGH);            // turns on doorOpenLEDPin (green)
  } else {
    // Neither switch is active so we're either transitioning or something is
    // broken.  Either way, lets blink the red LED.
    if (millis() - coopDoorLedBlinkTime >= coopDoorLedBlinkDelay) {
      coopDoorLedBlinkTime = millis();
      if (coopDoorLedBlinkState == HIGH) {
        coopDoorLedBlinkState = LOW;
      } else {
        coopDoorLedBlinkState = HIGH;
      }
      digitalWrite (doorClosedLEDPin, coopDoorLedBlinkState);
    }
  }
}

void readPhotocell() {

  photocellReading = analogRead(photocellPin);

  if (photocellReading <= photocellReadingThreshold) {
    if (photocellReadingLevel == 1) {
      Serial.print("  Light -> Dark change detected.  Debouncing...");
      if (++photocellDebounceCount >= photocellDebounceCountDelay) {
        photocellReadingLevel = 0;
        Serial.print("confirmed!");
        photocellDebounceCount = 0;
        interiorLightsTimer.reset();                    // Reset the interior lights timer
      }
      Serial.println();
    }
  } else {
    if (photocellReadingLevel == 0) {
      Serial.print(" Dark -> Light change detected.  Debouncing...");
      if (++photocellDebounceCount >= photocellDebounceCountDelay) {
        photocellReadingLevel = 1;
        Serial.print("confirmed!");
        photocellDebounceCount = 0;
        digitalWrite(interiorLightsPin, LOW);           // turns off the interior lights if they were on
      }
      Serial.println();
    }
  }
}

void loop() {

  if (SerialDisplay) {
    Serial.print("Top switch: ");
    Serial.print(topSwitchState);
    Serial.print(" Bottom switch: ");
    Serial.print(bottomSwitchState);
    Serial.print(" Photosensor: ");
    Serial.print(photocellReading);
    Serial.print(" (/");
    Serial.print(photocellReadingThreshold);
    Serial.print(")");
    Serial.print(" Debounce: ");
    Serial.print(photocellDebounceCount);
    Serial.print("/");
    Serial.print(photocellDebounceCountDelay);
    Serial.println();
  }

  // Flash the internal LED to let us know things are running
  if (internalLEDTimer.isReady()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    internalLEDTimer.reset();
  }

  if (photocellTimer.isReady()) {
    readPhotocell();
    photocellTimer.reset();
  }

  if (interiorLightsTimer.isReady()) {
    if (!photocellReadingLevel) { // if it's not light out
      digitalWrite(interiorLightsPin, HIGH);
    } else {
      digitalWrite(interiorLightsPin, LOW);
    }
    interiorLightsTimer.reset();
  }

  debounceTopReedSwitch();
  debounceBottomReedSwitch();

  doCoopDoor();
  doCoopDoorLed();

  delay(100);
}
