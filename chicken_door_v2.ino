#include <SimpleTimer.h>
#include <LiquidCrystal_I2C.h>

byte upArrow[] = {
  0b00100,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b01110,
  0b01110,
  0b01110
};

byte downArrow[] = {
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00100
};

int photocellPin = 0;     // photocell is connected to a0
int photocellReading;     // the analog reading from the sensor photocell
int photocellReadingLevel = 1; // 0=dark, 1=light. Default to light out.
int photocellDebounceCount = 0; // How many times the photocell has been read
// 300 = ~5min w/ a 1sec loop 
int photocellDebounceCountDelay = 4200; // How many times to see a dark/light switch before committing

int potentiometerPin = 1; //analog pin a1
int potentiometerReading;

const int roomLightsPin1 = 2;
const int roomLightsPin2 = 3;
const int doorMotorPin1 = 4;
const int doorMotorPin2 = 5;

const int doorClosedSwitch = 6;
const int doorClosedLEDPin = 8;

// times are in ms
SimpleTimer internalLEDTimer(1000);
SimpleTimer photocellTimer(1000);
SimpleTimer interiorLightsTimer(36000000); // 10 hours


LiquidCrystal_I2C lcd(0x27,16,2); // 16 cols, 2 rows

void setup() {
  
  Serial.begin(9600);

  Serial.println("Hello!  Serial console open...");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(doorClosedLEDPin, OUTPUT);
  pinMode(roomLightsPin1, OUTPUT);
  pinMode(roomLightsPin2, OUTPUT);

  pinMode(doorClosedSwitch, INPUT);
  digitalWrite(doorClosedSwitch, HIGH);

  pinMode (doorMotorPin1, OUTPUT);
  pinMode (doorMotorPin2, OUTPUT);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
}

void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  
  readPhotocell();
  readPotentiometer(); 
  delay(100);
  doCoopDoor();
  
  if (interiorLightsTimer.isReady()) {
    if (!photocellReadingLevel) { // if it's not light out
      doRoomLights(true);
    }
  }

  updateLCD();
}
  
void doCoopDoor(){
  if (photocellReadingLevel == 0) {        // If it's dark
    startDoorMotor(0);
  } else if (photocellReadingLevel == 1) {
    startDoorMotor(1);
  }
}

void doRoomLights(bool on) {
  // This is powered off of the 2nd set of pins on the L298N.  
  // Setting both to the same value is off, setting one high and one low is on.
  if (on) {
      digitalWrite(roomLightsPin1, HIGH);
      digitalWrite(roomLightsPin2, LOW);
  } else {
      digitalWrite(roomLightsPin1, HIGH);
      digitalWrite(roomLightsPin2, HIGH);
  }
}

void readPhotocell() {
  photocellReading = map(analogRead(photocellPin), 0, 1023, 0, 100);
  if (photocellReading <= potentiometerReading) {
    if (photocellReadingLevel == 1) {
      Serial.print("  Light -> Dark change detected.  Debouncing...");
      if (++photocellDebounceCount >= photocellDebounceCountDelay) {
        photocellReadingLevel = 0;
        Serial.println("confirmed!");
        photocellDebounceCount = 0;
        interiorLightsTimer.reset();      // Reset the interior lights timer
      }
    }
  } else {
    if (photocellReadingLevel == 0) {
      Serial.print(" Dark -> Light change detected.  Debouncing...");
      if (++photocellDebounceCount >= photocellDebounceCountDelay) {
        photocellReadingLevel = 1;
        Serial.println("confirmed!");
        photocellDebounceCount = 0;
        doRoomLights(false);           // turns off the interior lights if they were on
      }
    }
  }
}

void readPotentiometer() {
  potentiometerReading = map(analogRead(potentiometerPin), 0, 1023, 100, 0);
}

void startDoorMotor(int direction) {
  if (direction) { // open
    digitalWrite(doorMotorPin1, LOW);
    digitalWrite(doorMotorPin2, HIGH);
    digitalWrite(doorClosedLEDPin, HIGH);
  } else { // close
    if (!digitalRead(doorClosedSwitch)) {
      digitalWrite (doorMotorPin1, HIGH);
      digitalWrite (doorMotorPin2, LOW);
      digitalWrite(doorClosedLEDPin, !digitalRead(doorClosedLEDPin));
    } else {
      digitalWrite(doorMotorPin1, HIGH);
      digitalWrite(doorMotorPin2, HIGH);
      digitalWrite(doorClosedLEDPin, LOW);
    }
  }
}

void updateLCD() {
  lcd.setCursor(0,0);
  
  if (photocellReadingLevel == 0) {   // if it's dark
    if (photocellDebounceCount > 0) {
      lcd.write(1);
      lcd.print(" DBNCE ");
      lcd.print(photocellDebounceCount);
      lcd.print("/");
      lcd.print(photocellDebounceCountDelay);

    } else {
      if (digitalRead(doorClosedSwitch)) {\
        lcd.write(1);
        lcd.print(" CLOSED        ");
      } else {
        lcd.write(1);
        lcd.print(" CLOSING       ");        
      }
    }
  } else if (photocellReadingLevel == 1) {
    if (photocellDebounceCount > 0) {
      lcd.write(0);
      lcd.print(" DBNCE ");
      lcd.print(photocellDebounceCount);
      lcd.print("/");
      lcd.print(photocellDebounceCountDelay);

    } else {                          
      lcd.write(0);
      lcd.print(" OPEN          ");
    }
   }
  lcd.setCursor(0,1);
  lcd.print("Light: ");
  //lcd.setCursor(7,1);
  lcd.print(photocellReading);
  lcd.print("  ");
  //lcd.setCursor(11,1);
  lcd.print(potentiometerReading);    
  lcd.print("   ");

}
