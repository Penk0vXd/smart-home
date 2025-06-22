// Smart Home Dashboard Configuration
// Modify these settings to match your MQTT broker and device setup

const DASHBOARD_CONFIG = {
    // MQTT Broker Settings
    mqtt: {
        host: '7fad8ef106584fd6951e4a14e11fc5ea.s1.eu.hivemq.cloud',
        port: 8884,                 // WebSocket port for HiveMQ Cloud (not 8883)
        protocol: 'wss',            // Secure WebSocket
        username: 'PenkovXD_19',
        password: 'FYK_hxf2dct.emu@afc',
        clientIdPrefix: 'dashboard_',
        reconnectPeriod: 5000,      // Reconnect every 5 seconds
        connectTimeout: 10000,      // 10 second connection timeout
        secure: true,
    },

    // MQTT Topics Configuration
    topics: {
        sensors: {
            temperature: 'home/sensor/temperature',
            humidity: 'home/sensor/humidity',
            rain: 'home/sensors/rain',
            system: 'home/system/status'
        },
        controls: {
            lights: 'home/lights/',           // Will be appended with light number (1,2,3)
            lightsStatus: 'home/lights/',     // Will be appended with light number + '/status'
            buzzer: 'home/buzzer/command',
            buzzerStatus: 'home/buzzer/status'
        },
        // Subscribe to all sensor topics with wildcard
        subscriptions: [
            'home/sensor/temperature',
            'home/sensor/humidity', 
            'home/sensors/rain',
            'home/system/status',
            'home/lights/+/status',        // + is wildcard for single level
            'home/buzzer/status'
        ]
    },

    // UI Configuration
    ui: {
        // Light names (customize these)
        lightNames: [
            'Living Room (GPIO12)',
            'Kitchen (GPIO13)', 
            'Bedroom (GPIO14)',
            'Bathroom (GPIO27)'
        ],
        
        // Update intervals (in milliseconds)
        timestampUpdateInterval: 1000,     // Update "Last Update" every second
        
        // Notification timeouts
        errorTimeout: 5000,               // Hide error messages after 5 seconds
        successTimeout: 3000,             // Hide success messages after 3 seconds
        
        // Theme preference (saved to localStorage)
        defaultTheme: 'light',            // 'light' or 'dark'
    },

    // Device-specific settings
    devices: {
        // Rain sensor threshold (adjust based on your sensor)
        rainThreshold: 500,
        
        // Number of lights to control
        lightCount: 4,
        
        // WiFi signal strength thresholds
        wifiStrong: -50,                  // dBm - above this is "strong"
        wifiWeak: -70,                    // dBm - below this is "weak"
        
        // System status refresh settings
        systemStatusRefresh: 30000,       // 30 seconds
    }
};

// Export for use in main dashboard
if (typeof module !== 'undefined' && module.exports) {
    module.exports = DASHBOARD_CONFIG;
} 