#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

// LED Pins - 4 Smart Lights
#define LED1_PIN 12     // LED 1 - Living Room
#define LED2_PIN 13     // LED 2 - Kitchen  
#define LED3_PIN 14     // LED 3 - Bedroom
#define LED4_PIN 27     // LED 4 - Bathroom

// Sensor Pins
#define DHTPIN 4        // DHT22 temperature/humidity sensor data pin
#define DHTTYPE DHT22   // DHT 22 sensor type
#define RAIN_SENSOR_PIN A0  // Analog pin for rain sensor (KY-018, YL-83) - GPIO36
#define RAIN_THRESHOLD 500  // Threshold value for rain detection (adjust based on sensor)

// Control Pins
#define BUZZER_PIN 21   // Active buzzer module - Security alarm
#define BUTTON_PIN 5    // Physical push button - Manual buzzer control

// WiFi credentials
const char* ssid = "MIRKO";
const char* password = "12345678";

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;     // GMT offset in seconds
const int daylightOffset_sec = 0; // Daylight saving time offset in seconds

// MQTT settings
const char* mqtt_server = "7fad8ef106584fd6951e4a14e11fc5ea.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "PenkovXD_19";
const char* mqtt_pass   = "FYK_hxf2dct.emu@afc";

// MQTT topics - 4 Lights
const char* topic_light1 = "home/lights/1";
const char* topic_light2 = "home/lights/2";
const char* topic_light3 = "home/lights/3";
const char* topic_light4 = "home/lights/4";
const char* topic_light1_status = "home/lights/1/status";
const char* topic_light2_status = "home/lights/2/status";
const char* topic_light3_status = "home/lights/3/status";
const char* topic_light4_status = "home/lights/4/status";
const char* topic_temperature = "home/sensor/temperature";
const char* topic_humidity = "home/sensor/humidity";
const char* topic_buzzer_command = "home/buzzer/command";
const char* topic_buzzer_status = "home/buzzer/status";
const char* topic_rain_sensor = "home/sensors/rain";
const char* topic_system_status = "home/system/status";

// MQTT publish intervals
unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 5000; // Publish sensor data every 5 seconds
unsigned long lastRainPublish = 0;
const long rainPublishInterval = 10000; // Publish rain data every 10 seconds
unsigned long lastStatusPublish = 0;
const long statusPublishInterval = 30000; // Publish system status every 30 seconds

// SSL Certificate for HiveMQ Cloud
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

// System variables
int ledStates[4] = {0, 0, 0, 0};  // LED states for the 4 LEDs
float temperature = 0;
float humidity = 0;
int rainSensorValue = 0;
bool isRaining = false;
unsigned long lastDHTReadTime = 0;
const long dhtReadInterval = 2000; // Read DHT every 2 seconds
bool dhtInitialized = false;
int dhtRetryCount = 0;
const int maxDhtRetries = 5;

// Buzzer variables
bool buzzerActive = false;
unsigned long buzzerLastChange = 0;
int buzzerState = 0; // 0 = off, 1 = on
int buzzerToneState = LOW;

// Button debounce variables
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// MQTT connection variables
bool mqttConnected = false;
unsigned long lastMqttConnectionAttempt = 0;
const long mqttReconnectInterval = 5000; // Try to reconnect every 5 seconds

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
DHT dht(DHTPIN, DHTTYPE);

void setLEDs() {
  digitalWrite(LED1_PIN, ledStates[0]);  // GPIO12
  digitalWrite(LED2_PIN, ledStates[1]);  // GPIO13
  digitalWrite(LED3_PIN, ledStates[2]);  // GPIO14
  digitalWrite(LED4_PIN, ledStates[3]);  // GPIO27
  
  Serial.print("LEDs set - GPIO12:");
  Serial.print(ledStates[0]);
  Serial.print(", GPIO13:");
  Serial.print(ledStates[1]);
  Serial.print(", GPIO14:");
  Serial.print(ledStates[2]);
  Serial.print(", GPIO27:");
  Serial.print(ledStates[3]);
  Serial.println();
}

