#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST_PIN 16 // Use GPIO 16 for the OLED reset pin
#define SCREEN_I2C_ADDR 0x3C
#define thrshold 3300

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST_PIN);
Adafruit_MPU6050 mpu;

// Flex sensor pins
const int flexPin1 = 36;
const int flexPin2 = 39;
const int flexPin3 = 34;
const int flexPin4 = 35;
const int flexPin5 = 32;

// Variables to store flex sensor values
int flexValue1 = 0, flexValue2 = 0, flexValue3 = 0, flexValue4 = 0, flexValue5 = 0;
int fingerState1 = 0, fingerState2 = 0, fingerState3 = 0, fingerState4 = 0, fingerState5 = 0;

// Threshold values for finger states


// Offset variables for calibration
float gyroXOffset = 0, gyroYOffset = 0, gyroZOffset = 0;
float accelXOffset = 0, accelYOffset = 0, accelZOffset = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }

  // MPU6050 initialization
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 initialized successfully");

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Calibrate MPU6050
  calibrateSensor();

  // Configure ADC resolution for flex sensors
  analogReadResolution(12);  // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db);  // Full 3.3V range
}

void calibrateSensor() {
  const int numReadings = 100;
  float gyroXSum = 0, gyroYSum = 0, gyroZSum = 0;
  float accelXSum = 0, accelYSum = 0, accelZSum = 0;

  for (int i = 0; i < numReadings; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Sum gyroscope and accelerometer values
    gyroXSum += g.gyro.x;
    gyroYSum += g.gyro.y;
    gyroZSum += g.gyro.z;

    accelXSum += a.acceleration.x;
    accelYSum += a.acceleration.y;
    accelZSum += a.acceleration.z;

    delay(10);
  }

  // Calculate average offsets
  gyroXOffset = gyroXSum / numReadings;
  gyroYOffset = gyroYSum / numReadings;
  gyroZOffset = gyroZSum / numReadings;
  accelXOffset = accelXSum / numReadings;
  accelYOffset = accelYSum / numReadings;
  accelZOffset = accelZSum / numReadings;
}

// Modified bool function to use both accelerometer and gyroscope data
bool detectBye(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
               float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {
  // Gesture detection logic
  return ((calibratedGyroX > 0 && calibratedGyroX < 1.5) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1.5) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1.0) &&
          (calibratedAccelX > 0.0 && calibratedAccelX < 5.0) && // Condition for accelerometer X-axis
          (calibratedAccelY > 8.0 && calibratedAccelY < 13.0) && // Condition for accelerometer Y-axis
          fingerState2 == 1 && fingerState3 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState1 == 0);
}
bool detectHello(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5, float accelX, float accelY, float accelZ) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 1.5) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1.5) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1.5)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 4.0) && 
          (calibratedAccelY > 9.0 && calibratedAccelY < 11.5) && 
          (accelY < -5 && accelZ > 0) &&
          fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectIam(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 10) && 
          (calibratedGyroY > 0 && calibratedGyroY < 5) && 
          (calibratedGyroZ > 0 )&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 5.5) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) && 
          (calibratedAccelZ > 10.0 && calibratedAccelZ < 30.0) &&
          fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectPlease(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0.2 && calibratedGyroX < 5) && 
          (calibratedGyroY > 0.5 && calibratedGyroY < 3) && 
          (calibratedGyroZ > 1.0 && calibratedGyroZ < 5)&&
          (calibratedAccelX > 5.0 && calibratedAccelX < 15.5) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) && 
          (calibratedAccelZ > 6.0 && calibratedAccelZ < 15.0) &&
          fingerState2 == 0 && fingerState3 == 0 && fingerState4 == 0 && fingerState5 == 0 && fingerState1 == 0);
}
bool detectYes(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 5) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 5.0) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 13.0) && 
          (calibratedAccelZ > 0.0 && calibratedAccelZ < 13.0) &&
          fingerState3 == 1 && fingerState4 == 1 && fingerState2 == 1 && fingerState1 == 1 && fingerState5 == 1);
}
bool detectNo(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0.5 && calibratedGyroX < 2) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 5) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 13.0) && 
          (calibratedAccelZ > 0.0 && calibratedAccelZ < 13.0) &&
          fingerState1 == 0 && fingerState5 == 1 && fingerState3 == 0 && fingerState2 == 0 && fingerState4 == 1);
}
bool detectSorry(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 5) && 
          (calibratedGyroY > 0 && calibratedGyroY < 2) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 10)&&
          (calibratedAccelX > 8.0 && calibratedAccelX < 15.5) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) && 
          (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
          fingerState1 == 1 && fingerState5 == 1 && fingerState3 == 1 && fingerState2 == 1 && fingerState4 == 1);
}
bool detectWater(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 1) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) && 
          (calibratedAccelY > 8.0 && calibratedAccelY < 12.0) && 
          (calibratedAccelZ > 8.0 && calibratedAccelZ < 15.0) &&
          fingerState1 == 1 && fingerState5 == 1 && fingerState4 == 0 && fingerState3 == 0 && fingerState2 == 0);
}
bool detectI(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 1) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 5.0 && calibratedAccelX < 11.0) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) && 
          (calibratedAccelZ > 10.0 && calibratedAccelZ < 15.0) &&
          fingerState1 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState3 == 1 && fingerState2 == 0);
}
bool detectThirsty(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 1) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 5.0 && calibratedAccelX < 11.0) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 5.0) && 
          (calibratedAccelZ > 5.0 && calibratedAccelZ < 10.0) &&
          fingerState1 == 1 && fingerState4 == 1 && fingerState5 == 1 && fingerState3 == 1 && fingerState2 == 0);
}
bool detectNeed(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 1) && 
          (calibratedGyroY > 0 && calibratedGyroY < 1) && 
          (calibratedGyroZ > 0 && calibratedGyroZ < 1)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 2.5) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 2.0) && 
          (calibratedAccelZ > 0.0 && calibratedAccelZ < 5.0) &&
          fingerState3 == 1 && fingerState4 == 1 && fingerState1 == 1  && fingerState5 == 0 && fingerState2 == 0);
  }
  bool detectThankyou(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ, 
              float calibratedAccelX, float calibratedAccelY, float calibratedAccelZ,
               int fingerState1, int fingerState2, int fingerState3, int fingerState4, int fingerState5, float accelX, float accelY, float accelZ) {

  return ((calibratedGyroX > 0 && calibratedGyroX < 10) && 
          (calibratedGyroY > 0 && calibratedGyroY < 5) && 
          (calibratedGyroZ > 0)&&
          (calibratedAccelX > 0.0 && calibratedAccelX < 5.0) && 
          (calibratedAccelY > 0.0 && calibratedAccelY < 10.0) && 
          (calibratedAccelZ > 8.0 && calibratedAccelZ < 30.0) &&
          (accelY < -5 && accelZ < 0) &&
          fingerState3 == 0 && fingerState4 == 0 && fingerState1 == 0  && fingerState5 == 0 && fingerState2 == 0);
  }

