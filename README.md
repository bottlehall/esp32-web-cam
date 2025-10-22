# ESP32 Web Camera with MQTT Integration

A professional ESP32-CAM streaming solution with full MQTT integration, OTA updates, and heartbeat monitoring using British English spelling throughout.

## Features

- 🎥 Live MJPEG video streaming over HTTP
- 📱 Web-based viewer with responsive design
- 💾 Persistent configuration using ConfigurationManager
- 📡 Full MQTT integration with heartbeat messages
- 🔄 OTA firmware updates via MQTT
- ⚙️ Remote configuration via MQTT
- 🔧 Serial configuration interface
- 🔄 Automatic WiFi and MQTT reconnection
- 🇬🇧 British English spelling throughout codebase

## Hardware Requirements

- ESP32-CAM module (AI-Thinker or compatible)
- USB-to-Serial adapter (FTDI or CH340)
- USB cable
- MQTT broker (e.g., Mosquitto, HiveMQ)

## Pin Configuration

The project is configured for the AI-Thinker ESP32-CAM board:

- Camera pins are automatically configured in `WebCamServer.cpp`
- LED indicator: GPIO 33
- Boot button: GPIO 0 (for configuration mode)
- PSRAM support enabled for higher quality streaming

## Dependencies

This project uses the following libraries from your existing codebase:

- `esp32-configuration-manager` - Persistent configuration with NVS
- `esp32-mqtt-handler` - MQTT with TLS, OTA updates, and power management
- `esp32-wifi-manager` - WiFi connection management
- `ArduinoJson` - JSON serialisation for MQTT messages

## Installation

1. Open the project in PlatformIO:
   ```bash
   cd esp32-web-cam
   pio run
   ```

2. Connect your ESP32-CAM via USB-to-Serial adapter:
   - ESP32-CAM GND → Adapter GND
   - ESP32-CAM 5V → Adapter 5V  
   - ESP32-CAM U0R → Adapter TX
   - ESP32-CAM U0T → Adapter RX
   - Connect GPIO 0 to GND for programming mode

3. Upload the firmware:
   ```bash
   pio run --target upload
   ```

4. Disconnect GPIO 0 from GND and reset the board

5. Monitor the serial output:
   ```bash
   pio device monitor
   ```

## Initial Configuration

On first boot, the device will prompt for complete configuration via Serial Monitor (115200 baud):

1. **Device UUID** - Unique identifier for this camera
2. **WiFi SSID** - Your WiFi network name
3. **WiFi Password** - Your WiFi password
4. **MQTT Server** - MQTT broker hostname/IP
5. **MQTT Port** - Usually 1883 (unencrypted) or 8883 (TLS)
6. **MQTT Username** - MQTT authentication username
7. **MQTT Password** - MQTT authentication password
8. **MQTT Client Name** - Friendly name for this camera
9. **Power Settings** - WiFi/MQTT timeouts (defaults are fine)
10. Type `CONFIRM` to save the configuration

The device will restart and connect to WiFi and MQTT.

## Usage

### Web Streaming

Once configured and connected:

1. The Serial Monitor will display the stream URL (e.g., `http://192.168.1.100/`)
2. Open the URL in a web browser
3. The live video stream will be displayed

### MQTT Integration

The camera publishes to the following MQTT topics:

#### Heartbeat (every 60 seconds)
Topic: `heartbeat`

```json
{
  "id": "camera-01",
  "name": "Front Door Camera",
  "type": "webcam",
  "version": "1.0.0+1",
  "boot_count": 5,
  "uptime": 3600,
  "free_heap": 180000,
  "psram_found": true,
  "rssi": -45,
  "ip_address": "192.168.1.100",
  "camera_active": true,
  "stream_url": "http://192.168.1.100/",
  "deep_sleep_enabled": false,
  "always_on": true
}
```

#### Configuration Updates
Topic: `configure/[device-uuid]`

The device subscribes to this topic for:
- OTA firmware updates
- Configuration changes
- Power management settings

### OTA Updates

To perform an OTA update via MQTT:

```json
{
  "action": "update",
  "url": "http://your-server.com/firmware.bin"
}
```

Publish this to the `configure/[device-uuid]` topic.

### Remote Configuration

You can update settings remotely via MQTT. Publish to `configure/[device-uuid]`:

```json
{
  "action": "update_setting",
  "setting": "sleep_interval",
  "value": 30
}
```

Available settings match those in ConfigurationManager.

## Reconfiguration

To reconfigure the device:

1. Hold the GPIO 0 button (boot button)
2. Press the reset button
3. Release the reset button
4. Release the GPIO 0 button
5. Follow the configuration prompts in Serial Monitor

## Web Interface

The device hosts a simple web interface:

- **Index Page** (`/`) - HTML viewer with embedded video player
- **Stream Endpoint** (`/stream`) - Raw MJPEG stream

## Camera Settings