bool initDHT() {
  if (!dhtInitialized) {
    dht.begin();
    delay(1000);
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      dhtInitialized = true;
      Serial.println("DHT22 sensor initialized successfully");
      return true;
    } else {
      dhtRetryCount++;
      Serial.print("DHT22 initialization attempt ");
      Serial.print(dhtRetryCount);
      Serial.println(" failed");
      
      if (dhtRetryCount >= maxDhtRetries) {
        Serial.println("Max DHT22 initialization retries reached. Using default values.");
        return false;
      }
      delay(1000);
      return false;
    }
  }
  return true;
}

void readDHTSensor() {
  if (!dhtInitialized && !initDHT()) {
    Serial.println("DHT sensor not initialized, using default values");
    return;
  }
  
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("Failed to read from DHT sensor! Retaining previous values");
    return;
  }
  
  humidity = newHumidity;
  temperature = newTemperature;
}

void readRainSensor() {
  // Read multiple times and average for stability
  int readings = 0;
  for (int i = 0; i < 5; i++) {
    readings += analogRead(RAIN_SENSOR_PIN);
    delay(10);
  }
  rainSensorValue = readings / 5;
  
  isRaining = (rainSensorValue < RAIN_THRESHOLD);
  
  Serial.print("Rain sensor (GPIO36/A0) raw value: ");
  Serial.print(rainSensorValue);
  Serial.print(" (threshold: ");
  Serial.print(RAIN_THRESHOLD);
  Serial.print(") - Status: ");
  Serial.println(isRaining ? "RAINING" : "DRY");
  
  // Additional debug for troubleshooting
  if (rainSensorValue == 0 || rainSensorValue == 4095) {
    Serial.println("WARNING: Rain sensor may not be connected properly!");
    Serial.println("Expected range: 0-4095, got extreme value");
  }
}

void setBuzzer(int state) {
  if (buzzerToneState != state) {
    buzzerToneState = state;
    digitalWrite(BUZZER_PIN, state);
    
    if (state == HIGH && buzzerActive) {
      Serial.println("Buzzer turned ON");
    } else if (state == LOW && !buzzerActive) {
      Serial.println("Buzzer turned OFF");
    }
  } else {
    digitalWrite(BUZZER_PIN, state);
  }
}

void toggleBuzzer() {
  buzzerActive = !buzzerActive;
  
  if (buzzerActive) {
    Serial.println("Alarm activated");
    setBuzzer(HIGH);
    buzzerState = 1;
  } else {
    Serial.println("Alarm deactivated");
    setBuzzer(LOW);
    buzzerState = 0;
    buzzerToneState = LOW;
  }
  
  buzzerLastChange = millis();
  
  if (mqttClient.connected()) {
    const char* status = buzzerActive ? "ON" : "OFF";
    mqttClient.publish(topic_buzzer_command, status, true);
    publishBuzzerStatus();
  }
}

void updateBuzzer() {
  if (!buzzerActive) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerToneState = LOW;
    return;
  }
  
  unsigned long currentMillis = millis();
  unsigned long period = 1500;
  unsigned long onDuration = 1000;
  unsigned long cyclePosition = currentMillis % period;
  bool shouldBeOn = (cyclePosition < onDuration);
  
  if (buzzerToneState != (shouldBeOn ? HIGH : LOW)) {
    buzzerToneState = shouldBeOn ? HIGH : LOW;
    digitalWrite(BUZZER_PIN, buzzerToneState);
  }
}

