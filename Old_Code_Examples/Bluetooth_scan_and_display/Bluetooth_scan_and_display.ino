#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>

#define TFT_CS  5
#define TFT_RST -1
#define TFT_DC   4

#define BUTTON_SCROLL 35
#define BUTTON_SELECT 0

TFT_eSPI tft = TFT_eSPI();  // Invoke library
BLEScan* pBLEScan;

bool scanning = false;
int scrollIndex = 0;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (scrollIndex < 0) {
      scrollIndex = 0;
    }
    if (scrollIndex > 9) {
      scrollIndex = 9;
    }
    String deviceName = advertisedDevice.getName().c_str();
    Serial.print("Device found: ");
    Serial.println(deviceName);
    tft.setCursor(10, 20 + (scrollIndex * 20));
    tft.print(deviceName);
    scrollIndex++;
    if (scrollIndex >= 10) {
      scanning = false;
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_SCROLL, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
}

void loop() {
  if (digitalRead(BUTTON_SCROLL) == HIGH) {
    delay(50); // Debouncing delay
    if (digitalRead(BUTTON_SCROLL) == HIGH) {
      scrollIndex++;
      scanning = true;
    }
  }

  if (digitalRead(BUTTON_SELECT) == HIGH) {
    delay(50); // Debouncing delay
    if (digitalRead(BUTTON_SELECT) == HIGH) {
      // Connect to the selected device
      // You need to implement this part
    }
  }

  if (scanning) {
    scanForDevices();
  }
}

void scanForDevices() {
  Serial.println("Scanning for BLE devices...");
  pBLEScan->start(5);
  delay(5000); // Scanning duration
  pBLEScan->stop();
  Serial.println("Scan done!");
}
