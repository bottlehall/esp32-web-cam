#include "HeartbeatMqttPublisher.h"

void HeartbeatMqttPublisher::publishHeartbeat() {
    Serial.println("Publishing heartbeat...");
    
    JsonDocument heartbeatDoc;
    
    auto config = ConfigurationManager::getConfig();
    
    // Device identification
    heartbeatDoc["id"] = config.uuid;
    heartbeatDoc["name"] = config.mqttClientName;
    heartbeatDoc["type"] = "webcam";
    heartbeatDoc["version"] = getVersionStringWithBuild();
    heartbeatDoc["boot_count"] = config.bootCount;
    
    // System metrics
    heartbeatDoc["uptime"] = millis() / 1000;
    heartbeatDoc["free_heap"] = ESP.getFreeHeap();
    heartbeatDoc["psram_found"] = psramFound();
    
    // WiFi status
    heartbeatDoc["rssi"] = WiFi.RSSI();
    heartbeatDoc["ip_address"] = WiFi.localIP().toString();
    
    // Camera status
    heartbeatDoc["camera_active"] = true;
    heartbeatDoc["stream_url"] = "http://" + WiFi.localIP().toString() + "/";
    
    // Power settings (webcam doesn't use deep sleep)
    heartbeatDoc["deep_sleep_enabled"] = false;
    heartbeatDoc["always_on"] = true;
    
    String heartbeatJson;
    serializeJson(heartbeatDoc, heartbeatJson);
    
    MqttHandler::publish("heartbeat", heartbeatJson);
    
    Serial.println("Heartbeat published");
    Serial.println(heartbeatJson);
}
