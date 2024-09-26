// Define the pin connected to the flex sensor
const int flexSensorPin = 34;  // Change pin number as needed
const int threshold = 500;      // Adjust this threshold based on your sensor

// Function to read flex sensor and determine if it's flexed
bool isFlexed() {
    int sensorReading = analogRead(flexSensorPin); // Read the analog value from the sensor
    return sensorReading > threshold;               // Return true if bent, false otherwise
}

void setup() {
    Serial.begin(115200);                           // Initialize Serial for debugging
    pinMode(flexSensorPin, INPUT);                  // Set the flex sensor pin as input
}

void loop() {
    if (isFlexed()) {
        Serial.println("The sensor is flexed.");
    } else {
        Serial.println("The sensor is not flexed.");
    }
    delay(500); // Delay for readability
}

