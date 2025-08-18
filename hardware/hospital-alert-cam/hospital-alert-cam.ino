#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

// Replace with your network credentials
const char* ssid = "Manamike";
const char* password = "manamike";

// Web server on port 80
WebServer server(80);

// Camera configuration - Multiple board support
// AI-THINKER ESP32-CAM (most common)
#define CAMERA_MODEL_AI_THINKER

#ifdef CAMERA_MODEL_AI_THINKER
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
#endif

// Streaming state
bool emergencyMode = false;
bool serverRunning = false;

void setup() {
  Serial.begin(115200);
  delay(2000); // Give time for serial monitor to connect
  Serial.println("ESP32-CAM Emergency Stream Starting...");

  // Add some delay for power stabilization
  delay(1000);

  // Camera configuration
  camera_config_t config;
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Try different grab modes for compatibility
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  // Frame size and quality settings - Start with lower resolution
  config.frame_size = FRAMESIZE_VGA;  // Start smaller
  config.jpeg_quality = 15;           // Lower quality initially
  config.fb_count = 1;                // Single frame buffer
  
  // Additional settings for problematic modules
  config.fb_location = CAMERA_FB_IN_PSRAM;
  
  Serial.println("Attempting camera initialization...");
  Serial.printf("XCLK: %d, SIOD: %d, SIOC: %d\n", XCLK_GPIO_NUM, SIOD_GPIO_NUM, SIOC_GPIO_NUM);
  
  // Try to detect camera sensor
  Serial.println("Checking camera sensor...");
  
  // First try with power down disabled
  if (PWDN_GPIO_NUM != -1) {
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);  // Enable camera
    delay(100);
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    Serial.println("Restarting in 5 seconds...");
    delay(5000);
    ESP.restart();
    return;
  }
  Serial.println("Camera initialized successfully!");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Don't start server initially - only start in emergency mode
  Serial.println("Camera ready. Server will start only in emergency mode.");
  Serial.println("Ready for commands...");
}

void loop() {
  // Handle web server requests only if server is running
  if (serverRunning) {
    server.handleClient();
  }
  
  // Check for serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "emergency") {
      if (!emergencyMode) {
        emergencyMode = true;
        startServer();
        Serial.println("Emergency mode activated - server started and streaming available");
      }
    }
    else if (command == "all_good") {
      if (emergencyMode) {
        emergencyMode = false;
        stopServer();
        Serial.println("Emergency mode deactivated - server stopped completely");
      }
    }
  }
  
  delay(100);
}

void startServer() {
  if (!serverRunning) {
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/stream", handleStream);
    server.on("/status", handleStatus);
    
    server.begin();
    serverRunning = true;
    Serial.print("HTTP server started at: http://");
    Serial.println(WiFi.localIP());
  }
}

void stopServer() {
  if (serverRunning) {
    server.stop();
    server.close();
    serverRunning = false;
    Serial.println("HTTP server stopped completely");
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32-CAM Emergency Stream</title></head>";
  html += "<body style='font-family: Arial, sans-serif; text-align: center; padding: 50px;'>";
  html += "<h1>ESP32-CAM Emergency Stream</h1>";
  
  if (emergencyMode) {
    html += "<h2 style='color: red;'>🚨 EMERGENCY MODE ACTIVE 🚨</h2>";
    html += "<img src='/stream' style='max-width: 800px; border: 3px solid red;'>";
  } else {
    html += "<h2 style='color: green;'>✅ All Good - No Stream</h2>";
    html += "<p>Waiting for emergency signal...</p>";
  }
  
  html += "<br><br>";
  html += "<button onclick='location.reload()' style='padding: 10px 20px; font-size: 16px;'>Refresh</button>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleStream() {
  if (!emergencyMode) {
    server.send(503, "text/plain", "Stream not available - not in emergency mode");
    return;
  }

  WiFiClient client = server.client();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected() && emergencyMode) {
    // Check for serial commands even while streaming
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      
      if (command == "all_good") {
        emergencyMode = false;
        Serial.println("Emergency mode deactivated during streaming - stopping stream");
        break; // Exit streaming loop immediately
      }
      else if (command == "emergency") {
        // Already in emergency mode, just acknowledge
        Serial.println("Already in emergency mode");
      }
    }
    
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }

    String header = "--frame\r\n";
    header += "Content-Type: image/jpeg\r\n";
    header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    server.sendContent(header);
    
    size_t sent = 0;
    uint8_t* buf = fb->buf;
    size_t len = fb->len;
    
    while (sent < len && client.connected()) {
      size_t will_send = (len - sent > 1024) ? 1024 : len - sent;
      size_t actual_sent = client.write(buf + sent, will_send);
      if (actual_sent == 0) {
        break;
      }
      sent += actual_sent;
    }
    
    server.sendContent("\r\n");
    esp_camera_fb_return(fb);
    
    // Small delay to prevent overwhelming the connection
    delay(50); // Reduced delay for more responsive serial checking
  }
  
  // Clean up when exiting stream
  if (!emergencyMode) {
    stopServer();
  }
}

void handleStatus() {
  String json = "{";
  json += "\"emergency_mode\": " + String(emergencyMode ? "true" : "false") + ",";
  json += "\"server_running\": " + String(serverRunning ? "true" : "false") + ",";
  json += "\"streaming\": " + String(emergencyMode ? "true" : "false") + ",";
  json += "\"ip_address\": \"" + WiFi.localIP().toString() + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}