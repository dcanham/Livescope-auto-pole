#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLECharacteristic.h>
#include <ESP32Servo.h>

#define HID_SERVICE_UUID "00001812-0000-1000-8000-00805f9b34fb"
#define HID_REPORT_CHAR_UUID "00002A4D-0000-1000-8000-00805f9b34fb"
#define CLIENT_CHARACTERISTIC_CONFIG_UUID "00002902-0000-1000-8000-00805f9b34fb"
#define SERVO_PIN 16

BLEClient* pClient;
BLERemoteCharacteristic* pReportCharacteristic;
Servo servo;

#define LEFT_LIMIT 0     // Adjust left limit (in degrees)
#define RIGHT_LIMIT 180   // Adjust right limit (in degrees)

uint8_t lastButtonState = 0;   // Variable to store the last button state
int servoPosition = 90;        // Initial servo position (in degrees)
bool processHIDReport = true; // Flag to indicate whether to process HID reports

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (isNotify && processHIDReport) {
        Serial.print("Notification received from characteristic: ");
        Serial.println(String(pCharacteristic->getUUID().toString().c_str()));
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
    } else if (isNotify && !processHIDReport) {
        // Check if the HID report is a terminating report (0 0)
        if (length >= 2 && pData[0] == 0x00 && pData[1] == 0x00) {
            Serial.println("Terminating HID Report received. Ready to process next HID Report.");
            processHIDReport = true; // Set flag to process next HID report
        }
    }
}







void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize BLE
    BLEDevice::init("");

    // Connect to the BLE HID device
    pClient = BLEDevice::createClient();
    BLEAddress deviceAddress("c7:a3:e8:b2:b1:a9");
    if (!pClient->connect(deviceAddress)) {
        Serial.println("Failed to connect to the device.");
        return;
    }
    Serial.println("Connected to the device.");

    // Get the HID service
    BLERemoteService* pRemoteService = pClient->getService(BLEUUID(HID_SERVICE_UUID));
    if (pRemoteService == nullptr) {
        Serial.println("Failed to find HID service.");
        return;
    }

    // Get the Report characteristic
    pReportCharacteristic = pRemoteService->getCharacteristic(BLEUUID(HID_REPORT_CHAR_UUID));
    if (pReportCharacteristic == nullptr) {
        Serial.println("Failed to find HID report characteristic.");
        return;
    }
    Serial.println("HID report characteristic found.");

    // Enable notifications for the report characteristic
    if (!pReportCharacteristic->canNotify()) {
        Serial.println("Notifications are not supported for HID report characteristic.");
        return;
    }
    Serial.println("Enabling notifications for HID report characteristic...");
    pReportCharacteristic->getDescriptor(BLEUUID(CLIENT_CHARACTERISTIC_CONFIG_UUID))->writeValue("0100", true);

    // Register for notifications
    pReportCharacteristic->registerForNotify(notifyCallback);
    Serial.println("Registered for notifications.");

    // Initialize servo
    servo.attach(SERVO_PIN);
    Serial.println("Servo initialized.");
}

void loop() {
    // You can add additional functionality here if needed
}
