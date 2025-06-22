# Smart Home IoT Dashboard

A modern, responsive web dashboard for IoT smart home systems with real-time MQTT integration. Built with vanilla HTML, CSS, and JavaScript featuring Apple-inspired design language.

## ✨ Features

### 🎨 **Modern UI/UX**
- Clean, minimalist Apple-style design
- Smooth animations and transitions
- Light/Dark mode toggle
- Responsive layout (mobile, tablet, desktop)
- Real-time data visualization

### 🔌 **MQTT Integration**
- WebSocket-based MQTT client using `mqtt.js`
- Auto-reconnection with connection status indicators
- Support for both secured (WSS) and unsecured (WS) connections
- Retained message support for device states

### 📊 **Real-time Monitoring**
- **Temperature & Humidity** (DHT22 sensor)
- **Rain Detection** with threshold-based status
- **System Status** (WiFi signal, memory usage, uptime)
- **Device Status** indicators with live updates

### 🎛️ **Device Control**
- **Smart Lighting** - Toggle 4 lights independently (Living Room, Kitchen, Bedroom, Bathroom)
- **Security Alarm** - Control buzzer/alarm system
- **Emergency Stop** - Turn off all devices instantly
- Instant feedback with optimistic UI updates

## 🚀 Quick Start

### 1. Download Files
```bash
# Clone or download these files:
- index.html
- dashboard-config.js
- README.md
```

### 2. Configure MQTT Broker
Edit `dashboard-config.js` to match your setup:

```javascript
const DASHBOARD_CONFIG = {
    mqtt: {
        host: 'your-broker.com',          // Your MQTT broker
        port: 8000,                       // WebSocket port
        protocol: 'ws',                   // 'ws' or 'wss'
        // username: 'your-username',     // Optional
        // password: 'your-password',     // Optional
    }
};
```

### 3. Open in Browser
Simply open `index.html` in any modern web browser. No web server required!

## 🔧 Configuration Options

### MQTT Broker Examples

#### Local Mosquitto (Unsecured)
```javascript
mqtt: {
    host: 'localhost',        // or your Pi's IP: '192.168.1.100'
    port: 8000,
    protocol: 'ws'
}
```

#### HiveMQ Cloud (Secured)
```javascript
mqtt: {
    host: 'your-instance.s1.eu.hivemq.cloud',
    port: 8884,
    protocol: 'wss',
    username: 'your-username',
    password: 'your-password'
}
```

#### Eclipse IoT (Public)
```javascript
mqtt: {
    host: 'mqtt.eclipseprojects.io',
    port: 80,
    protocol: 'ws'
}
```

### Custom Device Names
```javascript
ui: {
    lightNames: [
        'Living Room (GPIO12)',
        'Kitchen (GPIO13)', 
        'Bedroom (GPIO14)',
        'Bathroom (GPIO27)'
    ]
}
```

### MQTT Topics
The dashboard expects these default topics (configurable):

```javascript
topics: {
    sensors: {
        temperature: 'home/sensor/temperature',    // Float value in °C
        humidity: 'home/sensor/humidity',          // Float value in %
        rain: 'home/sensors/rain',                 // JSON: {"raining": bool, "sensor_value": int}
        system: 'home/system/status'               // JSON with system info
    },
    controls: {
        lights: 'home/lights/',                    // Sends "ON"/"OFF" to home/lights/1,2,3
        buzzer: 'home/buzzer/command'              // Sends "ON"/"OFF"
    }
}
```

## 📱 Responsive Design

### Desktop (1200px+)
- 4-column grid layout
- Large sensor cards
- Side-by-side controls

### Tablet (768px - 1199px)
- 2-column grid layout
- Medium-sized cards
- Touch-friendly controls

### Mobile (< 768px)
- Single column layout
- Stacked navigation
- Optimized for thumb navigation

## 🎨 Theming

