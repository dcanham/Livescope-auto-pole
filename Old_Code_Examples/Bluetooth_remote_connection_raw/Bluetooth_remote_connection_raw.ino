#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLECharacteristic.h>

#define HID_SERVICE_UUID "00001812-0000-1000-8000-00805f9b34fb"
#define HID_REPORT_CHAR_UUID "00002A4D-0000-1000-8000-00805f9b34fb"
#define CLIENT_CHARACTERISTIC_CONFIG_UUID "00002902-0000-1000-8000-00805f9b34fb"

BLEClient* pClient;
BLERemoteCharacteristic* pReportCharacteristic;

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (isNotify) {
        Serial.print("Notification received from characteristic: ");
        Serial.println(String(pCharacteristic->getUUID().toString().c_str()));
        Serial.print("Received HID Report: ");
        for (int i = 0; i < length; i++) {
            Serial.print(pData[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
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
}

void loop() {
    // You can add additional functionality here if needed
}
