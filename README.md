# ESP32 Smart Home System

## ✅ Overview

This project implements a complete smart home system using an ESP32 DevKit V1 board, allowing control of multiple devices through MQTT protocol using HiveMQ Cloud as the broker. The system provides:

- Secure connection to MQTT broker using TLS
- Control of 3 LED lights via MQTT topics
- Temperature and humidity monitoring with DHT22 sensor
- Alarm system with a multi-tone buzzer that can be triggered via MQTT or physical button
- Web interface for local control with real-time sensor readings
- Automatic reconnection to WiFi and MQTT broker

## 🔌 Wiring Diagram

```
ESP32 DevKit V1     |     Components
-----------------------------------------
GPIO12 -------------- LED1 (+) → Resistor → GND
GPIO13 -------------- LED2 (+) → Resistor → GND
GPIO14 -------------- LED3 (+) → Resistor → GND
GPIO4  -------------- DHT22 Data Pin (2nd pin)
GPIO21 -------------- Buzzer1 (+) → GND
GPIO22 -------------- Buzzer2 (+) → GND
GPIO23 -------------- Buzzer3 (+) → GND
GPIO25 -------------- Buzzer4 (+) → GND
GPIO5  -------------- Push Button → GND
3.3V   -------------- DHT22 VCC (1st pin)
                       Push Button (w/ 10k pull-up resistor)
GND    -------------- DHT22 GND (4th pin)
                       LEDs, Buzzers, Button
```

### Component Requirements:
- 1× ESP32 DevKit V1
- 3× LEDs (any color)
- 3× 220Ω Resistors (for LEDs)
- 1× DHT22 Temperature & Humidity Sensor
- 4× Buzzers (or 1 if space limited)
- 1× Push Button
- 1× 10kΩ Resistor (for button pull-up)
- Breadboard and jumper wires

## 📋 Arduino Code Explanation

The code is structured into the following sections:

1. **Initialization and Configuration**
   - Pin definitions and MQTT topics
   - WiFi and MQTT broker settings
   - Sensor and device state variables

2. **Web Server Implementation**
   - Embedded HTML/CSS/JS for responsive web interface
   - API endpoints for device control and sensor data

3. **MQTT Functionality**
   - Secure TLS connection with `setInsecure()`
   - Unique client ID based on ESP32 MAC address
   - Topic subscription and message handling
   - Status publishing for all devices

4. **Sensor and Device Control**
   - DHT22 temperature/humidity reading
   - LED control via GPIO outputs
   - Buzzer alarm pattern generation
   - Button debounce handling

5. **Robust Connection Management**
   - WiFi reconnection logic
   - MQTT broker reconnection handling
   - Error detection and reporting

## 🖥️ MQTT Topic Table

| Topic                    | Direction | Data Format | Purpose                           |
|--------------------------|-----------|-------------|-----------------------------------|
| home/lights/1            | Subscribe | "ON"/"OFF"  | Turn LED 1 on or off              |
| home/lights/2            | Subscribe | "ON"/"OFF"  | Turn LED 2 on or off              |
| home/lights/3            | Subscribe | "ON"/"OFF"  | Turn LED 3 on or off              |
| home/lights/1/status     | Publish   | "ON"/"OFF"  | Current state of LED 1            |
| home/lights/2/status     | Publish   | "ON"/"OFF"  | Current state of LED 2            |
| home/lights/3/status     | Publish   | "ON"/"OFF"  | Current state of LED 3            |
| home/sensor/temperature  | Publish   | Float (°C)  | Current temperature reading       |
| home/sensor/humidity     | Publish   | Float (%)   | Current humidity reading          |
| home/buzzer/command      | Subscribe | "ON"/"OFF"  | Activate/deactivate buzzer alarm  |
| home/buzzer/status       | Publish   | "ON"/"OFF"  | Current state of buzzer alarm     |

## 🔧 Troubleshooting Tips

### Connection Issues
1. **WiFi Connection Failures**
   - Verify SSID and password are correct
   - Ensure ESP32 is within range of WiFi router
   - Check if router has too many connected devices

2. **MQTT Connection Failures**
   - Verify HiveMQ Cloud credentials (username/password)
   - Check error code from `mqttClient.state()` for specific issues:
     - `-4`: Connection timeout - Check network speed
     - `-3`: Connection lost - Check network stability
     - `-2`: Connect failed - Verify broker URL is correct
     - `4`: Bad credentials - Check username/password
     - `5`: Not authorized - Check permissions in HiveMQ dashboard

3. **SSL/TLS Problems**
   - Using `setInsecure()` bypasses certificate verification, which may not work on all networks
   - For higher security, use a proper CA certificate and `wifiClient.setCACert()`

### Hardware Issues
1. **DHT22 Readings Failed**
   - Check wiring connections (VCC, GND, Data)
   - Ensure 3.3V power is stable
   - Try a different DHT22 sensor if problems persist

2. **LEDs Not Working**
   - Verify correct GPIO pins (12, 13, 14)
   - Check LED polarity (longer leg is positive)
   - Confirm resistors are connected properly

3. **Buzzer Not Sounding**
   - Check wiring connections for all buzzers
   - Verify GPIO pins (21, 22, 23, 25)
   - Test with direct HIGH/LOW signals to isolate issue

4. **Button Not Triggering**
   - Verify pull-up resistor is connected
   - Check GPIO pin (5) configuration
   - Test button continuity with multimeter

### Software Troubleshooting
1. Run serial monitor at 115200 baud to view debug messages
2. Monitor MQTT messages using a client like MQTT Explorer
3. Use ArduinoOTA or Web Server to update firmware remotely
4. If ESP32 crashes, check for:
   - Memory leaks in loops
   - Stack overflow from recursive functions
   - Power supply issues (use capacitor between VCC/GND) 