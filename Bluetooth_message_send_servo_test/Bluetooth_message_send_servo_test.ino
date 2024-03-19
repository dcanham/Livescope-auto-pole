#include <ESP32Servo.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 16; // pin 27 GPIO16
String message = "";
char incomingChar;
int servopos = 90;
int servoposl = 0;
int servoposr = 180;
const int SERVO_STEP = 10;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 500, 2500); // attaches the servo on pin 18 to the servo object
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  
}

void loop() {
  /*
  message = "";
  if (SerialBT.available()) {
    message = SerialBT.readString();
    Serial.printf("the message is %s\n", message);
  }
  //delay(2000);
  delay(20);
  if (message == "hi"){
    myservo.write(0);
  }
  if (message == "hello"){
    myservo.write(180);
  }
  delay(20); */
  /*if (SerialBT.available()){
    for (char incomingChar = SerialBT.read(); incomingChar != '\n'; message += String(incomingChar)){
      if (message =="r"){
        //myservo.write(servoposl);
        Serial.printf("R letter if: %s\n", message);
        message = "";
        
        delay(20);
        }
      if (message =="l"){
        //myservo.write(servoposr);
        Serial.printf("L letter if: %s\n", message);
        message = "";
        delay(20);
      }
    }
  }*/ 
  // Check received message and control output accordingly
  /*
  if (SerialBT.available()) {  // Check if data is available to read from Bluetooth
    String command = SerialBT.readStringUntil('\n'); // Read the incoming string
    Serial.print("Received command: ");
    Serial.println(command);
    for (int i = 0; i < command.length(); i++) {
      Serial.print(command.charAt(i), DEC);
      Serial.print(" ");
    }
  Serial.println(); // Print a newline
  command.trim();
    
    if (command =="left") {  // If "left" is received, move servo to 0 degrees
      myservo.write(0);
      Serial.println("Servo moved to 0 degrees");
    } 
    else if (command == "right") { // If "right" is received, move servo to 180 degrees
      myservo.write(180);
      Serial.println("Servo moved to 180 degrees");
    } 
    else {
      Serial.println("Invalid command");
    }
  }
  */
  if (SerialBT.available()) {  // Check if data is available to read from Bluetooth
    String command = SerialBT.readStringUntil('\n'); // Read the incoming string
    command.trim(); // Remove leading and trailing whitespace, including carriage return ('\r')
    
    if (command.equals("left")) {  // If "volume_up" is received, move servo a few degrees left
      int newPosition = myservo.read() - SERVO_STEP;
      if (newPosition < 0) {
        newPosition += 180; // Wrap around to 180 degrees
      }
      myservo.write(constrain(newPosition, 0, 180));
      Serial.println("Servo moved a few degrees left");
    } 
    else if (command.equals("right")) { // If "volume_down" is received, move servo a few degrees right
      int newPosition = myservo.read() + SERVO_STEP;
      if (newPosition > 180) {
        newPosition -= 180; // Wrap around to 0 degrees
      }
      myservo.write(constrain(newPosition, 0, 180));
      Serial.println("Servo moved a few degrees right");
    } 
    else {
      Serial.println("Invalid command");
    }
  }

  
  
  
}