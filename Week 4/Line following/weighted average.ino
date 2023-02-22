#include <Wire.h>

// Defining the pins for the sensors
#define sensor1 33
#define sensor2 36
#define sensor3 39
#define sensor4 34
#define sensor5 35
#define sensor6 32
#define reverseLights 23
#define I2C_SLAVE_ADDR 0x04

// Defining the variables
int setPoint = 107;
int error;
int reading;
int weighting;
float weightedAverage;
int sensors[6];
int readings[6];
int sensorWhite[6];
int sensorBlack[6];

// Setup function
void setup()
{
    Serial.begin(9600);
    Wire.begin();
    // Setting the pins up
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
    pinMode(sensor4, INPUT);
    pinMode(sensor5, INPUT);
    pinMode(sensor6, INPUT);
    pinMode(reverseLights, OUTPUT);
    // Storing the sensor pins in an array
    sensors[0] = sensor1;
    sensors[1] = sensor2;
    sensors[2] = sensor3;
    sensors[3] = sensor4;
    sensors[4] = sensor5;
    sensors[5] = sensor6;
    // Calibrating the sensors
    Serial.println("Calibrating sensors");
    // Reading the white value for each sensor and storing it in an array
    for (int i = 0; i < 6; i++)
    {
        sensorWhite[i] = analogRead(sensors[i]);
    }
    Serial.println("Place robot on black surface");
    // Setting reverse light on to give visual feedback that the robot should be placed on the black surface
    digitalWrite(reverseLights, HIGH);
    delay(5000);
    // Reading the black value for each sensor and storing it in an array
    for (int i = 0; i < 6; i++)
    {
        sensorBlack[i] = analogRead(sensors[i]);
    }
}

// Function to send the speed and steering angle to the Nano
void drive(int leftMotor, int rightMotor, int angle)
{
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftMotor & 0x0000FF00) >> 8));
    Wire.write((byte)(leftMotor & 0x000000FF));
    Wire.write((byte)((rightMotor & 0x0000FF00) >> 8));
    Wire.write((byte)(rightMotor & 0x000000FF));
    Wire.write((byte)((angle & 0x0000FF00) >> 8));
    Wire.write((byte)(angle & 0x000000FF));
    Wire.endTransmission();
    delay(10);
}

// Main loop
void loop()
{
    weighting = 0;
    // For loop to get the sensor readings
    for (int i = 0; i < 6; i++)
    {
        reading = analogRead(sensors[i]);
        // Mapping the readings using the calibration values
        reading = map(reading, sensorBlack[i], sensorWhite[i], 0, 255);
        // Constrain the readings to 0-255
        readings[i] = constrain(reading, 0, 255);
        Serial.print(readings[i]);
        Serial.print(",");
    }
    Serial.println();
    // For loop to calculate the weighing
    for (int i = 0; i < 6; i++)
    {
        weighting += readings[i];
    }
    // Calculating the weighted average
    weightedAverage = (readings[0] * -37 + readings[1] * -23 + readings[2] * -7 + readings[3] * 7 + readings[4] * 23 + readings[5] * 37);
    weightedAverage = weightedAverage / weighting * 10;
    // Displaying the weighted average
    Serial.print("Weighted average: ");
    Serial.println(weightedAverage);
    steeringAngle = setpoint - weightedAverage;
    // Constraining the steering angle to not over turn the servo
    steeringAngle = constrain(steeringAngle, 72, 142);
    // Displaying the steering angle
    Serial.print("Steering angle: ");
    Serial.println(steeringAngle);
    // Calling the drive function
    drive(100, 100, steeringAngle);
    delay(200);
}