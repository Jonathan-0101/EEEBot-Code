#include <Wire.h>

// Defining the sensor pins
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
float error = 0;
float lastError = 0;
float Kp = 12.5;
float Ki = 0.825;
float Kd = 1.75;
float K = 0.0125;
float u;
float P;
float I;
float D;
int steeringAngle;
int rightMotorSpeed;
int leftMotorSpeed;
float weighting;
float weightedAverage;
int reading;
int sensors[6];
int readings[6];
int sensorWhite[6];
int sensorBlack[6];

// Setup function
void setup()
{
    delay(5000);
    Serial.begin(9600);
    Wire.begin();
    // Setting the sensor pins to input
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);
    pinMode(sensor4, INPUT);
    pinMode(sensor5, INPUT);
    pinMode(sensor6, INPUT);
    pinMode(reverseLights, OUTPUT);
    sensors[0] = sensor1;
    sensors[1] = sensor2;
    sensors[2] = sensor3;
    sensors[3] = sensor4;
    sensors[4] = sensor5;
    sensors[5] = sensor6;
    Serial.println("Calibrating sensors");
    // Sensor calibration code
    // Reading the sensors when on a white surface
    for (int i = 0; i < 6; i++)
    {
        sensorWhite[i] = analogRead(sensors[i]);
    }
    Serial.println("Place robot on black surface");
    digitalWrite(reverseLights, HIGH);
    delay(5000);
    // Reading the sensors when on a black surface
    for (int i = 0; i < 6; i++)
    {
        sensorBlack[i] = analogRead(sensors[i]);
    }
}

// Function for driving the motors
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
    // Reading sensors and mapping them to 0-255
    for (int i = 0; i < 6; i++)
    {
        reading = analogRead(sensors[i]);
        reading = map(reading, sensorBlack[i], sensorWhite[i], 0, 255);
        readings[i] = constrain(reading, 0, 255);
        Serial.print(readings[i]);
        Serial.print(",");
    }
    Serial.println();
    // Calculating weighting
    for (int i = 0; i < 6; i++)
    {
        weighting += readings[i];
    }
    // Calculating the numerator of the weighted average
    weightedAverage = (readings[0] * -37 + readings[1] * -23 + readings[2] * -7 + readings[3] * 7 + readings[4] * 23 + readings[5] * 37);
    // Checking that neither the weighting nor the weighted average is 0
    if (weighting != 0 && weightedAverage != 0)
    {
        // Calculating the weighted average
        weightedAverage = weightedAverage / weighting;
        Serial.print("Weighted average: ");
        Serial.println(weightedAverage);
        // Setting the error to the weighted average
        error = weightedAverage;
    }
    else
    {
        // If the weighting or the weighted average is 0, the error is set to 0
        error = 0;
    }
    // Setting PID values
    P = error;
    I = I + error;
    D = error - lastError;
    lastError = error;
    // PID equation
    u = (Kp * P) + (Ki * I) + (Kd * D);
    // Using value of u to set motor speeds
    leftMotorSpeed = 110 + K * u;
    rightMotorSpeed = 110 - K * u;
    steeringAngle = 107 + u;
    steeringAngle = constrain(steeringAngle, 72, 142);
    // Resseting weighting to 0
    weighting = 0;
    Serial.print("Steering angle: ");
    Serial.println(steeringAngle);
    Serial.print("Left motor speed: ");
    Serial.println(leftMotorSpeed);
    Serial.print("Right motor speed: ");
    drive(leftMotorSpeed, rightMotorSpeed, steeringAngle);
    delay(200);
}