void loop() {
  // MPU6050 sensor data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calibrated gyro and accelerometer values with absolute value adjustment
  float calibratedGyroX = abs(g.gyro.x - gyroXOffset);
  float calibratedGyroY = abs(g.gyro.y - gyroYOffset);
  float calibratedGyroZ = abs(g.gyro.z - gyroZOffset);
  float calibratedAccelX = abs(a.acceleration.x - accelXOffset);
  float calibratedAccelY = abs(a.acceleration.y - accelYOffset);
  float calibratedAccelZ = abs(a.acceleration.z - accelZOffset);
  float temperature = temp.temperature;  // Temperature from MPU6050
  float accelX = a.acceleration.x;
  float accelY = a.acceleration.y;
  float accelZ =a.acceleration.z;

  // Read flex sensor values
  flexValue1 = analogRead(flexPin1);
  flexValue2 = analogRead(flexPin2);
  flexValue3 = analogRead(flexPin3);
  flexValue4 = analogRead(flexPin4);
  flexValue5 = analogRead(flexPin5);

  // Determine finger states based on thresholds
  fingerState1 = (flexValue1 < thrshold) ? 1 : 0;
  fingerState2 = (flexValue2 < thrshold) ? 1 : 0;
  fingerState3 = (flexValue3 < thrshold) ? 1 : 0;
  fingerState4 = (flexValue4 < 3100) ? 1 : 0;
  fingerState5 = (flexValue5 < thrshold) ? 1 : 0;

  // Gesture detection
  String message = "";
  if (detectBye(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Good Bye !!!";
  } else if (detectHello(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5, accelX, accelY, accelZ)) {
    message = "Hello !!!";
  } else if (detectIam(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "I am";
  } else if (detectPlease(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Please..";
  } else if (detectYes(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Yes";
  } else if (detectNo(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "No";
  } else if (detectSorry(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Sorry..";
  } else if (detectWater(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Water";
  } else if (detectThirsty(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Thirsty";
  } else if (detectI(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "I";
    } else if (detectNeed(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5)) {
    message = "Need";
    } else if (detectThankyou(calibratedGyroX, calibratedGyroY, calibratedGyroZ, calibratedAccelX, calibratedAccelY, calibratedAccelZ,
                fingerState1, fingerState2, fingerState3, fingerState4, fingerState5, accelX, accelY, accelZ)) {
    message = "Thank you !!";
    }

  // Display only temperature and detected gesture on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Temp:%.1fC\n", temperature);  // Show temperature
  display.println(message);  // Show detected gesture (if any)
  display.display();

  // Serial output (Gyro, Accel, Temp, Flex values for debugging)
  Serial.print("Gyro X: "); Serial.print(calibratedGyroX);
  Serial.print(" | Gyro Y: "); Serial.print(calibratedGyroY);
  Serial.print(" | Gyro Z: "); Serial.print(calibratedGyroZ);
  Serial.print(" | Accel X: "); Serial.print(calibratedAccelX);
  Serial.print(" | Accel Y: "); Serial.print(calibratedAccelY);
  Serial.print(" | Accel Z: "); Serial.print(calibratedAccelZ);
  Serial.print(" | Temp: "); Serial.println(temperature);
  Serial.print("Finger 1: "); Serial.print(flexValue1);
  Serial.print(" | Finger 2: "); Serial.print(flexValue2);
  Serial.print(" | Finger 3: "); Serial.print(flexValue3);
  Serial.print(" | Finger 4: "); Serial.print(flexValue4);
  Serial.print(" | Finger 5: "); Serial.println(flexValue5);
  
  if (message.length() > 0) {
    Serial.println(message);
  }

  // Delay before the next reading
  delay(500);
}
