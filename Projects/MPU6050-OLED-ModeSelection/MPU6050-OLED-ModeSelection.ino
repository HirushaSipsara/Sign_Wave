#include <ESP8266WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_I2C_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST_PIN D3 

#define SWITCH1_PIN D4  // Switch 1 for mode selection
#define SWITCH2_PIN D5  // Switch 2 for mode selection

const char* ssid = "Hirusha";         // Replace with your WiFi SSID
const char* password = "home3815";    // Replace with your WiFi password

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);
Adafruit_MPU6050 mpu;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int currentMode = 0;  // Variable to store the selected mode (0: No mode, 1: Mode 1, 2: Mode 2)

void setup() {
  Serial.begin(115200);
  Wire.begin();

  resetDisplay();

  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select Mode:");
  display.println("1: Tilt Directions");
  display.println("2: Sign Gestures");
  display.display();  // Show mode selection message

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  // Mode selection based on switch input
  if (digitalRead(SWITCH1_PIN) == LOW) {
    currentMode = 1;  // Set to Mode 1
    delay(300);       // Debouncing delay
  }
  if (digitalRead(SWITCH2_PIN) == LOW) {
    currentMode = 2;  // Set to Mode 2
    delay(300);       // Debouncing delay
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  String dataToSend = "";

  if (currentMode == 1) {
    // Mode 1: Tilt Directions
    display.setTextSize(2);
    display.println("Mode 1");
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Determine tilt direction and prepare data to send
    if (a.acceleration.x > 5) {
      display.println("Tilt \nRight");
      dataToSend = "Tilt Right";
    } else if (a.acceleration.x < -5) {
      display.println("Tilt \nLeft");
      dataToSend = "Tilt Left";
    } else if (a.acceleration.y > 5) {
      display.println("Tilt \nForward");
      dataToSend = "Tilt Forward";
    } else if (a.acceleration.y < -5) {
      display.println("Tilt \nBackward");
      dataToSend = "Tilt Backward";
    } else if (a.acceleration.z > 9) {
      display.println("Flat");
      dataToSend = "Flat";
    } else if (a.acceleration.z < -9) {
      display.println("Upside \nDown");
      dataToSend = "Upside Down";
    } else {
      display.println("Neutral");
      dataToSend = "Neutral";
    }

  } else if (currentMode == 2) {
    // Mode 2: One Sign Gestures
    display.setTextSize(2);
    display.println("Mode 2");
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);  // Get sensor data

    // Get accelerometer and gyroscope readings
    float accelX = a.acceleration.x;
    float accelY = a.acceleration.y;
    float accelZ = a.acceleration.z;
    float gyroX = g.gyro.x;
    float gyroY = g.gyro.y;
    float gyroZ = g.gyro.z;

    // Identify specific gestures based on the sensor data
    if (detectMistakeGesture(accelY, gyroZ)) {
      display.println("Mistake Gesture");
      dataToSend = "Mistake";
    } else if (detectNameGesture(accelX, accelY, gyroZ)) {
      display.println("Name Gesture");
      dataToSend = "Name";
    } else if (detectPleaseGesture(gyroZ)) {
      display.println("Please Gesture");
      dataToSend = "Please";
    }

    delay(100);
  } else {
    // Default screen if no mode is selected
    display.setTextSize(1);
    display.println("Select Mode:");
    display.println("1: Tilt Directions");
    display.println("2: Sign Gestures");
  }

  display.display();

  if (dataToSend != "") {
    // Send the data to the webserver via WebSocket
    notifyClients(dataToSend);
  }

  delay(200);  // Short delay to avoid rapid updates
}

//Methords

void resetDisplay() {
  pinMode(OLED_RST_PIN, OUTPUT);
  digitalWrite(OLED_RST_PIN, LOW);
  delay(10);
  digitalWrite(OLED_RST_PIN, HIGH);
}

void notifyClients(String message) {
  ws.textAll(message);
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

bool detectMistakeGesture(float accelY, float gyroZ) {
  // Downward motion (Mistake sign)
  return (accelY < -5 && abs(gyroZ) < 0.1);
}

bool detectNameGesture(float accelX, float accelY, float gyroZ) {
  // Quick tapping motion (Name sign)
  return (abs(accelX) > 4 && abs(accelY) > 4 && abs(gyroZ) < 0.1);
}

bool detectPleaseGesture(float gyroZ) {
  // Circular motion over chest (Please sign)
  return (abs(gyroZ) > 1.5);  // Continuous rotation detected
}
