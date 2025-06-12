#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>  // Add JSON support

// LED Pins
#define LED1_PIN 12
#define LED2_PIN 13
#define LED3_PIN 14

#define DHTPIN 4     // DHT22 data pin
#define DHTTYPE DHT22   // DHT 22 sensor type

// Buzzer pin - using a single 3-pin active buzzer module
#define BUZZER_PIN 21  // Active buzzer module on pin 21
#define BUTTON_PIN 5   // Physical button pin

// WiFi credentials
const char* ssid = "MIRKO";
const char* password = "12345678 ";

// MQTT настройки
const char* mqtt_server = "7fad8ef106584fd6951e4a14e11fc5ea.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "PenkovXD_19";
const char* mqtt_pass   = "FYK_hxf2dct.emu@afc";

// MQTT topics
const char* topic_light1 = "home/lights/1";
const char* topic_light2 = "home/lights/2";
const char* topic_light3 = "home/lights/3";
const char* topic_light1_status = "home/lights/1/status";
const char* topic_light2_status = "home/lights/2/status";
const char* topic_light3_status = "home/lights/3/status";
const char* topic_temperature = "home/sensor/temperature";
const char* topic_humidity = "home/sensor/humidity";
const char* topic_buzzer_command = "home/buzzer/command";
const char* topic_buzzer_status = "home/buzzer/status";

// MQTT publish interval
unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 5000; // Publish MQTT data every 5 seconds

// SSL Certificate for HiveMQ Cloud
// DigiCert Global Root CA
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

// LED states - exactly 3 LEDs
int ledStates[3] = {0, 0, 0};  // LED states for the 3 LEDs
float temperature = 0;
float humidity = 0;
unsigned long lastDHTReadTime = 0;
const long dhtReadInterval = 2000; // Read DHT every 2 seconds
bool dhtInitialized = false;
int dhtRetryCount = 0;
const int maxDhtRetries = 5;

// Buzzer variables
bool buzzerActive = false;  // Make sure this is initialized to false
unsigned long buzzerLastChange = 0;
unsigned long buzzerToneChange = 0;
unsigned long buzzerHighDuration = 300;
unsigned long buzzerLowDuration = 200;
int buzzerState = 0; // 0 = off, 1 = high tone, 2 = low tone
int buzzerToneState = LOW; // Current state of the buzzer pin (HIGH or LOW)
int buzzerHighFreq = 1000;
int buzzerLowFreq = 600;
int currentFreq = 0;

// Button debounce variables
bool lastButtonState = HIGH;  // Assuming pull-up resistor
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// MQTT connection variables
bool mqttConnected = false;
unsigned long lastMqttConnectionAttempt = 0;
const long mqttReconnectInterval = 5000; // Try to reconnect every 5 seconds

