// Read soil moisture sensor and photoresistor and write values to serial comms and particle, etc.
// openweathermap.org bethesda city id = 4348599
// http://api.openweathermap.org/data/2.5/forecast?id=<id>

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
    //digitalWrite(stepper, LOW);

    // Just a test:
    // Publish value to a rest endpoint, also get this via: particle variable get aardvark_crazy analogValue
    // or, HTTP GET /v1/devices/{DEVICE_ID}/{VARIABLE}
    Particle.variable("analogValue", analogValue);

    // This was just a test: from a terminal enter `particle call aardvark_crazy blink` and it will blink the built-in
    // LED on the Particle
    Particle.function("blink", blinkMultiple);
    // Particle.function("remotePos", remotePos);
    // Particle.function("remoteStep", remoteStep);
    // Particle.function("remoteTest", positionS);

    //Particle.function("", positionS);
    Serial.begin(9600);   // open serial over USB
    Serial.println("Hello Computer");
}

void loop() {
    m = millis();
    //positionS(200);
    if (m - last > (1000 * 60 * minutesDelay)) {
        last = m;
        blinkLed(5);  // To indicate that a reading is about to take place

        // Provide power to the moisture sensor
        digitalWrite(powerOut, HIGH);
        delay(50);  // just in case the sensor needs a little time to power up
        analogValue = analogRead(readMoisture);
        digitalWrite(powerOut, LOW);  // turn off the moisture sensor when not taking a reading, to prevent oxidation
        light = analogRead(readLight);

        // To see events in the terminal: `particle subscribe davidws:moisture`
        // The moisture event is currently picked up by IFTTT.com and it sends it to a Google spreadsheet
        //Particle.publish("davidws:moisture", String(analogValue), 240, PRIVATE);
        // Particle.publish("davidws:light", String(light), 240, PRIVATE);

        // To see Serial output in terminal:  `particle serial monitor /dev/tty.usbmodem1411`
        // First use `particle serial list` to see my online device address(es)
        Serial.println(analogValue);
    }
      if (buttonStatus == HIGH) {
        // blinkLed(2);
        move(1, HIGH);
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
      digitalWrite(step, HIGH);
      delay(1);
      digitalWrite(step, LOW);
      delay(1);
    }
    if (dir == HIGH) {
      position -= n;
    } else {
      position += n;
    }
    Serial.println(position);
    Particle.publish("davidws:testing", String(position), 60, PRIVATE);
}

void moveToPosition(int p) {
    int dir = HIGH;
    int delta = position - p;
    int pol = abs(delta) / delta;
    if (pol < 0) {
      dir= LOW;
    }
    move(abs(delta), dir);
    Serial.println("after moveToPosition:");
    Serial.println(position);
}
/*
int remotePos(String s) {
  Particle.publish("davidws:test", s, 60, PRIVATE);
  positionS(s.toInt());
}

// tell stepper to go to an absolute position
int positionS(int newPos) {
    int delta = stepperPos - newPos;
    int direction = delta / abs(delta);
    step(delta, direction);
    stepperPos = newPos;
}

// tell stepper to step n times in a direction, where direction is either a positive or negative number
void step(int n, int direction) {
    if (direction < 0) {
        digitalWrite(stepperDirection, HIGH);
    } else {
        digitalWrite(stepperDirection, LOW);
    }
    for (int i = 0; i < n; i++) {
        digitalWrite(stepper, HIGH);
        delay(2);
        digitalWrite(stepper, LOW);
    }
}


// For testing, this is a registered Particle function so I can control the stepper remotely.
// s is a String which will hold an int for number of steps.
// Direction of stepping isn't important, at least not now.
int remoteStep(String s) {
    int dir = 1;
    RGB.control(true);
    RGB.color(255, 0, 0);
    int c = s.toInt();
    if (c < 0) {
      dir = -1;
    }
    step(abs(c), dir);
    RGB.control(false);
}

int remoteTest(String s) {
    Particle.publish("davidws:test", s, 60, PRIVATE);
}
*/


// Need a calibration routine to position stepper at the starting position of the gauge

void buttonChanged() {
  buttonStatus = digitalRead(button);
}
