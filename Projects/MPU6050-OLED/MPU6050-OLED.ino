#include <ESP8266WiFi.h>  // Use <ESP8266WiFi.h> for ESP8266
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Use <ESPAsyncTCP.h> for ESP8266

#define SCREEN_I2C_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST_PIN D3 

const char* ssid = "Hirusha";         // Replace with your WiFi SSID
const char* password = "home3815"; // Replace with your WiFi password

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);
Adafruit_MPU6050 mpu;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void resetDisplay() {
  // Manual reset using the reset pin
  pinMode(OLED_RST_PIN, OUTPUT);
  digitalWrite(OLED_RST_PIN, LOW);   // Pull the reset pin low
  delay(10);                         // Wait for 10ms
  digitalWrite(OLED_RST_PIN, HIGH);  // Pull the reset pin high
}

void notifyClients(String direction) {
  ws.textAll(direction);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char *)data;
    Serial.println(message);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  resetDisplay();
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Stay in loop if display fails to initialize
  }

  // Clear the OLED display to ensure it starts fresh
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Display Initialized");
  display.display();  // Show "Display Initialized" message

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Initialize MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Setup WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Start the server
  server.begin();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  String direction = getTiltDirection(a);

  // Clear the OLED screen and display MPU6050 data
  display.clearDisplay();  // Clear the previous content
  display.setCursor(0, 0);
  display.setTextSize(2);

  // Display tilt directions based on accelerometer data
  if (a.acceleration.x > 5) {
    display.println("Tilt \nRight");
  } else if (a.acceleration.x < -5) {
    display.println("Tilt \nLeft");
  } else if (a.acceleration.y > 5) {
    display.println("Tilt \nForward");
  } else if (a.acceleration.y < -5) {
    display.println("Tilt \nBackward");
  } else if (a.acceleration.z > 9) {
    display.println("Flat");
  } else if (a.acceleration.z < -9) {
    display.println("Upside \nDown");
  } else {
    display.println("Neutral");
  }

  display.display();

  notifyClients(direction);

  delay(2000);  // Notify clients every 2 seconds
}

String getTiltDirection(sensors_event_t a) {
  if (a.acceleration.x > 5) return "Tilt Right";
  if (a.acceleration.x < -5) return "Tilt Left";
  if (a.acceleration.y > 5) return "Tilt Forward";
  if (a.acceleration.y < -5) return "Tilt Backward";
  if (a.acceleration.z > 9) return "Flat";
  if (a.acceleration.z < -9) return "Upside Down";
  return "Neutral";
}