WiFiClientSecure  wifiClient;
PubSubClient      mqttClient(wifiClient);
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Smart Home</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box;transition:all 0.3s}
    body{
      background:linear-gradient(135deg, #19115d, #3B82F6, #4a2e93);
      background-size:400% 400%;
      animation:gradient 15s ease infinite;
      color:#fff;
      font-family:system-ui,-apple-system,'Segoe UI',Roboto,sans-serif;
      min-height:100vh;
      padding:20px;
      display:flex;
      flex-direction:column;
      align-items:center;
      justify-content:center
    }
    @keyframes gradient{
      0%{background-position:0% 50%}
      50%{background-position:100% 50%}
      100%{background-position:0% 50%}
    }
    .container{
      width:100%;
      max-width:600px;
      background:rgba(255,255,255,0.1);
      backdrop-filter:blur(10px);
      border-radius:20px;
      padding:30px;
      box-shadow:0 8px 32px rgba(0,0,0,0.2)
    }
    h1{
      margin:0 0 30px;
      text-align:center;
      font-size:2rem;
      font-weight:700;
      letter-spacing:1px;
      text-shadow:0 2px 5px rgba(0,0,0,0.3)
    }
    .grid{
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(130px,1fr));
      gap:20px;
      margin-bottom:20px
    }
    .btn{
      position:relative;
      border:none;
      background:rgba(255,255,255,0.15);
      color:#fff;
      padding:20px 10px;
      border-radius:16px;
      display:flex;
      flex-direction:column;
      align-items:center;
      cursor:pointer;
      overflow:hidden;
      box-shadow:0 4px 12px rgba(0,0,0,0.1)
    }
    .btn:hover{transform:translateY(-3px)}
    .btn.on{
      background:rgba(73,93,255,0.4);
      box-shadow:0 6px 20px rgba(59,130,246,0.4)
    }
    .indicator{
      width:40px;
      height:40px;
      border-radius:50%;
      margin-bottom:15px;
      background:#ffffff20;
      border:2px solid rgba(255,255,255,0.3)
    }
    .btn.on .indicator{
      background:#fff;
      box-shadow:0 0 15px #fff,0 0 30px rgba(59,130,246,0.7)
    }
    .btn-name{font-weight:500;font-size:1rem}
    .status{
      position:absolute;
      top:10px;
      right:10px;
      font-size:0.7rem;
      font-weight:500;
      opacity:0.7
    }
    .sensor-card{
      background:rgba(255,255,255,0.15);
      border-radius:16px;
      padding:20px;
      margin-bottom:20px;
      box-shadow:0 4px 12px rgba(0,0,0,0.1);
      text-align:center;
    }
    .sensor-title{
      font-size:1.2rem;
      margin-bottom:15px;
      font-weight:600;
    }
    .sensor-data{
      display:flex;
      justify-content:space-around;
    }
    .sensor-value{
      display:flex;
      flex-direction:column;
      align-items:center;
    }
    .value{
      font-size:1.8rem;
      font-weight:700;
      margin-bottom:5px;
    }
    .label{
      font-size:0.9rem;
      opacity:0.8;
    }
    .footer{
      margin-top:20px;
      text-align:center;
      font-size:0.8rem;
      opacity:0.7
    }
    @media(max-width:500px){
      .container{padding:20px 15px}
      h1{font-size:1.5rem;margin-bottom:20px}
      .grid{gap:12px}
      .btn{padding:15px 10px}
      .indicator{width:30px;height:30px;margin-bottom:10px}
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Smart Home Lighting</h1>
    <div class="sensor-card">
      <div class="sensor-title">DHT22 Sensor</div>
      <div class="sensor-data">
        <div class="sensor-value">
          <div class="value" id="temp">--&#176;C</div>
          <div class="label">Temperature</div>
        </div>
        <div class="sensor-value">
          <div class="value" id="hum">--%</div>
          <div class="label">Humidity</div>
        </div>
      </div>
    </div>
    <div class="grid">
      <button class="btn" id="l0" onclick="t(0)">
        <div class="indicator"></div>
        <span class="btn-name">LED 1</span>
        <span class="status">OFF</span>
      </button>
      <button class="btn" id="l1" onclick="t(1)">
        <div class="indicator"></div>
        <span class="btn-name">LED 2</span>
        <span class="status">OFF</span>
      </button>
      <button class="btn" id="l2" onclick="t(2)">
        <div class="indicator"></div>
        <span class="btn-name">LED 3</span>
        <span class="status">OFF</span>
      </button>
    </div>
    <div class="footer">Home Automation System</div>
  </div>
  <script>
    let s=[0,0,0];  // Using numeric values (0/1) for exactly 3 LEDs
    let temp = 0;
    let hum = 0;
    
    function u(){
      for(let i=0;i<3;i++){  // Loop through exactly 3 LEDs
        const btn=document.getElementById('l'+i);
        btn.classList[s[i]?'add':'remove']('on');
        btn.querySelector('.status').textContent=s[i]?'ON':'OFF';
      }
      
      // Format temperature and humidity with one decimal place
      const tempFormatted = (temp === null || isNaN(temp)) ? "--" : parseFloat(temp).toFixed(1);
      const humFormatted = (hum === null || isNaN(hum)) ? "--" : parseFloat(hum).toFixed(1);
      
      // Use String.fromCharCode for degree symbol in JavaScript
      document.getElementById('temp').textContent = tempFormatted + String.fromCharCode(176) + 'C';
      document.getElementById('hum').textContent = humFormatted + '%';
    }
    
    function t(i){
      fetch('/t?l='+i,{method:'POST'})
        .then(r=>r.json())
        .then(d=>{
          s=d.s;
          temp=d.temp;
          hum=d.hum;
          u();
        })
        .catch(e=>console.error(e));
    }
    
    function getSensorData() {
      fetch('/sensor')
        .then(r=>r.json())
        .then(d=>{
          temp=d.temp;
          hum=d.hum;
          u();
        })
        .catch(e=>console.error(e));
    }
    
    // Initial state
    fetch('/s')
      .then(r=>r.json())
      .then(d=>{
        s=d.s;
        temp=d.temp;
        hum=d.hum;
        u();
      })
      .catch(e=>console.error(e));
    
    // Update sensor data every 3 seconds
    setInterval(getSensorData, 3000);
  </script>
</body>
</html>
)rawliteral";