void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      if (currentButtonState == LOW) {
        Serial.println("Button pressed - toggling buzzer");
        toggleBuzzer();
      }
    }
  }
  
  lastButtonState = reading;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  if (strcmp(topic, topic_light1) == 0) {
    Serial.println("Processing Light 1 (GPIO12) command");
    ledStates[0] = (message == "ON") ? 1 : 0;
    publishLedStatus(0);
  } 
  else if (strcmp(topic, topic_light2) == 0) {
    Serial.println("Processing Light 2 (GPIO13) command");
    ledStates[1] = (message == "ON") ? 1 : 0;
    publishLedStatus(1);
  }
  else if (strcmp(topic, topic_light3) == 0) {
    Serial.println("Processing Light 3 (GPIO14) command");
    ledStates[2] = (message == "ON") ? 1 : 0;
    publishLedStatus(2);
  }
  else if (strcmp(topic, topic_light4) == 0) {
    Serial.println("Processing Light 4 (GPIO27) command");
    ledStates[3] = (message == "ON") ? 1 : 0;
    publishLedStatus(3);
  }
  else if (strcmp(topic, topic_buzzer_command) == 0) {
    bool previousState = buzzerActive;
    buzzerActive = (message == "ON");
    
    Serial.print("Buzzer command received via MQTT: ");
    Serial.println(buzzerActive ? "ON" : "OFF");
    
    if (buzzerActive != previousState) {
      if (buzzerActive) {
        Serial.println("Alarm activated via MQTT");
        setBuzzer(HIGH);
        buzzerState = 1;
      } else {
        Serial.println("Alarm deactivated via MQTT");
        setBuzzer(LOW);
        buzzerState = 0;
        buzzerToneState = LOW;
      }
      buzzerLastChange = millis();
      publishBuzzerStatus();
    }
  }
  else if (strcmp(topic, "home/sensors/rain/test") == 0) {
    Serial.println("Rain sensor test command received!");
    if (message == "RAIN") {
      Serial.println("Simulating RAIN detected");
      rainSensorValue = 200;  // Force rain detection
      isRaining = true;
      publishRainData();
    } else if (message == "DRY") {
      Serial.println("Simulating DRY conditions");
      rainSensorValue = 800;  // Force dry detection
      isRaining = false;
      publishRainData();
    } else if (message == "READ") {
      Serial.println("Forcing rain sensor reading...");
      readRainSensor();
      publishRainData();
    }
  }
  else {
    Serial.print("Unknown topic received: ");
    Serial.println(topic);
  }
  
  setLEDs();
}

String getUniqueClientId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId = "ESP32-";
  for (int i = 0; i < 6; i++) {
    clientId += String(mac[i], HEX);
  }
  return clientId;
}

bool connectMQTT() {
  if (mqttClient.connected()) {
    return true;
  }
  
  Serial.println("Connecting to MQTT broker...");
  
  String clientId = getUniqueClientId();
  Serial.print("Using client ID: ");
  Serial.println(clientId);
  
  if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT broker");
    
    mqttClient.subscribe(topic_light1);
    mqttClient.subscribe(topic_light2);
    mqttClient.subscribe(topic_light3);
    mqttClient.subscribe(topic_light4);
    mqttClient.subscribe(topic_buzzer_command);
    mqttClient.subscribe("home/sensors/rain/test");  // Rain sensor test commands
    
    Serial.println("Subscribed to all MQTT topics including rain test commands");
    
    publishAllStatus();
    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.print(mqttClient.state());
    switch(mqttClient.state()) {
      case -4: Serial.println(" (MQTT_CONNECTION_TIMEOUT)"); break;
      case -3: Serial.println(" (MQTT_CONNECTION_LOST)"); break;
      case -2: Serial.println(" (MQTT_CONNECT_FAILED)"); break;
      case -1: Serial.println(" (MQTT_DISCONNECTED)"); break;
      case 1: Serial.println(" (MQTT_CONNECT_BAD_PROTOCOL)"); break;
      case 2: Serial.println(" (MQTT_CONNECT_BAD_CLIENT_ID)"); break;
      case 3: Serial.println(" (MQTT_CONNECT_UNAVAILABLE)"); break;
      case 4: Serial.println(" (MQTT_CONNECT_BAD_CREDENTIALS)"); break;
      case 5: Serial.println(" (MQTT_CONNECT_UNAUTHORIZED)"); break;
      default: Serial.println(" (Unknown error)");
    }
    return false;
  }
}

void publishLedStatus(int ledIndex) {
  const char* topic;
  switch (ledIndex) {
    case 0: topic = topic_light1_status; break;
    case 1: topic = topic_light2_status; break;
    case 2: topic = topic_light3_status; break;
    case 3: topic = topic_light4_status; break;
    default: return;
  }
  
  const char* status = ledStates[ledIndex] ? "ON" : "OFF";
  mqttClient.publish(topic, status, true);
}

void publishBuzzerStatus() {
  const char* status = buzzerActive ? "ON" : "OFF";
  mqttClient.publish(topic_buzzer_status, status, true);
}

