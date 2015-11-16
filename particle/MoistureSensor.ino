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
int stepperPos = 0;  // represents the current stepper position
int stepper = A2;  // CHECK THAT THIS PIN IS SUITABLE
int stepperDirection = A3; // CHECK THAT THIS PIN IS SUITABLE
int button = D1;
int last = 0;
int m;
bool buttonStatus = false;
void calibrate(void);

void setup()
{
    attachInterrupt(button, buttonChange, CHANGE);
    pinMode(button, INPUT);
    pinMode(powerOut, OUTPUT);
    pinMode(ledBoard, OUTPUT);
    pinMode(readLight, INPUT);
    pinMode(readMoisture, INPUT);
    pinMode(stepper, OUTPUT);
    pinMode(stepperDirection, OUTPUT);
    digitalWrite(stepper, LOW);

    // Just a test:
    // Publish value to a rest endpoint, also get this via: particle variable get aardvark_crazy analogValue
    // or, HTTP GET /v1/devices/{DEVICE_ID}/{VARIABLE}
    Particle.variable("analogValue", analogValue);

    // This was just a test: from a terminal enter `particle call aardvark_crazy blink` and it will blink the built-in
    // LED on the Particle
    // Particle.function("blink", blinkMultiple);
    Particle.function("resetStepperPos", resetStepperPos);
    Particle.function("remoteStep", remoteStep);
    // Particle.function("remoteTest", remoteTest);

    Particle.function("positionStepper", positionStepper);
    Serial.begin(9600);   // open serial over USB
    Serial.println("Hello Computer");
}

void loop() {
    m = millis();
    if (m - last > (1000 * 60 * minutesDelay)) {
        last = m;
        blinkLed(5);  // To indicate that a reading is about to take place

        // Provide power to the moisture sensor
        digitalWrite(powerOut, HIGH);
        delay(100);  // just in case the sensor needs a little time to power up
        analogValue = analogRead(readMoisture);
        digitalWrite(powerOut, LOW);  // turn off the moisture sensor when not taking a reading, to prevent oxidation
        light = analogRead(readLight);

        // To see events in the terminal: `particle subscribe davidws:moisture`
        // The moisture event is currently picked up by IFTTT.com and it sends it to a Google spreadsheet
        Particle.publish("davidws:moisture", String(analogValue), 240, PRIVATE);
        Particle.publish("davidws:light", String(light), 240, PRIVATE);

        // To see Serial output in terminal:  `particle serial monitor /dev/tty.usbmodem1411`
        // First use `particle serial list` to see my online device address(es)
        Serial.println(analogValue);
    }
    if (buttonStatus) {
        // Particle.publish("davidws:interrupt", "interrupt", 60, PRIVATE);
        if (button == HIGH) {
          blinkLed(2);
        } else {
          blinkLed(1);
        }
        buttonStatus = false;
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

// tell stepper to go to an absolute position
int positionStepper(String newPos) {
    int delta = stepperPos - newPos.toInt();
    int direction = delta / abs(delta);
    step(delta, direction);
    return 1;
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
    RGB.control(true);
    RGB.color(255, 0, 0);
    int c = s.toInt();
    step(c, 1);
    RGB.control(false);
}

int resetStepperPos(String dummy) {
    stepperPos = 0;
    return 1;
}

int remoteTest(String s) {
    Particle.publish("davidws:test", s, 60, PRIVATE);
}

// Need a calibration routine to position stepper at the starting position of the gauge

void buttonChange() {
    buttonStatus = true;
}
