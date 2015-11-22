/* Read soil moisture sensor and photoresistor and write values to serial comms and particle, etc.
 openweathermap.org bethesda city id = 4348599
 http://api.openweathermap.org/data/2.5/forecast?id=<id>
 To see events in the terminal: `particle subscribe davidws:moisture`
 The moisture event is currently picked up by IFTTT.com and it sends it to a Google spreadsheet
 Particle.publish("davidws:light", String(light), 240, PRIVATE);
 To see Serial output in terminal:  `particle serial monitor /dev/tty.usbmodem1411`
 First use `particle serial list` to see my online device address(es)
*/
/*

pi@raspberrypi:~/weather_parse$ !ru
ruby get_weather.rb
{"temp"=>278.5,
 "temp_min"=>278.5,
 "temp_max"=>279.241,
 "pressure"=>1017.67,
 "sea_level"=>1031.8,
 "grnd_level"=>1017.67,
 "humidity"=>52,
 "temp_kf"=>-0.74}
41
command: curl https://api.particle.io/v1/devices/aardvark_crazy/remoteTemp -d arg=41 -d access_token=a8a66512b6b46995c6997361d82c0115aff8c49e
{
  "ok": false,
  "error": "Timed out."

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
int lastPos;
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
    // Particle.function("blink", blinkMultiple);
    bool success = Particle.function("remoteMove", remoteMove);
    success = Particle.function("remoteTemp", remoteTemp);
    success = Particle.function("remoteNone", remoteNone);
    int foo = EEPROM.get(10, lastPos);
    Particle.publish("davidws:debug", "foo: " + String(foo) + " lastPos: " + String(lastPos));
    if (foo > 0) {
      position = foo;
      moveToPosition(0);
    }
    // To monitor, e.g.: screen /dev/cu.usbmodem1411 9600
    Serial.begin(9600);   // open serial over USB
    Serial.println("Getting started...");

    // jiggle();  // just a visible/audible confirmation that the program has startedA
}

void loop() {
    m = millis();
    // take a reading
    if (m - last > (1000 * 60 * minutesDelay)) {
        last = m;
        blinkLed(5);  // To indicate that a reading is about to take place
        getMoisture();
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
int remoteNone(String s) {
  return 1;
}

void jiggle() {
  move(100, UP);
  move(200, DOWN);
  move(100, UP);
}

void getMoisture() {
    // Provide power to the moisture sensor
    digitalWrite(powerOut, HIGH);
    delay(50);  // just in case the sensor needs a little time to power up
    analogValue = analogRead(readMoisture);
    digitalWrite(powerOut, LOW);  // turn off the moisture sensor when not taking a reading, to prevent oxidation
    light = analogRead(readLight); // read the CdS light cell, just for fun
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
      EEPROM.put(10, position);
    } else {
      position -= n;
      EEPROM.put(10, position);
    }
    // Serial.println(position);
    // Particle.publish("davidws:testing", String(position), 60, PRIVATE);
}
// Move stepper to a particular position, positive or negative integer
void moveToPosition(int p) {
    int dir = DOWN;
    int delta = p - position;
    if (delta != 0) {
      int pol = abs(delta) / delta;
      if (pol > 0) {
        dir= UP;
      }
      move(abs(delta), dir);
    }
}

// Remotely tell indicator to move to a particular temperature
// This would be called from a HTTP POST
int remoteTemp(String temperature) {
  int x = ((temperature.toFloat() - BOTTOM_SCALE_NUMBER) / \
  (TOP_SCALE_NUMBER - BOTTOM_SCALE_NUMBER)) * \
  (TOP_SCALE_STEPPER_POSITION - BOTTOM_SCALE_STEPPER_POSITION);
  moveToPosition((int) x);
  return 1;
}

int remoteMove(String i) {
  moveToPosition(i.toInt());
  return 1;
}

void buttonChanged() {
  buttonStatus = digitalRead(button);
  if (buttonStatus == LOW) {
    buttonTurnedOff = true;
  }
}