void publishSensorData() {
  char tempStr[10];
  char humStr[10];
  
  dtostrf(temperature, 1, 1, tempStr);
  dtostrf(humidity, 1, 1, humStr);
  
  bool tempSuccess = mqttClient.publish(topic_temperature, tempStr, true);
  bool humSuccess = mqttClient.publish(topic_humidity, humStr, true);
  
  if (!tempSuccess || !humSuccess) {
    Serial.println("Failed to publish sensor data to MQTT");
  }
}

void publishRainData() {
  // Get current timestamp
  time_t now;
  time(&now);
  
  // Create JSON payload
  DynamicJsonDocument doc(128);
  doc["raining"] = isRaining;
  doc["sensor_value"] = rainSensorValue;
  doc["timestamp"] = now;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.print("Attempting to publish rain data to topic '");
  Serial.print(topic_rain_sensor);
  Serial.print("': ");
  Serial.println(payload);
  
  bool success = mqttClient.publish(topic_rain_sensor, payload.c_str(), true);
  
  if (success) {
    Serial.println("✓ Rain data published successfully");
  } else {
    Serial.println("✗ Failed to publish rain data to MQTT");
    Serial.print("MQTT Client State: ");
    Serial.println(mqttClient.state());
    Serial.print("MQTT Connected: ");
    Serial.println(mqttClient.connected() ? "Yes" : "No");
  }
}

void publishSystemStatus() {
  // Get system information
  long rssi = WiFi.RSSI();
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t heapSize = ESP.getHeapSize();
  float heapUsage = ((float)(heapSize - freeHeap) / heapSize) * 100;
  
  // Create JSON payload
  DynamicJsonDocument doc(256);
  doc["wifi_rssi"] = rssi;
  doc["free_heap"] = freeHeap;
  doc["heap_usage_percent"] = heapUsage;
  doc["mqtt_connected"] = mqttClient.connected();
  doc["uptime_ms"] = millis();
  
  String payload;
  serializeJson(doc, payload);
  
  bool success = mqttClient.publish(topic_system_status, payload.c_str(), true);
  
  if (success) {
    Serial.print("System status published: ");
    Serial.println(payload);
  } else {
    Serial.println("Failed to publish system status to MQTT");
  }
  
  // Also print to serial for debugging
  Serial.print("WiFi RSSI: ");
  Serial.print(rssi);
  Serial.print(" dBm, Free Heap: ");
  Serial.print(freeHeap);
  Serial.print(" bytes (");
  Serial.print(heapUsage, 1);
  Serial.print("% used), MQTT: ");
  Serial.println(mqttClient.connected() ? "Connected" : "Disconnected");
}

void publishAllStatus() {
  for (int i = 0; i < 4; i++) {
    publishLedStatus(i);
  }
  publishBuzzerStatus();
  publishSensorData();
  publishRainData();
  publishSystemStatus();
}

