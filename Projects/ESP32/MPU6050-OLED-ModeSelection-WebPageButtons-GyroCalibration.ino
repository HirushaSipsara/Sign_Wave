#include <WiFi.h>  // Use <WiFi.h> for ESP32
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>  // Use <AsyncTCP.h> for ESP32
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST_PIN D3
#define SCREEN_I2C_ADDR 0x3C

const char* ssid = "Hirusha";         // Replace with your WiFi SSID
const char* password = "home3815";    // Replace with your WiFi password

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int currentMode = 1;  // Default mode (1: Tilt, 2: Gestures)
// Offset variables(Updated Code>=25-28)
float gyroXOffset = 0;
float gyroYOffset = 0;
float gyroZOffset = 0;

void notifyClients(String message);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// Gesture detection functions
bool detectMistakeGesture(float accelY, float gyroZ) {
  return (accelY < -5 && abs(gyroZ) < 0.1);
}

bool detectNameGesture(float accelX, float accelY, float gyroZ) {
  return (abs(accelX) > 4 && abs(accelY) > 4 && abs(gyroZ) < 0.1);
}

bool detectPleaseGesture(float gyroZ) {
  return (abs(gyroZ) > 1.5);
}

// Define the gesture detection function(Updated Code>=48-52)
bool NameGesture(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return ((calibratedGyroX > -10 && calibratedGyroX < 10) && 
      (calibratedGyroY > -5 && calibratedGyroY < 5) && 
      (calibratedGyroZ > -10));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // MPU6050 setup
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1);
  }
  Serial.println("MPU6050 Found!");

  // Calibrate the sensor(Updated Code>=73-74)
  calibrateSensor();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // WebSocket setup
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Webpage handler
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", webpage());
  });

  // Start server
  server.begin();
}

// Calibrate the sensor Methord(Updated Code>=99-121)
void calibrateSensor() {
  const int numReadings = 100; // Number of readings for calibration
  float gyroXSum = 0, gyroYSum = 0, gyroZSum = 0;

  for (int i = 0; i < numReadings; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    gyroXSum += g.gyro.x;
    gyroYSum += g.gyro.y;
    gyroZSum += g.gyro.z;

    delay(10); // Short delay between readings
  }

  gyroXOffset = gyroXSum / numReadings;
  gyroYOffset = gyroYSum / numReadings;
  gyroZOffset = gyroZSum / numReadings;

  Serial.print("Calibrated Gyro X Offset: "); Serial.println(gyroXOffset);
  Serial.print("Calibrated Gyro Y Offset: "); Serial.println(gyroYOffset);
  Serial.print("Calibrated Gyro Z Offset: "); Serial.println(gyroZOffset);
}

void loop() {
  yield();
  String dataToSend = "";
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print accelerometer data
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.print("] X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" m/s^2");
  Serial.println(ESP.getFreeHeap());



  if (currentMode == 1) {
    // Mode 1: Tilt Directions
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println("Mode 1");

    if (a.acceleration.x > 5) {
      dataToSend = "Tilt Right";
      display.println("Tilt Right");
    } else if (a.acceleration.x < -5) {
      dataToSend = "Tilt Left";
      display.println("Tilt Left");
    } else if (a.acceleration.y > 5) {
      dataToSend = "Tilt Forward";
      display.println("Tilt Forward");
    } else if (a.acceleration.y < -5) {
      dataToSend = "Tilt Backward";
      display.println("Tilt Backward");
    } else if (a.acceleration.z > 9) {
      dataToSend = "Flat";
      display.println("Flat");
    } else if (a.acceleration.z < -9) {
      dataToSend = "Upside Down";
      display.println("Upside Down");
    } else {
      dataToSend = "Neutral";
      display.println("Neutral");
    }
    display.display();

    if (dataToSend != "") {
      notifyClients(dataToSend);
    }

  } else if (currentMode == 2) {
    // Mode 2: One Sign Gestures
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println("Mode 2");

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float accelX = a.acceleration.x;
    float accelY = a.acceleration.y;
    float gyroZ = g.gyro.z;

    //Validation(Updated Code>=203-210, 221-223)
    float calibratedGyroX = g.gyro.x - gyroXOffset;
    float calibratedGyroY = g.gyro.y - gyroYOffset;
    float calibratedGyroZ = g.gyro.z - gyroZOffset;

    Serial.print(" | Calibrated Gyro X: "); Serial.print(calibratedGyroX);
    Serial.print(" | Calibrated Gyro Y: "); Serial.print(calibratedGyroY);
    Serial.print(" | Calibrated Gyro Z: "); Serial.println(calibratedGyroZ);

    if (detectMistakeGesture(accelY, gyroZ)) {
      display.println("Mistake Gesture");
      dataToSend = "Mistake";
    } else if (detectNameGesture(accelX, accelY, g.gyro.x)) {
      display.println("Name Gesture");
      dataToSend = "Name";
    } else if (detectPleaseGesture(gyroZ)) {
      display.println("Please Gesture");
      dataToSend = "Please";
    } else if (NameGesture(calibratedGyroX, calibratedGyroY, calibratedGyroZ)) {
      display.println("Hi!!");
      dataToSend = "Hi";
    } else {
      display.println("Null");
    }
    display.display();
    if (dataToSend != "") {
      notifyClients(dataToSend);
    }

  }

  delay(100);
}

String webpage() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP Mode Selection</title>
      <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
      <style>
        body {
          font-family: 'Courier Prime', monospace;
          text-align: center;
          padding: 50px;
          background-color: #f8f9fa;
        }
      </style>
    </head>
    <body>
      <h1>ESP8266 Mode Selection</h1>
      <p>Select Mode:</p>
      <button class="btn btn-primary" onclick="selectMode(1)">Tilt Directions</button>
      <button class="btn btn-secondary" onclick="selectMode(2)">Sign Gestures</button>
      
      <div id="current-mode"></div>

      <script>
        const ws = new WebSocket('ws://' + window.location.hostname + '/ws');

        ws.onmessage = (event) => {
          document.getElementById('current-mode').innerHTML = event.data;
        };

        function selectMode(mode) {
          ws.send(mode.toString());
        }
      </script>
    </body>
    </html>
  )rawliteral";
}

void notifyClients(String message) {
  ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    currentMode = message.toInt();  // Update mode based on web selection
    Serial.println("Mode changed to: " + String(currentMode));
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