void setLEDs() {
  // Using direct pin values based on ledStates
  digitalWrite(LED1_PIN, ledStates[0]);
  digitalWrite(LED2_PIN, ledStates[1]);
  digitalWrite(LED3_PIN, ledStates[2]);
  
  // Debug output
  Serial.print("Setting LEDs: ");
  Serial.print(ledStates[0]);
  Serial.print(", ");
  Serial.print(ledStates[1]);
  Serial.print(", ");
  Serial.print(ledStates[2]);
}

void handleRoot() {
  // Serve the index.html file from SPIFFS if available
  if (SPIFFS.exists("/index.html")) {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    // Fallback to the embedded HTML
    server.send_P(200, "text/html", MAIN_page);
  }
}

void handleToggle() {
  if (server.hasArg("l")) {
    int idx = server.arg("l").toInt();
    if (idx >= 0 && idx < 3) {  // Only allow indices 0, 1, 2
      ledStates[idx] = !ledStates[idx];  // Toggle between 0 and 1
      setLEDs();
      
      // Debug output
      Serial.print("Toggled LED ");
      Serial.print(idx);
      Serial.print(" to ");
      Serial.println(ledStates[idx]);
      
      // Publish the change via MQTT so other interfaces stay in sync
      if (mqttClient.connected()) {
        const char* topic;
        switch (idx) {
          case 0: topic = topic_light1; break;
          case 1: topic = topic_light2; break;
          case 2: topic = topic_light3; break;
          default: return;
        }
        const char* status = ledStates[idx] ? "ON" : "OFF";
        mqttClient.publish(topic, status, true);
        
        // Also publish to status topic
        publishLedStatus(idx);
      }
    }
  }
  
  // Return numeric values in JSON
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "],"
              + "\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
  
  server.send(200, "application/json", json);
}

void handleStates() {
  // Return numeric values in JSON
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "],"
              + "\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
  
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleSensorData() {
  String json = "{\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
  
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

bool initDHT() {
  if (!dhtInitialized) {
    dht.begin();
    delay(1000); // Give the sensor time to stabilize
    
    // Try to read from the sensor to verify it's working
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
      
      delay(1000); // Wait before trying again
      return false;
    }
  }
  return true;
}

void readDHTSensor() {
  // Ensure DHT is initialized
  if (!dhtInitialized && !initDHT()) {
    Serial.println("DHT sensor not initialized, using default values");
    return;
  }
  
  // Reading temperature or humidity takes about 250 milliseconds
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  // Check if any reads failed and keep previous values
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("Failed to read from DHT sensor! Retaining previous values");
    return;
  }

  
  // Update values
  humidity = newHumidity;
  temperature = newTemperature;
}

