#include <TFT_eSPI.h> // Include the library

// Create an instance of the library
TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Initialize display
  tft.init();
  
  // Set screen rotation to horizontal. Adjust the value as needed for your display
  tft.setRotation(1); // Try values 0, 1, 2, or 3
  
  // Clear the screen to black
  tft.fillScreen(TFT_BLACK);
  
  // Set the text color to white
  tft.setTextColor(TFT_WHITE);
  
  // Set the text size
  tft.setTextSize(2);
  
  // Define a string variable with the text you want to display
  String textToDisplay = "Hello World";
  
  // Set the cursor position where the text will start
  tft.setCursor(0, 0);
  
  // Print the text contained in the string variable to the screen
  tft.println(textToDisplay);
}

void loop() {
  // No need to do anything here for this example
}
