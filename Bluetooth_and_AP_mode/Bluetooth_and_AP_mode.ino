#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <Arduino.h>

// Set up variables and things
const char* ssid = "ESP32-AP";
const char* password = "password";
bool isConnected = false;
bool alreadyPrinted = false;
bool wifiOff = false;
bool firstBoot = true;
bool addressEmpty = false;
const int servoPin = 22; // GPIO pin connected to the servo
const int pulseMin = 1250; // Minimum pulse width in microseconds
const int pulseMax = 1750; // Maximum pulse width in microseconds
const int pulseMid = 1500; // Middle pulse width to represent the mid-point
BLEClient* pClient;
WebServer server(80);
BLEScan* pBLEScan;
std::vector<BLEAdvertisedDevice> discoveredDevices;
BLEUUID HID_SERVICE_UUID;
BLEUUID HID_REPORT_CHAR_UUID;
#define CLIENT_CHARACTERISTIC_CONFIG_UUID "00002902-0000-1000-8000-00805f9b34fb"
#define SERVO_PIN 16
#define LEFT_LIMIT 0
#define RIGHT_LIMIT 180
#define EEPROM_ADDR_START 0
#define MAX_STRING_LENGTH 20  // Maximum length of the string to be stored in EEPROM
#define EEPROM_SIZE 200


Servo myservo;
Servo servo;
uint8_t lastButtonState = 0;
int servoPosition = 90;
bool processHIDReport = true;





class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    discoveredDevices.push_back(advertisedDevice);
  }
};

String getLastConnectedDeviceAddress() {
  String data = "";
  char byteRead;
  for (int i = 0; i < MAX_STRING_LENGTH; i++) {
    byteRead = char(EEPROM.read(EEPROM_ADDR_START + i));
    //Serial.print("Read byte: ");
    //Serial.println(byteRead);
    if (byteRead == '\0') {
      break;  // Stop reading if null terminator is encountered
    }
    data += byteRead;
  }

  // Print the read data to serial
  Serial.println("Data read from EEPROM:");
  Serial.println(data);
  return data;
}


void writeToEEPROM(String data) {
  int length = data.length();
  if (length > MAX_STRING_LENGTH - 1) {  // Ensure data fits within EEPROM size
    Serial.println("Data exceeds EEPROM capacity. Truncating...");
    length = MAX_STRING_LENGTH - 1;
    data.substring(0, length);  // Truncate data
  }

  Serial.println("Printing below to EEPROM: ");
  Serial.println(data);
  for (int i = 0; i < length; i++) {
    char byteToWrite = data[i];
    EEPROM.write(EEPROM_ADDR_START + i, byteToWrite);
    //Serial.print("Wrote byte: ");
    //Serial.println(byteToWrite);
  }
  EEPROM.write(EEPROM_ADDR_START + length, '\0');  // Null-terminate the string
  EEPROM.commit();                                 // Commit the changes to EEPROM
}