// Set buzzer state with forced setting to ensure state change
void setBuzzer(int state) {
  // Only print debug message if state is actually changing
  if (buzzerToneState != state) {
    buzzerToneState = state;
    digitalWrite(BUZZER_PIN, state);
    
    // Only print messages when buzzer is being turned on or off explicitly by user actions
    if (state == HIGH && buzzerActive) {
      Serial.println("Buzzer turned ON");
    } else if (state == LOW && !buzzerActive) {
      Serial.println("Buzzer turned OFF");
    }
  } else {
    // Still set the pin but don't print anything
    digitalWrite(BUZZER_PIN, state);
  }
}

void toggleBuzzer() {
  buzzerActive = !buzzerActive;
  
  if (buzzerActive) {
    Serial.println("Alarm activated");
    // Turn on buzzer immediately
    setBuzzer(HIGH);
    buzzerState = 1;
    currentFreq = buzzerHighFreq;
  } else {
    Serial.println("Alarm deactivated");
    // Turn off buzzer
    setBuzzer(LOW);
    buzzerState = 0;
    buzzerToneState = LOW;
  }
  
  buzzerLastChange = millis();
  buzzerToneChange = millis();
  
  // Publish MQTT status if connected
  if (mqttClient.connected()) {
    // Publish to both command and status topics to keep all interfaces in sync
    const char* status = buzzerActive ? "ON" : "OFF";
    mqttClient.publish(topic_buzzer_command, status, true);
    publishBuzzerStatus();
  }
}

void updateBuzzer() {
  // Force buzzer off if not active, regardless of current pin state
  if (!buzzerActive) {
    // Don't spam serial monitor, just quietly ensure buzzer is off
    digitalWrite(BUZZER_PIN, LOW);
    buzzerToneState = LOW;
    return;
  }
  
  unsigned long currentMillis = millis();
  
  // For an active buzzer, use a simple on/off pattern:
  // 1 second on, 0.5 second off - this is more noticeable than faster patterns
  unsigned long period = 1500; // Total cycle length in milliseconds
  unsigned long onDuration = 1000; // Time buzzer is on in milliseconds
  
  // Calculate position in the cycle
  unsigned long cyclePosition = currentMillis % period;
  
  // Determine if the buzzer should be on or off based on cycle position
  bool shouldBeOn = (cyclePosition < onDuration);
  
  // Set the buzzer state without debug messages for regular pattern updates
  if (buzzerToneState != (shouldBeOn ? HIGH : LOW)) {
    buzzerToneState = shouldBeOn ? HIGH : LOW;
    digitalWrite(BUZZER_PIN, buzzerToneState);
  }
}

void checkButton() {
  // Read the button state
  int reading = digitalRead(BUTTON_PIN);
  
  // Check if the button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // If the button state has been stable for the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // If the button is pressed (LOW with pull-up resistor)
      if (currentButtonState == LOW) {
        Serial.println("Button pressed - toggling buzzer");
        toggleBuzzer();
      }
    }
  }
  
  // Save the current reading for the next comparison
  lastButtonState = reading;
}

// MQTT callback function for handling incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string for easier handling
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Handle light commands
  if (strcmp(topic, topic_light1) == 0) {
    ledStates[0] = (message == "ON") ? 1 : 0;
    publishLedStatus(0);
  } 
  else if (strcmp(topic, topic_light2) == 0) {
    ledStates[1] = (message == "ON") ? 1 : 0;
    publishLedStatus(1);
  }
  else if (strcmp(topic, topic_light3) == 0) {
    ledStates[2] = (message == "ON") ? 1 : 0;
    publishLedStatus(2);
  }
  else if (strcmp(topic, topic_buzzer_command) == 0) {
    bool previousState = buzzerActive;
    buzzerActive = (message == "ON");
    
    Serial.print("Buzzer command received via MQTT: ");
    Serial.println(buzzerActive ? "ON" : "OFF");
    
    if (buzzerActive != previousState) {
      if (buzzerActive) {
        Serial.println("Alarm activated via MQTT");
        // Start with high tone and turn on immediately
        setBuzzer(HIGH);
        buzzerState = 1;
        currentFreq = buzzerHighFreq;
      } else {
        Serial.println("Alarm deactivated via MQTT");
        // Turn off buzzer
        setBuzzer(LOW);
        buzzerState = 0;
        buzzerToneState = LOW;
      }
      buzzerLastChange = millis();
      buzzerToneChange = millis();
      
      // Publish buzzer status
      publishBuzzerStatus();
    }
  }
  
  setLEDs();
}