Default configuration:
- Frame size: VGA (640x480) for smooth streaming
- JPEG quality: 10 (0-63, lower is better)
- Frame rate: ~15-30 FPS (depending on network and lighting)

Settings can be modified in `WebCamServer.cpp` in the `initialiseCam()` method.

## Project Structure

```
esp32-web-cam/
├── include/
│   ├── HeartbeatMqttPublisher.h   # MQTT heartbeat publishing
│   ├── WebCamServer.h             # Camera and HTTP server
│   └── version.h                  # Version information
├── src/
│   ├── HeartbeatMqttPublisher.cpp
│   ├── WebCamServer.cpp
│   └── main.cpp                   # Main application logic
├── platformio.ini                 # PlatformIO configuration
└── README.md
```

## Code Style

This project follows British English spelling conventions:

- `initialise` instead of `initialize`
- `colour` instead of `color`
- `centre` instead of `center`

Variable names, comments, and all text use British English spelling for consistency with the existing codebase.

## Heartbeat Monitoring

The camera sends heartbeat messages every 60 seconds containing:

- Device identification (UUID, name, type)
- Version information
- Boot count
- Uptime
- System metrics (free heap, PSRAM status)
- WiFi status (RSSI, IP address)
- Camera status and stream URL
- Power configuration

You can monitor these in your MQTT broker or Home Assistant.

## Troubleshooting

### Camera Initialisation Failed
- Check that the camera module is properly connected
- Ensure camera ribbon cable is inserted correctly (blue side up on AI-Thinker)
- Try power cycling the device

### WiFi Connection Failed
- Verify credentials in configuration
- Check WiFi signal strength
- Ensure using 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Hold GPIO 0 button during boot to reconfigure

### MQTT Connection Failed
- Verify MQTT broker is running and accessible
- Check MQTT credentials
- Verify MQTT port (1883 for unencrypted, 8883 for TLS)
- Check firewall settings

### Low Frame Rate
- Check network bandwidth
- Reduce JPEG quality or frame size in settings
- Ensure good lighting conditions

### PSRAM Not Found
- Some ESP32-CAM boards don't have PSRAM
- The code will automatically adjust settings for non-PSRAM boards
- Streaming quality may be reduced but will still work

## Advanced Configuration

### Changing Camera Resolution

Edit `WebCamServer.cpp`, line where `set_framesize` is called:

```cpp
s->set_framesize(s, FRAMESIZE_SVGA);  // Options: QVGA, VGA, SVGA, XGA, UXGA
```

### Adjusting JPEG Quality

Edit `WebCamServer.cpp`:

```cpp
s->set_quality(s, 12);  // 0-63, lower = higher quality, larger file size
```

### Adjusting Heartbeat Interval

Edit `main.cpp`:

```cpp
const unsigned long heartbeatInterval = 60000; // milliseconds (60 seconds)
```

### Custom Web Interface

Modify the HTML in `WebCamServer.cpp` `indexHandler()` method to customise the web interface.

## Integration Examples

### Home Assistant

Add to your `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "Front Door Camera Status"
      state_topic: "heartbeat"
      value_template: "{{ value_json.name }}"
      json_attributes_topic: "heartbeat"
      json_attributes_template: "{{ value_json | tojson }}"
    
    - name: "Front Door Camera RSSI"
      state_topic: "heartbeat"
      value_template: "{{ value_json.rssi }}"
      unit_of_measurement: "dBm"
      device_class: "signal_strength"
    
    - name: "Front Door Camera Uptime"
      state_topic: "heartbeat"
      value_template: "{{ value_json.uptime }}"
      unit_of_measurement: "s"

camera:
  - platform: generic
    name: Front Door Camera
    still_image_url: http://192.168.1.100/stream
    stream_source: http://192.168.1.100/stream
```

### Node-RED

Use MQTT nodes to subscribe to the heartbeat topic and display camera status.

## Security Notes

⚠️ Security considerations:

- WiFi credentials stored in NVS (not encrypted)
- HTTP streaming (not HTTPS)
- MQTT can use TLS if configured
- No authentication on web interface

For production use, consider:
- HTTPS support for web streaming
- Basic authentication on web interface
- VPN or network isolation
- Encrypted MQTT (TLS)

## Power Consumption

The ESP32-CAM runs continuously for live streaming:

- Active streaming: ~160-300mA @ 5V
- Idle with WiFi: ~80-100mA @ 5V
- Not suitable for battery operation (always-on design)

## Licence

This project is designed for educational and personal use.

## Credits

Built with:
- ESP32 Arduino Core
- ESP-IDF Camera Driver
- PlatformIO
- Your existing ESP32 libraries (ConfigurationManager, MQTTHandler, WiFiManager)

British English spelling maintained throughout for consistency with existing codebase.

## Version History

### 1.0.0
- Initial release
- Full MQTT integration
- Heartbeat monitoring
- OTA updates support
- Web streaming interface
# esp32-web-cam
