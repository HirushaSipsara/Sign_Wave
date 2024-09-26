//The gyro readings I put are rough guessings by considering moving direction.Also I still didnt include accelerometer so not a complete code.

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Offset variables
float gyroXOffset = 0;
float gyroYOffset = 0;
float gyroZOffset = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Set gyroscope and accelerometer range
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU6050 initialized successfully");

  // Calibrate the sensor
  calibrateSensor();
}

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

// Define the gesture detection function
bool detectHi(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return ((calibratedGyroX > 0 && calibratedGyroX < 10) && 
      (calibratedGyroY > 0 && calibratedGyroY < 5) && 
      (calibratedGyroZ > 0) || (calibratedGyroZ < 0));
}
bool detectBye(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY = 0 && calibratedGyroZ = 0);
}
bool detectIam(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY = 0 && calibratedGyroZ > 0);
}
bool detectThankyou(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY > 0 && calibratedGyroZ > 0);
}
bool detectYoureWelcome(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX > 0 && calibratedGyroY = 0 && calibratedGyroZ = 0);
}
bool detectThirsty(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY > 0 && calibratedGyroZ = 0);
}
bool detectHungry(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY > 0 && calibratedGyroZ = 0);
}
bool detectHappy(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY > 0 && calibratedGyroZ = 0);
}
bool detectYes(float calibratedGyroX, float calibratedGyroY, float calibratedGyroZ) {
  return (calibratedGyroX = 0 && calibratedGyroY = 0 && calibratedGyroZ > 0);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Apply offsets
  float calibratedGyroX = g.gyro.x - gyroXOffset;
  float calibratedGyroY = g.gyro.y - gyroYOffset;
  float calibratedGyroZ = g.gyro.z - gyroZOffset;

  Serial.print("Calibrated Gyro X: "); Serial.print(calibratedGyroX);
  Serial.print(" | Calibrated Gyro Y: "); Serial.print(calibratedGyroY);
  Serial.print(" | Calibrated Gyro Z: "); Serial.println(calibratedGyroZ);

  if(detectHi(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
     Serial.println("Hi!!");
  } else if(detectBye(calibratedGyroX, calibratedGyroY, calibratedGyroZ)) {
     Serial.println("Bye");
  } else if(detectIam(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("I am");
  } else if(detectThankyou(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("Thank you");
  } else if(detectYoureWelcome(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("You're Welcome");
  } else if(detectThirsty(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("Thirsty");
  } else if(detectHungry(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("Hungry");
  } else if(detectHappy(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("Happy");
  } else if(detectYes(calibratedGyroX, calibratedGyroY, calibratedGyroZ)){
    Serial.println("Yes");
  }

  delay(800);
}