// Use MAC address for unique client ID
String getUniqueClientId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId = "ESP32-";
  for (int i = 0; i < 6; i++) {
    clientId += String(mac[i], HEX);
  }
  return clientId;
}

// Connect to MQTT broker
bool connectMQTT() {
  if (mqttClient.connected()) {
    return true;
  }
  
  Serial.println("Connecting to MQTT broker...");
  
  // Generate unique client ID based on MAC address
  String clientId = getUniqueClientId();
  Serial.print("Using client ID: ");
  Serial.println(clientId);
  
  // Connect with credentials
  if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT broker");
    
    // Subscribe to topics - exactly 3 LEDs and buzzer
    mqttClient.subscribe(topic_light1);
    mqttClient.subscribe(topic_light2);
    mqttClient.subscribe(topic_light3);
    mqttClient.subscribe(topic_buzzer_command);
    
    // Publish initial states
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

// Publish LED status
void publishLedStatus(int ledIndex) {
  const char* topic;
  switch (ledIndex) {
    case 0: topic = topic_light1_status; break;
    case 1: topic = topic_light2_status; break;
    case 2: topic = topic_light3_status; break;
    default: return;
  }
  
  const char* status = ledStates[ledIndex] ? "ON" : "OFF";
  mqttClient.publish(topic, status, true); // retained message
}

// Publish buzzer status
void publishBuzzerStatus() {
  const char* status = buzzerActive ? "ON" : "OFF";
  mqttClient.publish(topic_buzzer_status, status, true); // retained message
}

// Publish temperature and humidity values
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

// Publish all status (for initial connection and reconnects)
void publishAllStatus() {
  // Publish all LED statuses - exactly 3 LEDs
  for (int i = 0; i < 3; i++) {
    publishLedStatus(i);
  }
  
  // Publish buzzer status
  publishBuzzerStatus();
  
  // Publish sensor data
  publishSensorData();
}

// Handle updates from external web pages
void handleExternalUpdate() {
  String message = "Update failed";
  int statusCode = 400;
  
  // Check if we have the right parameters
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Received update: " + body);
    
    // Try to parse JSON
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error) {
      // Check if it's a light update
      if (doc.containsKey("light") && doc.containsKey("state")) {
        int lightIndex = doc["light"].as<int>();
        bool state = doc["state"].as<bool>();
        
        if (lightIndex >= 0 && lightIndex < 3) {
          ledStates[lightIndex] = state ? 1 : 0;
          setLEDs();
          
          // Publish via MQTT to keep everything in sync
          if (mqttClient.connected()) {
            const char* topic;
            switch (lightIndex) {
              case 0: topic = topic_light1; break;
              case 1: topic = topic_light2; break;
              case 2: topic = topic_light3; break;
              default: break;
            }
            const char* status = ledStates[lightIndex] ? "ON" : "OFF";
            mqttClient.publish(topic, status, true);
            publishLedStatus(lightIndex);
          }
          
          message = "Light " + String(lightIndex) + " set to " + String(ledStates[lightIndex]);
          statusCode = 200;
        }
      }
      // Check if it's a buzzer update
      else if (doc.containsKey("buzzer")) {
        bool state = doc["buzzer"].as<bool>();
        
        Serial.print("Buzzer command received via HTTP: ");
        Serial.println(state ? "ON" : "OFF");
        
        // Only toggle if current state is different
        if (buzzerActive != state) {
          buzzerActive = state;
          
          if (buzzerActive) {
            // Start buzzer
            setBuzzer(HIGH);
            buzzerState = 1;
            currentFreq = buzzerHighFreq;
          } else {
            // Stop buzzer
            setBuzzer(LOW);
            buzzerState = 0;
            buzzerToneState = LOW;
          }
          
          buzzerLastChange = millis();
          buzzerToneChange = millis();
          
          // Publish via MQTT
          if (mqttClient.connected()) {
            const char* status = buzzerActive ? "ON" : "OFF";
            mqttClient.publish(topic_buzzer_command, status, true);
            publishBuzzerStatus();
          }
        }
        
        message = "Buzzer set to " + String(buzzerActive ? "ON" : "OFF");
        statusCode = 200;
      }
    } else {
      message = "Invalid JSON: " + String(error.c_str());
    }
  }
  
  // Return response with appropriate CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(statusCode, "text/plain", message);
}

