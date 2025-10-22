#ifndef HEARTBEAT_MQTT_PUBLISHER_H
#define HEARTBEAT_MQTT_PUBLISHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "MqttHandler.h"
#include "ConfigurationManager.h"
#include "version.h"

/**
 * @brief Publishes heartbeat messages to MQTT
 * 
 * Heartbeat includes device information, WiFi status, uptime, and system metrics
 */
class HeartbeatMqttPublisher {
public:
    /**
     * @brief Publish a heartbeat message
     */
    static void publishHeartbeat();
};

#endif // HEARTBEAT_MQTT_PUBLISHER_H
