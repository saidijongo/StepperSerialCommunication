// Pin Definitions
const int CW_PLUS_PIN = 5;
const int CCW_PLUS_PIN = 6;
const int RIGHT_SENSOR_PIN = 11;
const int LEFT_SENSOR_PIN = 10;

// Motor Step Delay (Speed)
unsigned long stepDelay = 10000;

// Flags to track motor movement and interruption
bool rotatingCW = false;
bool rotatingCCW = false;
bool rotatingReset = false;
bool stopped = true;

// Function to move the motor in a specified direction and angle
void moveMotor(int angleDegrees, bool clockwise) {
  int steps = angleDegrees / 0.72;  // Calculate steps based on angle

  if (clockwise) {
    digitalWrite(CW_PLUS_PIN, HIGH);
    digitalWrite(CCW_PLUS_PIN, LOW);
  } else {
    digitalWrite(CW_PLUS_PIN, LOW);
    digitalWrite(CCW_PLUS_PIN, HIGH);
  }

  // Move the motor for the specified steps
  for (int i = 0; i < steps; i++) {
    if (rotatingCW && digitalRead(LEFT_SENSOR_PIN) == LOW) {
      // Interrupted by left sensor during CW rotation
      rotatingCW = false;
      rotatingCCW = false;
      moveMotor(8, false);  // Rotate 8 degrees CCW
      Serial.println("Interrupted: CW - " + String(i * 0.72) + " degrees");
      return;
    }

    if (rotatingCCW && digitalRead(RIGHT_SENSOR_PIN) == LOW) {
      // Interrupted by right sensor during CCW rotation
      rotatingCW = false;
      rotatingCCW = false;
      moveMotor(8, true);  // Rotate 8 degrees CW
      Serial.println("Interrupted: CCW - " + String(i * 0.72) + " degrees");
      return;
    }

    if (stopped) {
      // Motor stopped by 'S' command
      digitalWrite(CW_PLUS_PIN, LOW);
      digitalWrite(CCW_PLUS_PIN, LOW);
      Serial.println("Motor Stopped");
      return;
    }

    // Move one step multiple times to achieve desired angle
    for (int j = 0; j < (angleDegrees > 90 ? 10 : 1); j++) {
      digitalWrite(CW_PLUS_PIN, HIGH);
      digitalWrite(CCW_PLUS_PIN, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(CW_PLUS_PIN, LOW);
      digitalWrite(CCW_PLUS_PIN, LOW);
      delayMicroseconds(stepDelay);
    }
  }

  Serial.println("Moved: " + String(angleDegrees) + " degrees " + (clockwise ? "CW" : "CCW"));
}

void setup() {
  pinMode(CW_PLUS_PIN, OUTPUT);
  pinMode(CCW_PLUS_PIN, OUTPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'C':
        rotatingCW = true;
        moveMotor(90, true);  // Rotate 90 degrees CW
        rotatingCW = false;
        break;
      case 'D':
        rotatingCCW = true;
        moveMotor(90, false);  // Rotate 90 degrees CCW
        rotatingCCW = false;
        break;
      case 'R':
        rotatingReset = true;
        while (digitalRead(RIGHT_SENSOR_PIN) == HIGH) {
          digitalWrite(CW_PLUS_PIN, HIGH);
          digitalWrite(CCW_PLUS_PIN, LOW);
        }
        moveMotor(188, true);  // Rotate 188 degrees CW
        if (digitalRead(LEFT_SENSOR_PIN) == LOW) {
          moveMotor(8, false);  // Rotate 8 degrees CCW
          Serial.println("Home Base");
        } else {
          moveMotor(8, true);  // Rotate 8 degrees CW
          Serial.println("Home Base");
        }
        rotatingReset = false;
        break;
      case 'S':
        stopped = true;
        break;
      case 'I':
        if (rotatingCW || rotatingCCW) {
          Serial.println("Rotating in " + String(rotatingCW ? "CW" : "CCW") + " direction");
        } else if (rotatingReset) {
          Serial.println("Rotating to Home Base");
        } else if (stopped) {
          if (digitalRead(LEFT_SENSOR_PIN) == LOW) {
            Serial.println("Stopped: Home Base");
          } else {
            Serial.println("Stopped: Not Home Base");
          }
        }
        break;
      case 'V':
        // Handle speed change (not implemented in this example)
        break;
      default:
        break;
    }
  }
}
