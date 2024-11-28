#include <AccelStepper.h>

// Define connections for the first stepper motor
#define dirPin1 4
#define stepPin1 5

// Define connections for the second stepper motor
#define dirPin2 9
#define stepPin2 8

// Define input pins
const int hGatePin = 2;          // H Gate input pin
const int cnotGatePin = 3;       // CNOT Gate input pin
const int measureButtonPin = 6;  // Measurement button input pin
const int buttonLED = 10;        //measure button light
const int baseSpeed = 50;
const int superSpeed = 500;

// Define button pins, LEDs and script numbers
const int buttonPins[] = { A0, A1, A2 };         // Button 1, 2, 3 pins
bool buttonPressed[] = { false, false, false };  // Tracks if each button was pressed
const int ledPins[] = { 12, 11, A3 };            // LED 1, 2, 3 pins
const int scripts[] = { 1, 2, 3 };               // Corresponding script numbers

// Create AccelStepper objects for each motor
AccelStepper motor1(AccelStepper::DRIVER, stepPin1, dirPin1);
AccelStepper motor2(AccelStepper::DRIVER, stepPin2, dirPin2);

// Gate detection flags
volatile bool HGateDetected = false;
volatile bool CnotGateDetected = false;
bool measureButtonPressed = false;
bool bellState = false;
bool waitingForSelection = true;
bool audioPlaying = false;

void setup() {
  Serial.begin(115200);

  // Set button pins as inputs
  for (int i = 0; i < 3; i++) {
    pinMode(buttonPins[i], INPUT);
  }

  // Set LED pins as outputs and ensure LEDs are off initially
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Set sensor pins as inputs
  pinMode(hGatePin, INPUT);                 // H Gate sensor
  pinMode(cnotGatePin, INPUT);              // CNOT Gate sensor
  pinMode(measureButtonPin, INPUT_PULLUP);  // Measure button with internal pull-up
  pinMode(buttonLED, OUTPUT);               //Measre button light

  // Attach interrupts to the sensor pins
  attachInterrupt(digitalPinToInterrupt(hGatePin), HGateISR, CHANGE);        // H Gate Sensor
  attachInterrupt(digitalPinToInterrupt(cnotGatePin), CnotGateISR, CHANGE);  // CNOT Gate Sensor

  // Set the maximum speed for each motor (steps per second)
  motor1.setMaxSpeed(1000);
  motor2.setMaxSpeed(1000);

  // Set the acceleration for each motor (steps per second^2)
  motor1.setAcceleration(1000);
  motor2.setAcceleration(1000);

  // Set initial speeds for both motors
  motor1.setSpeed(50);
  motor2.setSpeed(50);
}

void loop() {
 // Serial.println(scripts[0]);
  gateInteractions();  //handles all interactions with gates before measurement
  // Serial.println("2");
  // delay(10);
// Check for confirmation from ESP32
  if (Serial.available()) {
    String confirmation = Serial.readStringUntil('\n');
    if (confirmation == "ACK") {
      Serial.println("ESP32 acknowledged the message!");
    }
  }
  buttonControl();  //handles button detection that triggers audio script

  // Check for measure button press
  if (!measureButtonPressed && digitalRead(measureButtonPin) == HIGH) {
    // Debounce delay
    delay(50);
    if (digitalRead(measureButtonPin) == HIGH) {  // Confirm button is still pressed
      Serial.println("measure button pressed");
      measureButtonPressed = true;  // Mark button as pressed
      handleMeasurement();
    }
  }
}

