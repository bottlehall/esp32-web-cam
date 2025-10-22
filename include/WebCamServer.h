#ifndef WEB_CAM_SERVER_H
#define WEB_CAM_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <esp_http_server.h>

/**
 * @brief Manages the ESP32-CAM web server and streaming functionality
 * 
 * This class handles camera initialisation, web server setup,
 * and video streaming over HTTP
 */
class WebCamServer {
public:
    /**
     * @brief Construct a new Web Cam Server object
     */
    WebCamServer();
    
    /**
     * @brief Destroy the Web Cam Server object
     */
    ~WebCamServer();
    
    /**
     * @brief Initialise the camera with default settings
     * 
     * @return true if camera initialisation was successful
     * @return false if camera initialisation failed
     */
    bool initialiseCam();
    
    /**
     * @brief Start the web server for streaming
     * 
     * @return true if server started successfully
     * @return false if server failed to start
     */
    bool startServer();
    
    /**
     * @brief Stop the web server
     */
    void stopServer();
    
    /**
     * @brief Check if the server is running
     * 
     * @return true if server is running
     * @return false if server is stopped
     */
    bool isRunning();
    
    /**
     * @brief Get the stream URL
     * 
     * @return String containing the full stream URL
     */
    String getStreamUrl();

private:
    httpd_handle_t streamHttpd;
    bool serverRunning;
    
    /**
     * @brief Configure camera pins for AI-Thinker ESP32-CAM
     */
    void configureCameraPins();
    
    /**
     * @brief HTTP handler for the stream endpoint
     */
    static esp_err_t streamHandler(httpd_req_t *req);
    
    /**
     * @brief HTTP handler for the index page
     */
    static esp_err_t indexHandler(httpd_req_t *req);
};

#endif // WEB_CAM_SERVER_H
