// Stepper testing

int step = A2;
int direction = A3;
int blueLed = D7;
int position = 0;
bool go = true;

void setup() {
  pinMode(step, OUTPUT);
  pinMode(direction, OUTPUT);
  pinMode(blueLed, OUTPUT);
  digitalWrite(direction, HIGH);
  Particle.variable("position", position);
  Serial.begin(9600);
  Serial.println("Hello");
}

void loop() {
  digitalWrite(blueLed, HIGH);
  if (go) {
    move(600, HIGH);
    delay(2000);
    moveToPosition(1200);
    go = false;
    Serial.println("from loop");
  }
}

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
    position = p;
    Serial.println("after moveToPosition:");
    Serial.println(position);
}

void blink(int n) {
    for (int c = 0; c < n; c++) {
      digitalWrite(blueLed, HIGH);
      delay(100);
      digitalWrite(blueLed, LOW);
      delay(70);
    }
}

void publish(String s) {
  Particle.publish("davidws:testing", s, 60, PRIVATE);
}
