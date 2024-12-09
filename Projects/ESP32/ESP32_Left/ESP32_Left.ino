#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST_PIN 16 // Use GPIO 16 for the OLED reset pin (change if needed)
#define SCREEN_I2C_ADDR 0x3C
#define threshold 4095

const char *ssid = "Hirusha";      // Replace with your WiFi SSID
const char *password = "home3815"; // Replace with your WiFi password

// Define the analog input pins for the flex sensors
const int flexSensor2Pin = 36; // GPIO36 (Analog Input for Flex Sensor 1)
const int flexSensor3Pin = 39; // GPIO39 (Analog Input for Flex Sensor 2)
const int flexSensor4Pin = 34; // GPIO34 (Analog Input for Flex Sensor 3)
const int flexSensor5Pin = 35; // GPIO35 (Analog Input for Flex Sensor 4)
const int flexSensor1Pin = 32; // GPIO32 (Analog Input for Flex Sensor 5)

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variables to store the sensor readings
int flex1Value = 0;
int flex2Value = 0;
int flex3Value = 0;
int flex4Value = 0;
int flex5Value = 0;
int fingerState1 = 0;
int fingerState2 = 0;
int fingerState3 = 0;
int fingerState4 = 0;
int fingerState5 = 0;

int currentMode = 0; // Default mode (0: off, 3: on)
// Offset variables
float gyroXOffset = 0;
float gyroYOffset = 0;
float gyroZOffset = 0;
float accelXOffset = 0;
float accelYOffset = 0;
float accelZOffset = 0;

void notifyClients(String message);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// Modified bool function to use both accelerometer and gyroscope data
bool detectBye(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
               float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{
    // Gesture detection logic
    return ((calibratedGyroX > 0 && calibratedGyroX < 10) &&
            (calibratedGyroY > 0 && calibratedGyroY < 10) &&
            (calibratedGyroZ > 0 || calibratedGyroZ < 0) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 0.5) &&  // Condition for accelerometer X-axis
            (calibratedAccelY > 8.0 && calibratedAccelY < 10.0) && // Condition for accelerometer Y-axis
            fingerState2 == 1 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState1 == 0);
}
bool detectHello(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                 float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                 int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 10) &&
            (calibratedGyroY > 0 && calibratedGyroY < 5) &&
            (calibratedGyroZ > 0 || calibratedGyroZ < 0) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 0.5) &&
            (calibratedAccelY > 8.0 && calibratedAccelY < 10.0) &&
            fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectIam(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
               float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 10) &&
            (calibratedGyroY > 0 && calibratedGyroY < 5) &&
            (calibratedGyroZ > 0) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 5.5) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) &&
            (calibratedAccelZ > 10.0 && calibratedAccelZ < 30.0) &&
            fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectPlease(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                  float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                  int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 5) &&
            (calibratedGyroY > 0 && calibratedGyroY < 2) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 10) &&
            (calibratedAccelX > 8.0 && calibratedAccelX < 15.5) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) &&
            (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
            fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectYes(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
               float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 2.0) &&
            (calibratedAccelZ > 0.0 && calibratedAccelZ < 5.0) &&
            fingerState3 == 1 && fingerState4 == 1 && fingerState2 == 1 && fingerState1 == 1 && fingerState5 == 1);
}
bool detectNo(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
              int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) &&
            (calibratedAccelY > 8.0 && calibratedAccelY < 12.0) &&
            (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
            fingerState1 == 0 && fingerState5 == 1 && fingerState3 == 0 && fingerState2 == 0 && fingerState4 == 1);
}
bool detectSorry(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                 float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                 int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 5) &&
            (calibratedGyroY > 0 && calibratedGyroY < 2) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 10) &&
            (calibratedAccelX > 8.0 && calibratedAccelX < 15.5) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) &&
            (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
            fingerState1 == 1 && fingerState5 == 1 && fingerState3 == 1 && fingerState2 == 1 && fingerState4 == 1);
}
bool detectWater(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                 float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                 int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) &&
            (calibratedAccelY > 8.0 && calibratedAccelY < 12.0) &&
            (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
            fingerState1 == 1 && fingerState5 == 1 && fingerState4 == 0 && fingerState3 == 0 && fingerState2 == 0);
}
bool detectThirsty(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                   float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                   int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 10.0) &&
            (calibratedAccelY > 6.0 && calibratedAccelY < 10.0) &&
            (calibratedAccelZ > 0.0 && calibratedAccelZ < 6.0) &&
            fingerState1 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState3 == 0 && fingerState2 == 0);
}
bool detectI(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
             float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
             int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 5.0 && calibratedAccelX < 11.0) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) &&
            (calibratedAccelZ > 5.0 && calibratedAccelZ < 10.0) &&
            fingerState1 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState3 == 1 && fingerState2 == 0);
}
bool detectNeed(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 1) &&
            (calibratedGyroY > 0 && calibratedGyroY < 1) &&
            (calibratedGyroZ > 0 && calibratedGyroZ < 1) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 2.0) &&
            (calibratedAccelZ > 0.0 && calibratedAccelZ < 5.0) &&
            fingerState3 == 1 && fingerState4 == 1 && fingerState1 == 1 && fingerState5 == 0 && fingerState2 == 0);
}
bool detectThankyou(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ,
                    float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
                    int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5)
{

    return ((calibratedGyroX > 0 && calibratedGyroX < 10) &&
            (calibratedGyroY > 0 && calibratedGyroY < 5) &&
            (calibratedGyroZ > 0) &&
            (calibratedAccelX > 0.0 && calibratedAccelX < 5.0) &&
            (calibratedAccelY > 0.0 && calibratedAccelY < 10.0) &&
            (calibratedAccelZ > 8.0 && calibratedAccelZ < 30.0) &&
            fingerState3 == 0 && fingerState4 == 0 && fingerState1 == 0 && fingerState5 == 0 && fingerState2 == 0);
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // OLED setup
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.display();

    // MPU6050 setup
    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
            ;
    }
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 initialized successfully");
    Serial.println("MPU6050 Found!");

    // Calibrate the sensor
    calibrateSensor();

    // Configure ADC resolution for flex sensors
    analogReadResolution(12);       // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // Full 3.3V range

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // WebSocket setup
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Webpage handler
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", webpage()); });

    // Start server
    server.begin();
}

