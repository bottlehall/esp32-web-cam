#include "WebCamServer.h"

// Camera pin definitions for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebCamServer::WebCamServer() : streamHttpd(nullptr), serverRunning(false) {
}

WebCamServer::~WebCamServer() {
    stopServer();
}

void WebCamServer::configureCameraPins() {
    // This method is kept for potential future customisation
    // Pin configuration is done in initialiseCam()
}

bool WebCamServer::initialiseCam() {
    // Power up sequence for camera
    Serial.println("Powering up camera module...");
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);  // Power ON
    delay(100);
    
    // Check if camera is already initialized and deinit if needed
    esp_camera_deinit();
    delay(100);
    
    camera_config_t config;
    memset(&config, 0, sizeof(camera_config_t));
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 10000000;  // Reduced from 20MHz for stability
    config.pixel_format = PIXFORMAT_JPEG;
    
    config.frame_size = FRAMESIZE_QVGA;  // Start with smallest size: 320x240
    config.jpeg_quality = 15;  // Lower quality for testing
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    Serial.println("Initializing camera with minimal settings...");
    Serial.println("Frame size: QVGA (320x240)");
    Serial.println("XCLK: 10MHz");
    Serial.println("Frame buffers: 1 (DRAM)");
    
    // Camera initialisation with error checking
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        
        if (err == ESP_ERR_NOT_FOUND) {
            Serial.println("Camera sensor not found");
        } else if (err == ESP_ERR_INVALID_ARG) {
            Serial.println("Invalid camera configuration");
        } else if (err == ESP_ERR_NO_MEM) {
            Serial.println("Out of memory");
        }
        
        return false;
    }
    
    Serial.println("Camera hardware initialized successfully");
    delay(300);
    
    // Test frame capture
    Serial.println("Testing frame capture...");
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture test frame");
        return false;
    }
    Serial.printf("Test frame captured: %u bytes\n", fb->len);
    esp_camera_fb_return(fb);
    
    // Get sensor
    sensor_t * s = esp_camera_sensor_get();
    if (s == nullptr) {
        Serial.println("Failed to get camera sensor");
        return false;
    }
    
    Serial.println("Sensor acquired, applying basic settings...");
    
    // Minimal sensor config
    s->set_framesize(s, FRAMESIZE_VGA);  // Upgrade to VGA now that it's working
    s->set_quality(s, 12);
    
    Serial.println("Camera fully initialized and ready");
    return true;
}

esp_err_t WebCamServer::streamHandler(httpd_req_t *req) {
    camera_fb_t * fb = nullptr;
    esp_err_t res = ESP_OK;
    size_t jpg_buf_len = 0;
    uint8_t * jpg_buf = nullptr;
    char part_buf[64];
    
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    if (res != ESP_OK) {
        return res;
    }
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        
        if (fb->format != PIXFORMAT_JPEG) {
            Serial.println("Non-JPEG format not supported");
            esp_camera_fb_return(fb);
            res = ESP_FAIL;
            break;
        }
        
        jpg_buf_len = fb->len;
        jpg_buf = fb->buf;
        
        if (res == ESP_OK) {
            size_t hlen = snprintf(part_buf, 64,
                "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)jpg_buf, jpg_buf_len);
        }
        
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 13);
        }
        
        esp_camera_fb_return(fb);
        
        if (res != ESP_OK) {
            break;
        }
    }
    
    return res;
}

esp_err_t WebCamServer::indexHandler(httpd_req_t *req) {
    const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM Stream</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        h1 {
            color: #333;
        }
        #stream {
            max-width: 100%;
            border: 3px solid #333;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32-CAM Live Stream</h1>
        <img id="stream" src="/stream">
    </div>
</body>
</html>
)rawliteral";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

bool WebCamServer::startServer() {
    if (serverRunning) {
        Serial.println("Server is already running");
        return true;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;
    
    if (httpd_start(&streamHttpd, &config) != ESP_OK) {
        Serial.println("Failed to start HTTP server");
        return false;
    }
    
    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = indexHandler,
        .user_ctx  = nullptr
    };
    httpd_register_uri_handler(streamHttpd, &index_uri);
    
    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = streamHandler,
        .user_ctx  = nullptr
    };
    httpd_register_uri_handler(streamHttpd, &stream_uri);
    
    serverRunning = true;
    Serial.println("HTTP server started successfully");
    Serial.println("Stream available at: " + getStreamUrl());
    
    return true;
}

void WebCamServer::stopServer() {
    if (streamHttpd != nullptr) {
        httpd_stop(streamHttpd);
        streamHttpd = nullptr;
        serverRunning = false;
        Serial.println("HTTP server stopped");
    }
}

bool WebCamServer::isRunning() {
    return serverRunning;
}

String WebCamServer::getStreamUrl() {
    if (!serverRunning) {
        return "Server not running";
    }
    return "http://" + WiFi.localIP().toString() + "/";
}