### Light Mode (Default)
- White backgrounds
- Apple-style shadows
- Blue accent colors (#007AFF)

### Dark Mode
- Dark backgrounds (#000000, #1C1C1E)
- Reduced contrast
- Same accent colors

Toggle between themes with the 🌙/☀️ button. Preference saved to localStorage.

## 🔌 Compatible Devices

### Tested With:
- **ESP32** with Arduino IDE
- **ESP8266** with Arduino IDE  
- **Raspberry Pi** with Python/Node.js
- **Arduino** with WiFi modules

### Required Libraries (for Arduino/ESP32):
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
```

## 📊 MQTT Message Formats

### Sensor Data (Published by devices)
```javascript
// Temperature (home/sensor/temperature)
"23.5"

// Humidity (home/sensor/humidity) 
"65.2"

// Rain Sensor (home/sensors/rain)
{
    "raining": false,
    "sensor_value": 723,
    "timestamp": 1640995200
}

// System Status (home/system/status)
{
    "wifi_rssi": -45,
    "free_heap": 234567,
    "heap_usage_percent": 15.2,
    "mqtt_connected": true,
    "uptime_ms": 3600000
}
```

### Control Commands (Published by dashboard)
```javascript
// Light Control (home/lights/1, home/lights/2, home/lights/3, home/lights/4)
"ON" or "OFF"

// Buzzer Control (home/buzzer/command)
"ON" or "OFF"
```

## 🛠️ Development

### Project Structure
```
smart-home-dashboard/
├── index.html              # Main dashboard file
├── dashboard-config.js     # Configuration file
├── README.md              # This file
└── assets/               # Optional: custom icons/images
```

### Key Technologies
- **HTML5** - Semantic structure
- **CSS3** - Custom properties, Grid, Flexbox
- **JavaScript ES6+** - Modern async/await, modules
- **MQTT.js** - WebSocket MQTT client library

### Browser Compatibility
- Chrome/Edge 80+
- Firefox 75+
- Safari 13+
- Mobile browsers (iOS Safari, Chrome Mobile)

## 🔒 Security Notes

### For Production Use:
1. **Use WSS (WebSocket Secure)** connections
2. **Enable MQTT authentication** (username/password)
3. **Use proper SSL certificates** for your broker
4. **Implement MQTT ACLs** to restrict topic access
5. **Consider VPN** for remote access

### Development Setup:
- Local brokers (Mosquitto) can use unsecured WS
- Public brokers should always use authentication

## 🐛 Troubleshooting

### Common Issues:

#### "MQTT Connection Failed"
```javascript
// Check browser console for detailed error
// Verify broker settings in dashboard-config.js
// Ensure broker allows WebSocket connections
// Check firewall/network settings
```

#### "WebSocket Connection Blocked"
```javascript
// HTTPS pages can only connect to WSS (secure) brokers
// HTTP pages can connect to both WS and WSS
// Use matching protocols: HTTP+WS or HTTPS+WSS
```

#### "No Data Received"
```javascript
// Verify your device is publishing to correct topics
// Check topic names match configuration
// Ensure device is connected to same broker
// Check broker logs for message activity
```

### Debug Mode
Open browser Developer Tools (F12) and check Console tab for detailed logs.

## 🚀 Advanced Features

### Custom Animations
Add your own CSS animations:
```css
.custom-pulse {
    animation: customPulse 2s infinite;
}

@keyframes customPulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
}
```

### Additional Sensors
Extend the dashboard by adding new sensor cards:
```html
<div class="card">
    <div class="card-header">
        <h3 class="card-title">Air Quality</h3>
        <div class="card-icon">🌬️</div>
    </div>
    <div class="sensor-value" id="air-quality">--</div>
    <div class="sensor-unit">AQI</div>
</div>
```

### Custom Controls
Add new control types:
```html
<div class="control-item">
    <span class="control-label">Fan Speed</span>
    <input type="range" min="0" max="100" value="50" 
           onchange="setFanSpeed(this.value)">
</div>
```

## 📄 License

MIT License - feel free to modify and use in your projects!

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## 📞 Support

For issues and questions:
- Check the troubleshooting section
- Open an issue on GitHub
- Review browser console for error details

---

**Built with ❤️ for the IoT community** 