#include <ESP32Servo.h>

Servo myservo;  // create servo object to control a servo

// Update servo pin to 16
const int servoPin = 16; // GPIO pin connected to the servo
const int pulseMin = 1000; // Minimum pulse width in microseconds
const int pulseMax = 2000; // Maximum pulse width in microseconds
const int pulseMid = 1500; // Middle pulse width to represent the mid-point

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
  myservo.setPeriodHertz(50);    // Standard 50Hz servo
  myservo.attach(servoPin, pulseMin, pulseMax);  // attaches the servo on pin to the servo object
                                                 // and defines min and max pulse widths
  Serial.println("Servo debug started...");
}

void loop() {
  // Move servo to the left position
  Serial.println("Moving to min...");
  myservo.writeMicroseconds(pulseMin);
  Serial.print("Pulse width: ");
  Serial.println(pulseMin);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds

  // Move servo to the left position
  Serial.println("Moving to min...");
  myservo.writeMicroseconds(pulseMin);
  Serial.print("Pulse width: ");
  Serial.println(pulseMin);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds

  // Move servo to the left position
  Serial.println("Moving to min...");
  myservo.writeMicroseconds(pulseMin);
  Serial.print("Pulse width: ");
  Serial.println(pulseMin);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds


  // Move servo to the right position
  Serial.println("Moving to max...");
  myservo.writeMicroseconds(pulseMax);
  Serial.print("Pulse width: ");
  Serial.println(pulseMax);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds

  // Move servo to the right position
  Serial.println("Moving to max...");
  myservo.writeMicroseconds(pulseMax);
  Serial.print("Pulse width: ");
  Serial.println(pulseMax);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds

  // Move servo to the right position
  Serial.println("Moving to max...");
  myservo.writeMicroseconds(pulseMax);
  Serial.print("Pulse width: ");
  Serial.println(pulseMax);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds

  // Move servo to the right position
  Serial.println("Moving to max...");
  myservo.writeMicroseconds(pulseMax);
  Serial.print("Pulse width: ");
  Serial.println(pulseMax);
  delay(200);
  myservo.writeMicroseconds(pulseMid);
  delay(2000); // Wait for 2 seconds
}
