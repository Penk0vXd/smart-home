#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// LED Pins
#define LED1_PIN 12  // Changed back to 16 to match the working code
#define LED2_PIN 13
#define LED3_PIN 14
#define DHTPIN 4     // DHT22 data pin
#define DHTTYPE DHT22   // DHT 22 sensor type

// Buzzer pins - using 4 buzzers for louder sound
#define BUZZER1_PIN 21  // Primary buzzer pin
#define BUZZER2_PIN 22  // Additional buzzer pins
#define BUZZER3_PIN 23
#define BUZZER4_PIN 25
#define BUTTON_PIN 5   // Physical button pin

// WiFi credentials
const char* ssid = "Wifi-Name";
const char* password = "Wifi-Password";

// Using integers (0/1) instead of booleans to match JavaScript expectations
int ledStates[4] = {0, 0, 0, 0};  // Changed to int array with 4 elements
float temperature = 0;
float humidity = 0;
unsigned long lastDHTReadTime = 0;
const long dhtReadInterval = 2000; // Read DHT every 2 seconds
bool dhtInitialized = false;
int dhtRetryCount = 0;
const int maxDhtRetries = 5;

// Buzzer variables
bool buzzerActive = false;
unsigned long buzzerLastChange = 0;
unsigned long buzzerToneChange = 0;
unsigned long buzzerHighDuration = 300; // High tone duration in ms
unsigned long buzzerLowDuration = 200;  // Low tone duration in ms
int buzzerState = 0; // 0 = off, 1 = high tone, 2 = low tone
int buzzerToneState = LOW; // Current state of the buzzer pin (HIGH or LOW)
int buzzerHighFreq = 1000; // High frequency in Hz
int buzzerLowFreq = 600;   // Low frequency in Hz
int currentFreq = 0;       // Current frequency being generated

// Button debounce variables
bool lastButtonState = HIGH;  // Assuming pull-up resistor
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

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
          <div class="value" id="temp">--°C</div>
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
    let s=[0,0,0,0];  // Using numeric values (0/1) instead of booleans
    let temp = 0;
    let hum = 0;
    
    function u(){
      for(let i=0;i<3;i++){  // Only loop through 3 LEDs
        const btn=document.getElementById('l'+i);
        btn.classList[s[i]?'add':'remove']('on');
        btn.querySelector('.status').textContent=s[i]?'ON':'OFF';
      }
      
      // Format temperature and humidity with one decimal place
      const tempFormatted = (temp === null || isNaN(temp)) ? "--" : parseFloat(temp).toFixed(1);
      const humFormatted = (hum === null || isNaN(hum)) ? "--" : parseFloat(hum).toFixed(1);
      
      // Fix: Use clean degree symbol without Â character
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
  Serial.println(ledStates[2]);
}

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
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
    }
  }
  
  // Return numeric values in JSON
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "," + String(ledStates[3]) + "],"
              + "\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
  
  server.send(200, "application/json", json);
}

void handleStates() {
  // Return numeric values in JSON
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "," + String(ledStates[3]) + "],"
              + "\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
  
  server.send(200, "application/json", json);
}

void handleSensorData() {
  String json = "{\"temp\":" + String(temperature, 1) + ","
              + "\"hum\":" + String(humidity, 1) + "}";
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
    return;
  }
  
  // Reading temperature or humidity takes about 250 milliseconds
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  // Check if any reads failed and keep previous values
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Only update if we got valid readings
  humidity = newHumidity;
  temperature = newTemperature;
  
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.print("% Temperature: ");
  Serial.print(temperature, 1);
  Serial.println("°C");
}

// Set all buzzers to the same state
void setBuzzers(int state) {
  digitalWrite(BUZZER1_PIN, state);
  digitalWrite(BUZZER2_PIN, state);
  digitalWrite(BUZZER3_PIN, state);
  digitalWrite(BUZZER4_PIN, state);
}

void toggleBuzzer() {
  buzzerActive = !buzzerActive;
  
  if (buzzerActive) {
    Serial.println("Alarm activated");
    // Start with high tone
    buzzerState = 1;
    currentFreq = buzzerHighFreq;
  } else {
    Serial.println("Alarm deactivated");
    // Turn off all buzzers
    setBuzzers(LOW);
    buzzerState = 0;
    buzzerToneState = LOW;
  }
  
  buzzerLastChange = millis();
  buzzerToneChange = millis();
}

void updateBuzzer() {
  if (!buzzerActive) return;
  
  unsigned long currentMillis = millis();
  
  // Update buzzer tone pattern (high/low alternation)
  if (buzzerState == 1 && currentMillis - buzzerLastChange >= buzzerHighDuration) {
    // Switch to low tone
    buzzerState = 2;
    currentFreq = buzzerLowFreq;
    buzzerLastChange = currentMillis;
  }
  else if (buzzerState == 2 && currentMillis - buzzerLastChange >= buzzerLowDuration) {
    // Switch back to high tone
    buzzerState = 1;
    currentFreq = buzzerHighFreq;
    buzzerLastChange = currentMillis;
  }
  
  // Non-blocking tone generation
  if (currentFreq > 0) {
    // Calculate half period in microseconds
    int halfPeriod = 500000 / currentFreq; // in microseconds
    
    // Toggle all buzzer pins state at the appropriate frequency
    if (currentMillis - buzzerToneChange >= (halfPeriod / 1000)) {
      buzzerToneChange = currentMillis;
      buzzerToneState = !buzzerToneState;
      setBuzzers(buzzerToneState);
    }
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
        toggleBuzzer();
      }
    }
  }
  
  // Save the current reading for the next comparison
  lastButtonState = reading;
}

void setup() {
  Serial.begin(115200);
  
  // Test the LEDs to verify connections
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  
  // Quick LED test at startup
  Serial.println("Testing LEDs...");
  digitalWrite(LED1_PIN, HIGH);
  delay(300);
  digitalWrite(LED1_PIN, LOW);
  
  digitalWrite(LED2_PIN, HIGH);
  delay(300);
  digitalWrite(LED2_PIN, LOW);
  
  digitalWrite(LED3_PIN, HIGH);
  delay(300);
  digitalWrite(LED3_PIN, LOW);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  
  // Setup all buzzer pins
  pinMode(BUZZER1_PIN, OUTPUT);
  pinMode(BUZZER2_PIN, OUTPUT);
  pinMode(BUZZER3_PIN, OUTPUT);
  pinMode(BUZZER4_PIN, OUTPUT);
  
  // Ensure all buzzers are off
  setBuzzers(LOW);
  
  // Initialize all LEDs to off
  for (int i = 0; i < 4; i++) {
    ledStates[i] = 0;
  }
  setLEDs();
  
  // Initialize button state
  currentButtonState = digitalRead(BUTTON_PIN);
  lastButtonState = currentButtonState;
  
  // Initialize DHT sensor with retry mechanism
  Serial.println("Initializing DHT22 sensor...");
  initDHT();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/t", HTTP_POST, handleToggle);
  server.on("/s", HTTP_GET, handleStates);
  server.on("/sensor", HTTP_GET, handleSensorData);
  server.begin();
  
  // Initial DHT reading
  readDHTSensor();
  
  Serial.println("System ready!");
}

void loop() {
  server.handleClient();
  
  // Read DHT sensor at intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTReadTime >= dhtReadInterval) {
    lastDHTReadTime = currentMillis;
    readDHTSensor();
  }
  
  // Check button state
  checkButton();
  
  // Update buzzer sound pattern
  updateBuzzer();
} 