// Handle buzzer status request
void handleBuzzerStatus() {
  String json = "{\"active\":" + String(buzzerActive ? "true" : "false") + "}";
  
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
  
  Serial.print("Buzzer status requested. Current state: ");
  Serial.println(buzzerActive ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println("\n\n--- ESP32 Smart Home Lighting Starting ---");
  
  // Initialize pin modes
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);  // Single buzzer pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize buzzer to OFF state - call twice to ensure it's off
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
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
  } else {
    Serial.println("SPIFFS initialized successfully");
  }
  
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
  } else {
    Serial.println("\nWiFi connection failed!");
  }
  
  // Setup MQTT
  wifiClient.setInsecure(); // Use setInsecure instead of certificates
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  // Try to connect to MQTT server
  connectMQTT();
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/t", handleToggle);
  server.on("/s", handleStates);
  server.on("/sensor", handleSensorData);
  server.on("/buzzer", handleBuzzerStatus);  // Add buzzer status endpoint
  server.on("/update", HTTP_POST, handleExternalUpdate);
  
  // Enable CORS for cross-origin requests
  server.enableCORS(true);
  
  // Start web server
  server.begin();
  Serial.println("HTTP server started");
  
  // Set all LEDs to initial state
  setLEDs();
}

void loop() {
  server.handleClient();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    // If WiFi disconnected, try to reconnect
    static unsigned long lastWiFiReconnectAttempt = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastWiFiReconnectAttempt >= 30000) { // Try every 30 seconds
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      lastWiFiReconnectAttempt = currentMillis;
      WiFi.reconnect();
    }
  } else {
    // Handle MQTT connection and messages
    if (!mqttClient.connected()) {
      // Try to reconnect periodically
      unsigned long currentMillis = millis();
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
      // Process MQTT messages
      mqttClient.loop();
    }
  }
  
  // Read DHT sensor at intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTReadTime >= dhtReadInterval) {
    lastDHTReadTime = currentMillis;
    readDHTSensor();
  }
  
  // Publish MQTT data periodically
  if (mqttClient.connected() && (currentMillis - lastMqttPublish >= mqttPublishInterval)) {
    lastMqttPublish = currentMillis;
    publishSensorData();
  }
  
  // Check button state
  checkButton();
  
  // Update buzzer sound pattern
  updateBuzzer();
  
  // Add additional safety check - if not supposed to be active, ensure it's off
  if (!buzzerActive && digitalRead(BUZZER_PIN) == HIGH) {
    digitalWrite(BUZZER_PIN, LOW);
    // Only print this message once in a while to avoid spamming
    static unsigned long lastSafetyMessage = 0;
    if (millis() - lastSafetyMessage > 5000) {
      Serial.println("Safety check: Turning off buzzer that should be inactive");
      lastSafetyMessage = millis();
    }
  }
} 