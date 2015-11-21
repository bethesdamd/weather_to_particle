/* Read soil moisture sensor and photoresistor and write values to serial comms and particle, etc.
 openweathermap.org bethesda city id = 4348599
 http://api.openweathermap.org/data/2.5/forecast?id=<id>
 To see events in the terminal: `particle subscribe davidws:moisture`
 The moisture event is currently picked up by IFTTT.com and it sends it to a Google spreadsheet
 Particle.publish("davidws:light", String(light), 240, PRIVATE);
 To see Serial output in terminal:  `particle serial monitor /dev/tty.usbmodem1411`
 First use `particle serial list` to see my online device address(es)
*/

float TOP_SCALE_NUMBER = 70.0;
float BOTTOM_SCALE_NUMBER = 25.0;
float TOP_SCALE_STEPPER_POSITION = 6700.0;
float BOTTOM_SCALE_STEPPER_POSITION = 0.0;
int UP = HIGH;
int DOWN = LOW;
int powerOut = A5;
int readMoisture = A0;
int analogValue;
int light;
int ledBoard = D7;
int readLight = A1;
int minutesDelay = 2;
int position = 0;  // represents the current stepper position
int step = A2;
int stepperDirection = A3;
int button = D1;
int last = 0;
int m;
int buttonStatus = 0;
int direction = A3;
bool buttonChange = false;
bool buttonTurnedOff = false; // calibration momentary pushbutton was released
void buttonChanged(void);

void setup()
{
    attachInterrupt(button, buttonChanged, CHANGE);
    pinMode(button, INPUT);
    pinMode(powerOut, OUTPUT);
    pinMode(ledBoard, OUTPUT);
    pinMode(readLight, INPUT);
    pinMode(readMoisture, INPUT);
    pinMode(step, OUTPUT);
    pinMode(direction, OUTPUT);

    // Just testing: Publish value to a rest endpoint, also get this via: particle variable get aardvark_crazy analogValue
    // or, HTTP GET /v1/devices/{DEVICE_ID}/{VARIABLE}
    Particle.variable("analogValue", analogValue);

    // This was just a test: from a terminal enter `particle call aardvark_crazy blink` and it will blink the built-in
    // LED on the Particle
    Particle.function("blink", blinkMultiple);
    Particle.function("remoteMove", remoteMove);
    Particle.function("remoteTemp", remoteTemp);

    // To monitor, e.g.: screen /dev/cu.usbmodem1411 9600
    Serial.begin(9600);   // open serial over USB
    Serial.println("Getting started...");
}

void loop() {
    m = millis();
    if (m - last > (1000 * 60 * minutesDelay)) {
        last = m;
        blinkLed(5);  // To indicate that a reading is about to take place

        // Provide power to the moisture sensor
        digitalWrite(powerOut, HIGH);
        delay(50);  // just in case the sensor needs a little time to power up
        analogValue = analogRead(readMoisture);
        digitalWrite(powerOut, LOW);  // turn off the moisture sensor when not taking a reading, to prevent oxidation
        light = analogRead(readLight); // read the CdS light cell, just for fun

    }
    // This is the calibration button.  Hold it until reaching the calibration point, then released
    // This resets the zero-location.
    if (buttonStatus == HIGH) {
      move(1, DOWN);
    }

    if (buttonTurnedOff) {
      position = 0;
      buttonTurnedOff = false;
    }
}

// Particle functions must take a String as an argument and return an int
int blinkMultiple(String dummy) {
    blinkLed(3);
    return 1;
}

// Blink the onboard LED at D7
void blinkLed(int c) {
    for (int i = 0; i < c; i++) {
        digitalWrite(ledBoard, HIGH);
        delay(200);
        digitalWrite(ledBoard, LOW);
        delay(200);
    }
}

// move stepper n steps in dir direction
// dir = HIGH is CCW when looking down shaft towards motor
void move(int n, int dir) {
    digitalWrite(direction, dir);
    for(int i = 0; i < n; i++) {
      digitalWrite(step, UP);
      delay(1);
      digitalWrite(step, DOWN);
      delay(1);
    }
    if (dir == UP) {
      position += n;
    } else {
      position -= n;
    }
    // Serial.println(position);
    Particle.publish("davidws:testing", String(position), 60, PRIVATE);
}
// Move stepper to a particular position
void moveToPosition(int p) {
    int dir = DOWN;
    int delta = p - position;
    int pol = abs(delta) / delta;
    if (pol > 0) {
      dir= UP;
    }
    move(abs(delta), dir);
}

// Remotely tell indicator to move to a particular temperature
int remoteTemp(String temperature) {
  int x = ((temperature.toFloat() - BOTTOM_SCALE_NUMBER) / (TOP_SCALE_NUMBER - BOTTOM_SCALE_NUMBER)) * (TOP_SCALE_STEPPER_POSITION - BOTTOM_SCALE_STEPPER_POSITION);
  moveToPosition((int) x);
}

int remoteMove(String i) {
  moveToPosition(i.toInt());
}

void buttonChanged() {
  buttonStatus = digitalRead(button);
  if (buttonStatus == LOW) {
    buttonTurnedOff = true;
  }
}