// Calibrate the sensor method
void calibrateSensor()
{
    const int numReadings = 100; // Number of readings for calibration
    float gyroXSum = 0, gyroYSum = 0, gyroZSum = 0;
    float accelXSum = 0, accelYSum = 0, accelZSum = 0;

    for (int i = 0; i < numReadings; i++)
    {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        gyroXSum += g.gyro.x;
        gyroYSum += g.gyro.y;
        gyroZSum += g.gyro.z;

        accelXSum += a.acceleration.x;
        accelYSum += a.acceleration.y;
        accelZSum += a.acceleration.z;

        delay(10); // Short delay between readings
    }

    gyroXOffset = gyroXSum / numReadings;
    gyroYOffset = gyroYSum / numReadings;
    gyroZOffset = gyroZSum / numReadings;
    accelXOffset = accelXSum / numReadings;
    accelYOffset = accelYSum / numReadings;
    accelZOffset = accelZSum / numReadings;

    Serial.print("Calibrated Gyro X Offset: ");
    Serial.println(gyroXOffset);
    Serial.print("Calibrated Gyro Y Offset: ");
    Serial.println(gyroYOffset);
    Serial.print("Calibrated Gyro Z Offset: ");
    Serial.println(gyroZOffset);
}

void loop()
{
    yield();
    String dataToSend = "";
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if(currentMode == 0)
    {
        Serial.print("ESP32 Off\n");
    }
    else if (currentMode == 1)
    {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.println("Mode 1");

        // Read flex sensor values
        flex1Value = analogRead(flexSensor1Pin);
        flex2Value = analogRead(flexSensor2Pin);
        flex3Value = analogRead(flexSensor3Pin);
        flex4Value = analogRead(flexSensor4Pin);
        flex5Value = analogRead(flexSensor5Pin);

          Serial.print("Finger 1: "); Serial.print(flex1Value);
          Serial.print(" | Finger 2: "); Serial.print(flex2Value);
          Serial.print(" | Finger 3: "); Serial.print(flex3Value);
          Serial.print(" | Finger 4: "); Serial.print(flex4Value);
          Serial.print(" | Finger 5: "); Serial.println(flex5Value);

        // Determine finger states based on thresholds
        fingerState1 = (flex1Value > 4096) ? 1 : 0;
        fingerState2 = (flex2Value > 2000) ? 1 : 0;
        fingerState3 = (flex3Value > 3500) ? 1 : 0;
        fingerState4 = (flex4Value > 2000) ? 1 : 0;
        fingerState5 = (flex5Value > 1500) ? 1 : 0;

        if (a.acceleration.x > 5 && fingerState1 == 0 && fingerState2 == 1 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "Bad";
          display.println("Bad");
        } else if (a.acceleration.x > -5 && fingerState1 == 0 && fingerState2 == 1 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "Good";
          display.println("Good");
        } else if (a.acceleration.y > 5 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "I do not understand";
          display.println("I do not understand");
        } else if (a.acceleration.y > 5 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0) {
          dataToSend = "Where is the bathroom";
          display.println("Where is the bathroom");
        } else if (a.acceleration.y < -5 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "Nice to meet you";
          display.println("Nice to meet you");
        } else if (a.acceleration.y < -5 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "What time is it";
          display.println("What time is it");
        } else if (a.acceleration.z > 9 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "How much does this cost";
          display.println("How much does this cost");
        } else if (a.acceleration.z < -9 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "Can you help me";
          display.println("Can you help me");
        } else if (a.acceleration.z < -9 && fingerState1 == 0 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "What is your name";
          display.println("What is your name");
        }else if (a.acceleration.z > 9 && fingerState1 == 1 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "I am hungry";
          display.println("I am hungry");
        }else if (a.acceleration.z < -9 && fingerState1 == 1 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "I am thirsty";
          display.println("I am thirsty");
        }else if (a.acceleration.y < -5 && fingerState1 == 1 && fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1) {
          dataToSend = "I am tired";
          display.println("I am tired");
        }else if(fingerState1 == 1 && fingerState2 == 1 && fingerState3 == 0 && fingerState4 == 1 && fingerState5 == 1){
          dataToSend = "Fuck you";
          display.println("Fuck you");
        }
        display.display();

        if (dataToSend != "") {
          notifyClients(dataToSend);
        }
    }
    else if (currentMode == 2)
    {
        // Mode 2: One Sign Gestures
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.println("Mode 2");

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Calibrated gyro and accelerometer values with absolute value adjustment
        float calibratedGyroX = abs(g.gyro.x - gyroXOffset);
        float calibratedGyroY = abs(g.gyro.y - gyroYOffset);
        float calibratedGyroZ = abs(g.gyro.z - gyroZOffset);
        float calibratedAccelX = abs(a.acceleration.x - accelXOffset);
        float calibratedAccelY = abs(a.acceleration.y - accelYOffset);
        float calibratedAccelZ = abs(a.acceleration.z - accelZOffset);
        float temperature = temp.temperature; // Temperature from MPU6050

        // Read flex sensor values
        flex1Value = analogRead(flexSensor1Pin);
        flex2Value = analogRead(flexSensor2Pin);
        flex3Value = analogRead(flexSensor3Pin);
        flex4Value = analogRead(flexSensor4Pin);
        flex5Value = analogRead(flexSensor5Pin);

        // Determine finger states based on thresholds
        fingerState1 = (flex1Value > 4096) ? 1 : 0;
        fingerState2 = (flex2Value > 2000) ? 1 : 0;
        fingerState3 = (flex3Value > 3500) ? 1 : 0;
        fingerState4 = (flex4Value > 2000) ? 1 : 0;
        fingerState5 = (flex5Value > 1500) ? 1 : 0;

        if (detectBye(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                      fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
            {
                display.println("Good Bye !!!");
                dataToSend = "Good Bye";
            }
        else if (detectHello(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                             fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Hello !!!");
            dataToSend = "Hello";
        }
        else if (detectIam(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                           fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("I am");
            dataToSend = "I am";
        }
        else if (detectPlease(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                              fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Please..");
            dataToSend = "Please..";
        }
        else if (detectYes(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                           fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Yes");
            dataToSend = "Yes";
        }
        else if (detectNo(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                          fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("No");
            dataToSend = "No";
        }
        else if (detectSorry(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                             fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Sorry..");
            dataToSend = "Sorry";
        }
        else if (detectWater(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                             fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Water");
            dataToSend = "Water";
        }
        else if (detectThirsty(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                               fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Thirsty");
            dataToSend = "Thirsty";
        }
        else if (detectI(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                         fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("I");
            dataToSend = "I";
        }
        else if (detectNeed(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                            fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Need");
            dataToSend = "Need";
        }
        else if (detectThankyou(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5))
        {
            display.println("Thank you !!");
            dataToSend = "Thank you !!";
        }else if(flex1Value == 1 && flex2Value == 1 && flex3Value == 0 && flex4Value == 1 && flex5Value == 1){
            display.println("Fuck you");
            dataToSend = "Fuck you";
        }
        else
        {
            display.println("");
        }
        display.display();
        if (dataToSend != "")
        {
            notifyClients(dataToSend);
        }
    }

    delay(100);
}

String webpage()
{
    return R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 Mode Selection</title>
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
      <h1>ESP32 Mode Selection</h1>
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

void notifyClients(String message)
{
    ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        String message = (char *)data;
        currentMode = message.toInt(); // Update mode based on web selection
        Serial.println("Mode changed to: " + String(currentMode));
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.println("WebSocket client connected");
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("WebSocket client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {
        handleWebSocketMessage(arg, data, len);
    }
}
