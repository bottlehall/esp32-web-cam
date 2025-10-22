#ifndef WEB_CAM_CONFIG_H
#define WEB_CAM_CONFIG_H

#include <Arduino.h>
#include <nvs_flash.h>
#include <Preferences.h>

/**
 * @brief Configuration settings for the web camera
 */
struct WebCamConfigurationSettings {
    String wifiSSID;
    String wifiPassword;
    bool isConfirmed;
};

/**
 * @brief Manages web camera configuration storage
 * 
 * This class handles storing and retrieving configuration values
 * such as WiFi credentials using the ESP32's NVS (Non-Volatile Storage)
 */
class WebCamConfiguration {
private:
    static constexpr const char *PREFERENCE_NAMESPACE = "webcam";
    static constexpr const char *CONFIRMATION_KEY = "confirmed";
    static constexpr const char *EXPECTED_CONFIRMATION = "CONFIRM";
    static constexpr const char *WIFI_SSID_KEY = "wifi_ssid";
    static constexpr const char *WIFI_PASS_KEY = "wifi_pass";
    static constexpr const int BOOT_BUTTON_PIN = 0;  // GPIO 0 for ESP32-CAM boot button
    
    static WebCamConfigurationSettings config;
    static Preferences preferences;
    
    /**
     * @brief Get input from Serial
     * @param prompt The prompt to display
     * @return String containing the user input
     */
    static String getInput(String prompt);
    
    /**
     * @brief Initialise the NVS system
     * @return true if initialisation successful
     */
    static bool initNVS();
    
    /**
     * @brief Display current configuration
     * @param config Configuration to display
     * @param showPassword Whether to show the password
     */
    static void displayConfiguration(WebCamConfigurationSettings config, bool showPassword = false);
    
public:
    /**
     * @brief Get current configuration
     * @return WebCamConfigurationSettings Current configuration
     */
    static WebCamConfigurationSettings getConfig() { return config; }
    
    /**
     * @brief Load configuration from NVS
     * @return true if configuration loaded successfully
     */
    static bool loadConfiguration();
    
    /**
     * @brief Request configuration via Serial
     */
    static void requestConfiguration();
    
    /**
     * @brief Save configuration to NVS
     * @param newConfig Configuration to save
     * @return true if save successful
     */
    static bool saveConfiguration(WebCamConfigurationSettings newConfig);
    
    /**
     * @brief Main setup method
     */
    static void setup();
    
    /**
     * @brief Check if setup mode should be entered
     * @return true if boot button pressed or config invalid
     */
    static bool shouldEnterSetupMode();
};

#endif // WEB_CAM_CONFIG_H
