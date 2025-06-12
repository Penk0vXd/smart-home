#include <Wire.h>

// Define the SDA and SCL pins
#define SDA_PIN 23  // Using your SDA pin
#define SCL_PIN 22  // Using your SCL pin

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("\nI2C Scanner");
  
  // Initialize I2C with your specific pins
  Wire.begin(SDA_PIN, SCL_PIN);
}

void loop() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("Scanning for I2C devices...");
  
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
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("Done scanning\n");
  }
  
  delay(5000); // Wait 5 seconds before scanning again
} 