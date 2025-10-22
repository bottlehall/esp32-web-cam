#include "WebCamConfiguration.h"

WebCamConfigurationSettings WebCamConfiguration::config;
Preferences WebCamConfiguration::preferences;

void WebCamConfiguration::displayConfiguration(WebCamConfigurationSettings config, bool showPassword) {
    Serial.println("\n=== Web Camera Configuration ===");
    Serial.println("WiFi SSID:         " + config.wifiSSID);
    if (showPassword) {
        Serial.println("WiFi Password:     " + config.wifiPassword);
    }
    Serial.println("==============================\n");
}

String WebCamConfiguration::getInput(String prompt) {
    Serial.print(prompt + ": ");
    String input = "";
    while (true) {
        if (Serial.available()) {
            char characterTyped = Serial.read();
            Serial.print(characterTyped);
            input += characterTyped;
            if (characterTyped == '\n') {
                break;
            }
        }
        delay(10);
    }
    input.trim();
    return input;
}

bool WebCamConfiguration::initNVS() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("Erasing NVS partition and reinitialising...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return (err == ESP_OK);
}

bool WebCamConfiguration::loadConfiguration() {
    if (!initNVS()) {
        Serial.println("Failed to initialise NVS");
        return false;
    }
    
    bool success = preferences.begin(PREFERENCE_NAMESPACE, false);
    if (!success) {
        Serial.println("Failed to open preferences namespace");
        return false;
    }
    
    bool confirmed = preferences.getBool(CONFIRMATION_KEY, false);
    
    if (confirmed) {
        config.wifiSSID = preferences.getString(WIFI_SSID_KEY, "");
        config.wifiPassword = preferences.getString(WIFI_PASS_KEY, "");
        config.isConfirmed = true;
        
        preferences.end();
        return true;
    }
    
    preferences.end();
    return false;
}

void WebCamConfiguration::requestConfiguration() {
    WebCamConfigurationSettings newConfig;
    
    Serial.println("\n=== Web Camera Configuration Input ===");
    
    newConfig.wifiSSID = getInput("WiFi SSID");
    newConfig.wifiPassword = getInput("WiFi Password");
    
    Serial.println("\nConfiguration summary:");
    displayConfiguration(newConfig, true);
    
    String confirmation = getInput("\nType '" + String(EXPECTED_CONFIRMATION) + "' to save these settings");
    
    if (confirmation == EXPECTED_CONFIRMATION) {
        newConfig.isConfirmed = true;
        if (saveConfiguration(newConfig)) {
            Serial.println("Configuration saved successfully!");
        } else {
            Serial.println("Failed to save configuration!");
        }
    } else {
        Serial.println("Configuration not saved.");
        delay(2000);
        ESP.restart();
    }
}

bool WebCamConfiguration::saveConfiguration(WebCamConfigurationSettings newConfig) {
    if (!initNVS()) {
        Serial.println("Failed to initialise NVS");
        return false;
    }
    
    bool success = preferences.begin(PREFERENCE_NAMESPACE, false);
    if (!success) {
        Serial.println("Failed to open preferences namespace for writing");
        return false;
    }
    
    preferences.putString(WIFI_SSID_KEY, newConfig.wifiSSID);
    preferences.putString(WIFI_PASS_KEY, newConfig.wifiPassword);
    preferences.putBool(CONFIRMATION_KEY, true);
    
    preferences.end();
    
    config = newConfig;
    return true;
}

bool WebCamConfiguration::shouldEnterSetupMode() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    bool buttonPressed = (digitalRead(BOOT_BUTTON_PIN) == LOW);
    
    // Enter setup if button pressed or config not loaded
    return buttonPressed || !loadConfiguration();
}

void WebCamConfiguration::setup() {
    Serial.println("\nESP32 Web Camera Configuration Manager");
    
    if (shouldEnterSetupMode()) {
        requestConfiguration();
    } else {
        displayConfiguration(config);
    }
}