void handleRoot() {
  String page = "<html><body>";
  page += "<h1>Welcome to ESP32 Web Server</h1>";
  page += "<button onclick=\"window.location.href='/scan'\">Scan</button>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

void handleScan() {
  discoveredDevices.clear();
  BLEScanResults foundDevices = pBLEScan->start(5);
  server.sendHeader("Location", "/results", true);
  server.send(302, "text/plain", "");
}

void handleResults() {
  String page = "<html><body>";
  page += "<h1>Scan Results</h1>";
  page += "<ul>";
  for (size_t i = 0; i < discoveredDevices.size(); ++i) {
    BLEAdvertisedDevice device = discoveredDevices[i];
    BLEAddress address = device.getAddress();
    String name = device.getName().c_str();
    page += "<li>Address: " + String(address.toString().c_str()) + ", Name: " + String(name.c_str()) + " <button onclick=\"window.location.href='/connect?address=" + String(address.toString().c_str()) + "'\">Connect</button></li>";
  }
  page += "</ul>";

  // Add rescan button
  page += "<button onclick=\"window.location.href='/scan'\">Rescan</button>";

  page += "</body></html>";
  server.send(200, "text/html", page);
}

void registerForNotifications() {
  Serial.println("Started registerForNotifications part");
  BLERemoteService* pService = pClient->getService(HID_SERVICE_UUID);
  if (pService) {
    Serial.println("HID Service found.");
    Serial.print("HID Service UUID: ");
    Serial.println(pService->getUUID().toString().c_str());

    BLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(HID_REPORT_CHAR_UUID);
    if (pCharacteristic) {
      Serial.println("HID Report Characteristic found.");
      Serial.print("HID Report Characteristic UUID: ");
      Serial.println(pCharacteristic->getUUID().toString().c_str());

      // Use the hardcoded CLIENT_CHARACTERISTIC_CONFIG_UUID here
      const char* CCCD_UUID = "00002902-0000-1000-8000-00805f9b34fb";
      BLEUUID cccdUUID(CCCD_UUID);
      BLERemoteDescriptor* pDescriptor = pCharacteristic->getDescriptor(cccdUUID);
      if (pDescriptor) {
        Serial.println("Client Characteristic Configuration Descriptor found.");
        Serial.print("Client Characteristic Configuration Descriptor UUID: ");
        Serial.println(pDescriptor->getUUID().toString().c_str());

        Serial.println("Registering for notifications on HID report characteristic.");
        pDescriptor->writeValue((uint8_t*)"\x01\x00", 2);  // Enable notifications (0x0001)
        pCharacteristic->registerForNotify(notifyCallback);
        Serial.println("Registered for notifications.");
      } else {
        Serial.println("Failed to find Client Characteristic Configuration Descriptor.");
      }
    } else {
      Serial.println("Failed to find HID Report Characteristic.");
    }
  } else {
    Serial.println("Failed to find HID Service.");
  }
}

void handleConnect() {
  Serial.println("Handling connect request...");
  String addressParam = server.arg("address");  // Get the BLE device address from the URL parameter
  addressParam.trim();                          // Remove leading and trailing whitespace characters

  Serial.println("Connecting to device at address: " + addressParam);

  if (addressParam.length() == 0) {
    Serial.println("No address provided.");
    server.send(400, "text/plain", "No address provided.");
    return;
  }

  BLEAddress address = BLEAddress(addressParam.c_str());  // Convert the address string to BLEAddress object

  // Check if already connected
  if (isConnected) {
    Serial.println("Already connected to a device.");
    server.send(200, "text/plain", "Already connected to a device.");
    return;
  }

  Serial.println("Attempting to connect to the device...");

  // Initialize BLE client
  pClient = BLEDevice::createClient();  // Removed local declaration to use the global one
  if (!pClient) {
    Serial.println("Failed to create BLE client.");
    server.send(500, "text/plain", "Failed to create BLE client.");
    return;
  }

  // Attempt to connect to the device
  if (pClient->connect(address)) {
    Serial.println("Connected to the device.");
    isConnected = true;  // Set connection flag to true
    // Save the connected device address to EEPROM
    writeToEEPROM(addressParam);
    Serial.println("Written to EEProm");


    Serial.println("Connect request handled. Lets anaylze ");

    // Get the HID service UUID and HID report characteristic UUID
    std::map<std::string, BLERemoteService*>* services = pClient->getServices();
    if (services) {
      for (auto& service : *services) {
        std::map<std::string, BLERemoteCharacteristic*>* characteristics = service.second->getCharacteristics();
        if (characteristics) {
          for (auto& characteristic : *characteristics) {
            if (characteristic.second->canNotify()) {
              if (characteristic.second->getUUID().equals(BLEUUID((uint16_t)0x2A4D))) {  // HID report characteristic UUID is 0x2A4D
                Serial.println("Found HID report characteristic.");
                HID_SERVICE_UUID = BLEUUID(service.first);
                HID_REPORT_CHAR_UUID = BLEUUID(characteristic.first);
                registerForNotifications();
                return;
              }
            }
          }
        }
      }
    }
  } else {
    Serial.println("Failed to connect to the device.");
    server.send(500, "text/plain", "Failed to connect to the device.");
  }

  Serial.println("Failed to find HID service or report characteristic.");
}

void handleConnectEEProm(String addressParam) {
  Serial.println("Handling connect request...");
  //String addressParam = server.arg("address"); // Get the BLE device address from the URL parameter
  addressParam.trim();  // Remove leading and trailing whitespace characters

  Serial.println("Connecting to device at address: " + addressParam);

  if (addressParam.length() == 0) {
    Serial.println("No address provided.");
    server.send(400, "text/plain", "No address provided.");
    return;
  }

  BLEAddress address = BLEAddress(addressParam.c_str());  // Convert the address string to BLEAddress object

  // Check if already connected
  // if (isConnected) {
  //     Serial.println("Already connected to a device.");
  //     server.send(200, "text/plain", "Already connected to a device.");
  //     return;
  // }

  Serial.println("Attempting to connect to the device...");

  // Initialize BLE client
  pClient = BLEDevice::createClient();  // Removed local declaration to use the global one
  if (!pClient) {
    Serial.println("Failed to create BLE client.");
    //server.send(500, "text/plain", "Failed to create BLE client.");
    return;
  }

  // Attempt to connect to the device
  if (pClient->connect(address)) {
    Serial.println("Connected to the device.");
    isConnected = true;  // Set connection flag to true
    // Save the connected device address to EEPROM
    writeToEEPROM(addressParam);
    Serial.println("Written to EEProm");


    Serial.println("Connect request handled. Lets anaylze ");

    // Get the HID service UUID and HID report characteristic UUID
    std::map<std::string, BLERemoteService*>* services = pClient->getServices();
    if (services) {
      for (auto& service : *services) {
        std::map<std::string, BLERemoteCharacteristic*>* characteristics = service.second->getCharacteristics();
        if (characteristics) {
          for (auto& characteristic : *characteristics) {
            if (characteristic.second->canNotify()) {
              if (characteristic.second->getUUID().equals(BLEUUID((uint16_t)0x2A4D))) {  // HID report characteristic UUID is 0x2A4D
                Serial.println("Found HID report characteristic.");
                HID_SERVICE_UUID = BLEUUID(service.first);
                HID_REPORT_CHAR_UUID = BLEUUID(characteristic.first);
                registerForNotifications();
                return;
              }
            }
          }
        }
      }
    }
  } else {
    Serial.println("Failed to connect to the device.");
    //server.send(500, "text/plain", "Failed to connect to the device.");
  }

  Serial.println("Failed to find HID service or report characteristic.");
}

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (isNotify && processHIDReport) {
    Serial.print("Notification received from characteristic: ");
    Serial.println(pCharacteristic->getUUID().toString().c_str());

    Serial.print("Received HID Report: ");
    for (int i = 0; i < length; i++) {
      Serial.print(pData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    // Check if the HID report is a terminating report (0 0)
    if (length >= 2 && pData[0] == 0x00 && pData[1] == 0x00) {
      Serial.println("Terminating HID Report received. Ready to process next HID Report.");
      processHIDReport = true;  // Set flag to process next HID report
    } else {
      // Check HID report and control servo accordingly
      if (length >= 2) {
        if (pData[0] == 0xEA && pData[1] == 0x00) {  // Volume down button
          Serial.println("Volume down button pressed. Moving servo left.");
          myservo.writeMicroseconds(pulseMin);
          delay(200);
          myservo.writeMicroseconds(pulseMid);
        } else if (pData[0] == 0xE9 && pData[1] == 0x00) {  // Volume up button
          Serial.println("Volume up button pressed. Moving servo right.");
          myservo.writeMicroseconds(pulseMax);
          delay(200);
          myservo.writeMicroseconds(pulseMid);
        }
      }
      processHIDReport = false;  // Set flag to skip processing subsequent HID reports until a terminating report is received
    }
    // Update last button state regardless of the button press
    lastButtonState = pData[0];
  } else if (isNotify && !processHIDReport) {
    // Check if the HID report is a terminating report (0 0)
    if (length >= 2 && pData[0] == 0x00 && pData[1] == 0x00) {
      Serial.println("Terminating HID Report received. Ready to process next HID Report.");
      processHIDReport = true;  // Set flag to process next HID report
    }
  }
}


void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started. Connect to: " + String(ssid));
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/results", HTTP_GET, handleResults);
  server.on("/connect", HTTP_GET, handleConnect);
  server.begin();
  // Initialize servo

  myservo.setPeriodHertz(50);    // Standard 50Hz servo
  myservo.attach(servoPin, pulseMin, pulseMax);  // attaches the servo on pin to the servo object
                                                 // and defines min and max pulse widths
  //servo.attach(SERVO_PIN);
  EEPROM.begin(EEPROM_SIZE);

  //String testaddress = "00:11:22:33:44:55"; // Example Bluetooth address
  //writeToEEPROM(testaddress);


  Serial.println("Servo initialized.");
  String eepromData = getLastConnectedDeviceAddress();
  Serial.print("Last connected device address: ");
  Serial.println(eepromData);
}

void loop() {

  if (firstBoot) {
    // Attempt to connect to the stored Bluetooth address on first boot

    firstBoot = false;
    String eepromData = getLastConnectedDeviceAddress();
    Serial.print("Last connected device address: ");
    Serial.println(eepromData);

    if (eepromData.length() == 0) {
      Serial.println("No address provided.");
      addressEmpty = true;

    } else {
      Serial.print("Address Provided lets connect");
      handleConnectEEProm(eepromData);
    }

    if (!isConnected && addressEmpty) {
      // Proceed with normal boot process if connection fails
      Serial.print("Address Empty, not connecting and trying normal boot on next void loop run");
    } else if (isConnected){
      //we conencted, lets clean up wifi and output
      alreadyPrinted = true;
      // Turn off Wifi to save power
      WiFi.mode(WIFI_OFF);

      Serial.println("WiFi AP disabled because we connected to a previously known device.");
      wifiOff = true;
    } else {
      Serial.println("We didn't connect to an older device so we'll move on back to void loop");

    }

  } else {
    // Proceed with normal boot process
    if (!isConnected) {
      Serial.println("No device connected. Let's try to find one.");
      server.handleClient();  // This should only be called when a client is connected and sending requests
    }

    if (isConnected && !alreadyPrinted && !wifiOff) {
      alreadyPrinted = true;
      // Disable WiFi AP
      WiFi.softAPdisconnect(true);
      delay(200);

      // Optionally, you can also turn off WiFi completely
      WiFi.mode(WIFI_OFF);

      Serial.println("WiFi AP disabled. ");
      wifiOff = true;
    }
  }

  delay(200);
}
