#include <WiFi.h>
#include <WebServer.h>

// LED Pins
#define LED1_PIN 16
#define LED2_PIN 17
#define LED3_PIN 18

// WiFi credentials
const char* ssid = "Vivacom_FiberNet-30DD";
const char* password = "TA66u7aCXC";

bool ledStates[4] = {false, false, false};
WebServer server(80);

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head>
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
    let s=[0,0,0,0];
    function u(){
      for(let i=0;i<4;i++){
        const btn=document.getElementById('l'+i);
        btn.classList[s[i]?'add':'remove']('on');
        btn.querySelector('.status').textContent=s[i]?'ON':'OFF';
      }
    }
    function t(i){
      fetch('/t?l='+i,{method:'POST'})
        .then(r=>r.json())
        .then(d=>{s=d.s;u()})
        .catch(e=>console.error(e));
    }
    fetch('/s')
      .then(r=>r.json())
      .then(d=>{s=d.s;u()})
      .catch(e=>console.error(e));
  </script>
</body>
</html>
)rawliteral";

void setLEDs() {
  digitalWrite(LED1_PIN, ledStates[0]);
  digitalWrite(LED2_PIN, ledStates[1]);
  digitalWrite(LED3_PIN, ledStates[2]);
}

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleToggle() {
  if (server.hasArg("l")) {
    int idx = server.arg("l").toInt();
    if (idx >= 0 && idx <= 3) {
      ledStates[idx] = !ledStates[idx];
      setLEDs();
    }
  }
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "," + String(ledStates[3]) + "]}";
  server.send(200, "application/json", json);
}

void handleStates() {
  String json = "{\"s\":[" + String(ledStates[0]) + "," + String(ledStates[1]) + "," 
              + String(ledStates[2]) + "," + String(ledStates[3]) + "]}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  setLEDs();
  
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
  server.begin();
}

void loop() {
  server.handleClient();
} 