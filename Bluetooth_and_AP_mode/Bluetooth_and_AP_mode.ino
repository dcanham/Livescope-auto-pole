#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESP32Servo.h>


//Set up variables and things
const char* ssid = "ESP32-AP";
const char* password = "password";
bool isConnected = false; // Flag to track Bluetooth connection status
bool alreadyPrinted = false;
bool wifiOff = false;
BLEClient* pClient; // Declare global BLEClient object
WebServer server(80);
BLEScan* pBLEScan;
std::vector<BLEAdvertisedDevice> discoveredDevices;
BLEUUID HID_SERVICE_UUID;
BLEUUID HID_REPORT_CHAR_UUID;
#define CLIENT_CHARACTERISTIC_CONFIG_UUID "00002902-0000-1000-8000-00805f9b34fb"
#define SERVO_PIN 16
#define LEFT_LIMIT 0     // Adjust left limit (in degrees)
#define RIGHT_LIMIT 180   // Adjust right limit (in degrees)
Servo servo;
uint8_t lastButtonState = 0;   // Variable to store the last button state
int servoPosition = 90;        // Initial servo position (in degrees)
bool processHIDReport = true; // Flag to indicate whether to process HID reports


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        discoveredDevices.push_back(advertisedDevice);
    }
};

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
        page += "<li>Address: " + String(address.toString().c_str()) + ", Name: " + String(name.c_str()) +
                " <button onclick=\"window.location.href='/connect?address=" + String(address.toString().c_str()) + "'\">Connect</button></li>";
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
                pDescriptor->writeValue((uint8_t*)"\x01\x00", 2); // Enable notifications (0x0001)
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
    String addressParam = server.arg("address"); // Get the BLE device address from the URL parameter
    addressParam.trim(); // Remove leading and trailing whitespace characters

    Serial.println("Connecting to device at address: " + addressParam);

    if (addressParam.length() == 0) {
        Serial.println("No address provided.");
        server.send(400, "text/plain", "No address provided.");
        return;
    }

    BLEAddress address = BLEAddress(addressParam.c_str()); // Convert the address string to BLEAddress object

    // Check if already connected
    if (isConnected) {
        Serial.println("Already connected to a device.");
        server.send(200, "text/plain", "Already connected to a device.");
        return;
    }

    Serial.println("Attempting to connect to the device...");
    
    // Initialize BLE client
    pClient = BLEDevice::createClient(); // Removed local declaration to use the global one
    if (!pClient) {
        Serial.println("Failed to create BLE client.");
        server.send(500, "text/plain", "Failed to create BLE client.");
        return;
    }

    // Attempt to connect to the device
    if (pClient->connect(address)) {
        Serial.println("Connected to the device.");
        isConnected = true; // Set connection flag to true
    } else {
        Serial.println("Failed to connect to the device.");
        server.send(500, "text/plain", "Failed to connect to the device.");
    }

    Serial.println("Connect request handled. Lets anaylze ");

    // Get the HID service UUID and HID report characteristic UUID
    std::map<std::string, BLERemoteService*>* services = pClient->getServices();
    if (services) {
        for (auto& service : *services) {
            std::map<std::string, BLERemoteCharacteristic*>* characteristics = service.second->getCharacteristics();
            if (characteristics) {
                for (auto& characteristic : *characteristics) {
                    if (characteristic.second->canNotify()) {
                        if (characteristic.second->getUUID().equals(BLEUUID((uint16_t)0x2A4D))) { // HID report characteristic UUID is 0x2A4D
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
            processHIDReport = true; // Set flag to process next HID report
        } else {
            // Check HID report and control servo accordingly
            if (length >= 2) {
                if (pData[0] == 0xEA && pData[1] == 0x00) { // Volume down button
                    Serial.println("Volume down button pressed. Moving servo left.");
                    if (servoPosition > LEFT_LIMIT) {
                        servoPosition -= 10; // Decrease servo position by 10 degrees
                        servo.write(servoPosition);
                    } else {
                        // Flip around to the other side
                        servoPosition = RIGHT_LIMIT;
                        servo.write(servoPosition);
                    }
                } else if (pData[0] == 0xE9 && pData[1] == 0x00) { // Volume up button
                    Serial.println("Volume up button pressed. Moving servo right.");
                    if (servoPosition < RIGHT_LIMIT) {
                        servoPosition += 10; // Increase servo position by 10 degrees
                        servo.write(servoPosition);
                    } else {
                        // Flip around to the other side
                        servoPosition = LEFT_LIMIT;
                        servo.write(servoPosition);
                    }
                }
            }
            processHIDReport = false; // Set flag to skip processing subsequent HID reports until a terminating report is received
        }
        // Update last button state regardless of the button press
        lastButtonState = pData[0];
    }  else if (isNotify && !processHIDReport) {
        // Check if the HID report is a terminating report (0 0)
        if (length >= 2 && pData[0] == 0x00 && pData[1] == 0x00) {
            Serial.println("Terminating HID Report received. Ready to process next HID Report.");
            processHIDReport = true; // Set flag to process next HID report
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
    servo.attach(SERVO_PIN);
    Serial.println("Servo initialized.");
}

void loop() {
    if (!isConnected) {
        Serial.println("No device connected. Let's try to find one.");
        server.handleClient(); // This should only be called when a client is connected and sending requests
    }

    if (isConnected && !alreadyPrinted && !wifiOff) {
        alreadyPrinted = true;
        // Disable WiFi AP
        WiFi.softAPdisconnect(true);
        delay(200);

        // Optionally, you can also turn off WiFi completely
        WiFi.mode(WIFI_OFF);

        Serial.println("WiFi AP disabled.");
        wifiOff = true;
    }
  delay(200);
}



