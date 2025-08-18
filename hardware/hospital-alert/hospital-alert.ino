#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Pin definitions
#define LED_PIN D0
#define BUZZER_PIN D1

// Touch calibration values (you may need to adjust these for your display)
#define TOUCH_CS_PIN 4  // D2 on ESP8266 (GPIO4)

// Button properties
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 80
#define BUTTON_X ((tft.width() - BUTTON_WIDTH) / 2)
#define BUTTON_Y ((tft.height() - BUTTON_HEIGHT) / 2)

// Colors
#define GREEN_COLOR TFT_GREEN
#define RED_COLOR TFT_RED
#define WHITE_TEXT TFT_WHITE
#define BLACK_TEXT TFT_BLACK

// State management
bool isEmergency = false;
bool lastTouchState = false;
unsigned long lastTouchTime = 0;
const unsigned long DEBOUNCE_DELAY = 200; // 200ms debounce

void setup() {
  Serial.begin(115200);
  Serial.println("TFT Touch Button Starting...");
  
  // Initialize LED and Buzzer pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize TFT
  tft.init();
  tft.setRotation(1); // Adjust rotation as needed
  tft.fillScreen(TFT_BLACK);
  
  // Setup indicator: LED blink and buzzer beep once
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Draw initial button state
  drawButton();
  
  Serial.println("all_good"); // Initial state
}

void loop() {
  // Check for touch
  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);
  
  // Debounce touch input
  if (touched && !lastTouchState && (millis() - lastTouchTime > DEBOUNCE_DELAY)) {
    // Check if touch is within button area
    if (x >= BUTTON_X && x <= (BUTTON_X + BUTTON_WIDTH) && 
        y >= BUTTON_Y && y <= (BUTTON_Y + BUTTON_HEIGHT)) {
      
      // Toggle state
      isEmergency = !isEmergency;
      
      // Control LED and buzzer based on emergency state
      if (isEmergency) {
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
      }
      
      // Redraw button
      drawButton();
      
      // Send serial data
      if (isEmergency) {
        Serial.println("emergency");
      } else {
        Serial.println("all_good");
      }
      
      lastTouchTime = millis();
    }
  }
  
  lastTouchState = touched;
  
  delay(50); // Small delay to prevent excessive polling
}

void drawButton() {
  // Clear button area
  tft.fillRect(BUTTON_X - 10, BUTTON_Y - 10, BUTTON_WIDTH + 20, BUTTON_HEIGHT + 20, TFT_BLACK);
  
  // Draw button background
  uint16_t buttonColor = isEmergency ? RED_COLOR : GREEN_COLOR;
  tft.fillRoundRect(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 10, buttonColor);
  
  // Draw button border
  tft.drawRoundRect(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 10, TFT_WHITE);
  
  // Set text properties
  tft.setTextColor(WHITE_TEXT, buttonColor);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM); // Middle center alignment
  
  // Draw text
  String buttonText = isEmergency ? "EMERGENCY" : "ALL GOOD";
  tft.drawString(buttonText, BUTTON_X + BUTTON_WIDTH/2, BUTTON_Y + BUTTON_HEIGHT/2);
}

// Optional: Touch calibration function
// Call this once to get calibration values for your specific display
void touchCalibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  tft.println("Touch corners as indicated");
  
  tft.setTextFont(1);
  tft.println();
  
  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  
  Serial.println();
  Serial.println("Calibration complete!");
  Serial.println("Add these values to your code:");
  Serial.print("uint16_t calData[5] = {");
  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println("};");
  Serial.println("tft.setTouch(calData);");
}