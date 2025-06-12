#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display I2C address and pins
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C  // Most common address for 0.96" OLED displays
#define SSD1306_I2C_ADDRESS_ALT 0x3D  // Alternative address that some displays use
#define OLED_SDA 23
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n--- OLED Display Test ---");
  
  // Initialize I2C connection
  Wire.begin(OLED_SDA, OLED_SCL);
  delay(100); // Short delay after I2C initialization
  
  Serial.println("Checking for I2C devices...");
  byte error, address;
  int deviceCount = 0;
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found!");
    Serial.println("Check your connections and wiring.");
  }
  
  // Try to initialize the display with the primary address
  Serial.print("Trying to initialize display at address 0x");
  Serial.print(SSD1306_I2C_ADDRESS, HEX);
  Serial.println("...");
  
  if (display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println("Display initialized with address 0x3C!");
    testDisplay();
  } else {
    // Try the alternative address
    Serial.print("Failed. Trying alternative address 0x");
    Serial.print(SSD1306_I2C_ADDRESS_ALT, HEX);
    Serial.println("...");
    
    if (display.begin(SSD1306_I2C_ADDRESS_ALT, OLED_RESET)) {
      Serial.println("Display initialized with address 0x3D!");
      testDisplay();
    } else {
      Serial.println("Display initialization failed!");
      Serial.println("Here are some troubleshooting tips:");
      Serial.println("1. Check your wiring connections (SDA, SCL, VCC, GND)");
      Serial.println("2. Verify the display is properly powered (3.3V)");
      Serial.println("3. Try different I2C pins on your ESP32");
      Serial.println("4. Try a different I2C address (some displays use 0x3C, others use 0x3D)");
      Serial.println("5. Check for damaged wires or components");
    }
  }
}

void testDisplay() {
  // Test patterns for the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Display Test");
  display.println("If you can see this");
  display.println("the display works!");
  display.display();
  delay(2000);
  
  // Draw a rectangle
  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.display();
  delay(1000);
  
  // Draw a filled rectangle
  display.clearDisplay();
  display.fillRect(10, 10, SCREEN_WIDTH-20, SCREEN_HEIGHT-20, SSD1306_WHITE);
  display.display();
  delay(1000);
  
  // Draw a circle
  display.clearDisplay();
  display.drawCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 20, SSD1306_WHITE);
  display.display();
  delay(1000);
  
  // Show a simple animation
  for (int i = 0; i < SCREEN_WIDTH; i+=4) {
    display.clearDisplay();
    display.drawLine(0, 0, i, SCREEN_HEIGHT-1, SSD1306_WHITE);
    display.display();
    delay(10);
  }
  
  // Final success message
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("SUCCESS!");
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.println("Display works!");
  display.display();
}

void loop() {
  // Nothing in the loop - test is run once at startup
  delay(1000);
} 