//handles control of gate interactions and motor behavior
void gateInteractions() {
  // Only adjust motor speeds if the measure button hasn't been pressed yet
  if (!measureButtonPressed) {
    // Serial.println("measure button not pressed yet");
    if (HGateDetected && !CnotGateDetected) {
      Serial.println("H Gate Detected");
      motor1.setSpeed(superSpeed);
      digitalWrite(buttonLED, HIGH);
      // Run motors at their set speeds
      motor1.runSpeed();
      motor2.runSpeed();
    } else if (HGateDetected && CnotGateDetected) {
      bellState = true;
      Serial.println("H and CNot Gate Detected");
      motor1.setSpeed(superSpeed);
      motor2.setSpeed(superSpeed);
      // Run motors at their set speeds
      motor1.runSpeed();
      motor2.runSpeed();
      digitalWrite(buttonLED, HIGH);
    } else if (CnotGateDetected && !HGateDetected) {
      Serial.println("CNot Gate Detected");
      motor1.setSpeed(baseSpeed);
      motor2.setSpeed(baseSpeed);
      // Run motors at their set speeds
      motor1.runSpeed();
      motor2.runSpeed();
      digitalWrite(buttonLED, HIGH);
    } else {
      resetGates();
    }
  }
  if (!bellState) {
    motor2.setSpeed(baseSpeed);
    motor2.runSpeed();
  }
  if (!HGateDetected && !CnotGateDetected) {
    resetGates();
  }

  if (!HGateDetected && !CnotGateDetected && !audioPlaying) {
    resetExperience();
  }
}

// Handle measurement logic
void handleMeasurement() {
  digitalWrite(buttonLED, LOW);
  Serial.println("handle measurement");
  if (HGateDetected && !CnotGateDetected) {
    // 50% chance for motor1 to move to 0 or 100
    Serial.println("Qbit 1 stop");
    if (random(0, 2) == 0) {
      motor1.moveTo(0);
    } else {
      motor1.moveTo(100);
    }
  } else if (HGateDetected && CnotGateDetected) {
    // 50% chance for both motors to move to 0 or 100
    if (random(0, 2) == 0) {
      motor1.moveTo(0);
      motor2.moveTo(0);
    } else {
      motor1.moveTo(100);
      motor2.moveTo(100);
    }
  }
}

//calls audioScriptControl in loop
void buttonControl() {
  // Check each button for interaction
  for (int i = 0; i < 3; i++) {
    audioScriptControl(i);  // Call for each button index
  }
}

//handles audio selection based on button press
void audioScriptControl(int buttonIndex) {
  if (waitingForSelection && digitalRead(buttonPins[buttonIndex]) == HIGH) {
    // Debounce delay
    delay(50);
    if (digitalRead(buttonPins[buttonIndex]) == HIGH) {  // Confirm button is still pressed
      buttonPressed[buttonIndex] = true;  // Mark button as pressed
      digitalWrite(ledPins[buttonIndex], HIGH);
      Serial.println(scripts[buttonIndex]);  // Print the selected script
      Serial.println(scripts[buttonIndex]);
      Serial.println(scripts[buttonIndex]);
      Serial.println(scripts[buttonIndex]);
      audioPlaying = true;
    }
  }

  // Exit waiting mode if any button is pressed
  if (buttonPressed[0] || buttonPressed[1] || buttonPressed[2]) {
    waitingForSelection = false;
  }
}

void resetGates(){
  bellState = false;
  measureButtonPressed = false;
  digitalWrite(buttonLED, LOW);
}
void resetExperience() {
  //reset detection flags and turn off LEDs
  waitingForSelection = true;
  bellState = false;
  measureButtonPressed = false;
  digitalWrite(buttonLED, LOW);
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], LOW);
    // buttonPressed[i] = false;
  }

  // Run motors at their base speeds
  motor1.setSpeed(baseSpeed);
  motor2.setSpeed(baseSpeed);
  motor1.runSpeed();
  motor2.runSpeed();
}

// Interrupt service routine for the H Gate sensor
void HGateISR() {
  int signalValueHGate = digitalRead(hGatePin);
  HGateDetected = (signalValueHGate == LOW);
  // Serial.print("H Gate Detected: ");
  //Serial.println(HGateDetected ? "YES" : "NO");
}

// Interrupt service routine for the CNOT Gate sensor
void CnotGateISR() {
  int signalValueCnotGate = digitalRead(cnotGatePin);
  CnotGateDetected = (signalValueCnotGate == LOW);
  //Serial.print("CNOT Gate Detected: ");
  // Serial.println(CnotGateDetected ? "YES" : "NO");
}
