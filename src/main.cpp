#include <Arduino.h>
#include <WiFi.h>
#include "ConfigurationManager.h"
#include "MqttHandler.h"
#include "WiFiManager.h"
#include "WebCamServer.h"
#include "HeartbeatMqttPublisher.h"
#include "version.h"

// Global objects - declare camera server first
WebCamServer camServer;

// Device identification for MQTT
const char *getDeviceAndApplication = "esp32-web-cam";
char mqttConfigureTopic[64];

// MQTT light control stubs (required by MqttHandler but not used for webcam)
int mqttLightDuration = 0;
String mqttLightState = "off";
bool mqttLightCommandReceived = false;

// Heartbeat interval (milliseconds)
const unsigned long heartbeatInterval = 60000; // 60 seconds
unsigned long lastHeartbeat = 0;

// LED pin for status indication
const int ledPin = 33;

void setup() {
    // Initialise serial communication
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n====================================");
    Serial.println("ESP32-CAM Web Streaming System");
    Serial.println("====================================");
    Serial.printf("Version: %s\n", getVersionStringWithBuild());
    
    // Print memory status
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Serial.println("====================================\n");
    
    // Initialise LED pin
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
    // ***Initialize camera BEFORE configuration to grab memory first***
    Serial.println("=== Camera Initialisation (Priority Init) ===");
    if (!camServer.initialiseCam()) {
        Serial.println("❌ Camera initialisation failed!");
        while (1) {
            digitalWrite(ledPin, HIGH);
            delay(500);
            digitalWrite(ledPin, LOW);
            delay(500);
        }
    }
    Serial.println("✅ Camera initialised");
    Serial.printf("Free heap after camera: %d bytes\n\n", ESP.getFreeHeap());
    
    // Setup configuration
    Serial.println("=== Configuration ===");
    ConfigurationManager::setup();
    ConfigurationManager::incrementBootCount();
    
    auto config = ConfigurationManager::getConfig();
    Serial.printf("Device: %s\n", config.uuid.c_str());
    Serial.printf("Boot: %d\n", config.bootCount);
    Serial.printf("Free heap: %d bytes\n\n", ESP.getFreeHeap());
    
    // Set MQTT buffer size SMALL for webcam
    MqttHandler::setBufferSize(512);  // Reduced from default 2048
    
    // WiFi
    Serial.println("=== WiFi ===");
    WiFiManager::setupLowPower(WiFiPowerMode::HIGH_PERFORMANCE);
    if (!WiFiManager::connectQuick(config.wifiTimeoutSeconds)) {
        Serial.println("❌ WiFi failed");
        while (1) { delay(1000); }
    }
    Serial.printf("✅ IP: %s\n", WiFiManager::getIPAddress().c_str());
    Serial.printf("Free heap: %d bytes\n\n", ESP.getFreeHeap());
    digitalWrite(ledPin, HIGH);
    
    // Web server
    Serial.println("=== Web Server ===");
    if (!camServer.startServer()) {
        Serial.println("❌ Server failed");
        while (1) { delay(1000); }
    }
    Serial.println("✅ Server started");
    Serial.printf("Free heap: %d bytes\n\n", ESP.getFreeHeap());
    
    // MQTT
    Serial.println("=== MQTT ===");
    String configTopic = "configure/" + config.uuid;
    configTopic.toCharArray(mqttConfigureTopic, sizeof(mqttConfigureTopic));
    
    MqttHandler::setup(0);
    if (MqttHandler::connect(config.mqttTimeoutSeconds)) {
        Serial.println("✅ MQTT connected");
        for (int i = 0; i < 5; i++) {
            MqttHandler::loop();
            delay(50);
        }
        HeartbeatMqttPublisher::publishHeartbeat();
    } else {
        Serial.println("⚠️ MQTT failed - will retry");
    }
    Serial.printf("Free heap: %d bytes\n\n", ESP.getFreeHeap());
    
    Serial.println("====================================");
    Serial.println("System Ready!");
    Serial.println("Stream: " + camServer.getStreamUrl());
    Serial.println("====================================\n");
    
    lastHeartbeat = millis();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        auto config = ConfigurationManager::getConfig();
        WiFiManager::connectQuick(config.wifiTimeoutSeconds);
    }
    
    if (!MqttHandler::isConnected()) {
        auto config = ConfigurationManager::getConfig();
        MqttHandler::connect(config.mqttTimeoutSeconds);
    } else {
        MqttHandler::loop();
    }
    
    unsigned long currentMillis = millis();
    if (currentMillis - lastHeartbeat >= heartbeatInterval) {
        if (MqttHandler::isConnected()) {
            HeartbeatMqttPublisher::publishHeartbeat();
        }
        lastHeartbeat = currentMillis;
    }
    
    delay(100);
}