void initTimeSync() {
  Serial.println("Initializing NTP time sync...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set
  int attempts = 0;
  while (!time(nullptr) && attempts < 10) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }
  
  if (!time(nullptr)) {
    Serial.println("\nFailed to initialize NTP time");
  } else {
    Serial.println("\nNTP time synchronized successfully");
    time_t now = time(nullptr);
    Serial.print("Current time: ");
    Serial.println(ctime(&now));
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println("\n\n--- ESP32 Smart Home MQTT Client Starting ---");
  
  // Initialize pin modes
  Serial.println("Setting up GPIO pins:");
  pinMode(LED1_PIN, OUTPUT);      // LED 1 - Living Room (GPIO12)
  Serial.println("✓ GPIO12 (LED1) set as OUTPUT");
  pinMode(LED2_PIN, OUTPUT);      // LED 2 - Kitchen (GPIO13)
  Serial.println("✓ GPIO13 (LED2) set as OUTPUT");
  pinMode(LED3_PIN, OUTPUT);      // LED 3 - Bedroom (GPIO14)
  Serial.println("✓ GPIO14 (LED3) set as OUTPUT");
  pinMode(LED4_PIN, OUTPUT);      // LED 4 - Bathroom (GPIO27)
  Serial.println("✓ GPIO27 (LED4) set as OUTPUT");
  pinMode(BUZZER_PIN, OUTPUT);    // Buzzer - Security Alarm (GPIO21)
  Serial.println("✓ GPIO21 (Buzzer) set as OUTPUT");
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button - Manual Control (GPIO5)
  Serial.println("✓ GPIO5 (Button) set as INPUT_PULLUP");
  // Rain sensor (GPIO36/A0) - No pinMode needed for analog pins
  Serial.println("✓ GPIO36 (Rain Sensor) ready for analog reading");
  
  // Initialize buzzer to OFF state
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  buzzerActive = false;
  buzzerToneState = LOW;
  
  // Test buzzer briefly
  Serial.println("Testing buzzer...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Buzzer test complete");
  
  // Initialize DHT22
  initDHT();
  
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize NTP time synchronization
    initTimeSync();
  } else {
    Serial.println("\nWiFi connection failed!");
  }
  
  // Setup MQTT
  wifiClient.setInsecure(); // Use setInsecure for simplified TLS
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  // Try to connect to MQTT server
  connectMQTT();
  
  // Set all LEDs to initial state
  setLEDs();
  
  // Initial sensor readings
  readDHTSensor();
  readRainSensor();
  
  // Rain sensor configuration info
  Serial.println("\n=== RAIN SENSOR CONFIGURATION ===");
  Serial.print("Pin: GPIO");
  Serial.print(RAIN_SENSOR_PIN);
  Serial.println(" (A0)");
  Serial.print("Threshold: ");
  Serial.print(RAIN_THRESHOLD);
  Serial.println(" (below = RAIN, above = DRY)");
  Serial.print("Initial reading: ");
  Serial.println(rainSensorValue);
  Serial.print("Initial status: ");
  Serial.println(isRaining ? "RAINING" : "DRY");
  Serial.println("Test commands available:");
  Serial.println("- Publish 'RAIN' to 'home/sensors/rain/test' to simulate rain");
  Serial.println("- Publish 'DRY' to 'home/sensors/rain/test' to simulate dry");
  Serial.println("- Publish 'READ' to 'home/sensors/rain/test' to force reading");
  Serial.println("==================================\n");
  
  Serial.println("Setup complete - entering main loop");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWiFiReconnectAttempt = 0;
    
    if (currentMillis - lastWiFiReconnectAttempt >= 30000) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      lastWiFiReconnectAttempt = currentMillis;
      WiFi.reconnect();
    }
  } else {
    // Handle MQTT connection and messages
    if (!mqttClient.connected()) {
      if (currentMillis - lastMqttConnectionAttempt >= mqttReconnectInterval) {
        lastMqttConnectionAttempt = currentMillis;
        if (connectMQTT()) {
          mqttConnected = true;
          Serial.println("MQTT reconnected successfully");
        } else {
          mqttConnected = false;
        }
      }
    } else {
      mqttClient.loop();
    }
  }
  
  // Read DHT sensor at intervals
  if (currentMillis - lastDHTReadTime >= dhtReadInterval) {
    lastDHTReadTime = currentMillis;
    readDHTSensor();
  }
  
  // Read rain sensor and publish every 10 seconds
  if (currentMillis - lastRainPublish >= rainPublishInterval) {
    lastRainPublish = currentMillis;
    readRainSensor();
    
    if (mqttClient.connected()) {
      publishRainData();
    } else {
      Serial.println("Cannot publish rain data - MQTT not connected");
    }
  }
  
  // Publish sensor data periodically
  if (mqttClient.connected() && (currentMillis - lastMqttPublish >= mqttPublishInterval)) {
    lastMqttPublish = currentMillis;
    publishSensorData();
  }
  
  // Publish system status periodically
  if (mqttClient.connected() && (currentMillis - lastStatusPublish >= statusPublishInterval)) {
    lastStatusPublish = currentMillis;
    publishSystemStatus();
  }
  
  // Check button state
  checkButton();
  
  // Update buzzer sound pattern
  updateBuzzer();
  
  // Safety check for buzzer
  if (!buzzerActive && digitalRead(BUZZER_PIN) == HIGH) {
    digitalWrite(BUZZER_PIN, LOW);
    static unsigned long lastSafetyMessage = 0;
    if (millis() - lastSafetyMessage > 5000) {
      Serial.println("Safety check: Turning off buzzer that should be inactive");
      lastSafetyMessage = millis();
    }
  }
} 