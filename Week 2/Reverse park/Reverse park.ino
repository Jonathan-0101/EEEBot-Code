#include <Wire.h>
#include <MPU6050_tockn.h>

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal

MPU6050 mpu6050(Wire);

long duration;
float distanceCm;
int reverseLights = 23;
int echoPin = 18;
int trigPin = 5;
long timer = 0;
int leftSpeed;
int rightSpeed;
int steeringAngle;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  mpu6050.begin();
  mpu6050.setGyroOffsets(0, 0, 0);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}

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

void loop()
{
  // Setting the variables for the car to drive forward for 1 second
  leftSpeed = 150;
  rightSpeed = 150;
  steeringAngle = 107;
  // Calling the drive function
  drive(leftSpeed, rightSpeed, steeringAngle);
  delay(1000);
  // make the car turn around 180 degrees
  leftSpeed = 150;
  rightSpeed = 150;
  steeringAngle = 40;
  drive(leftSpeed, rightSpeed, steeringAngle);
  // check the car has rotated 180 degrees usig the gyroscope
  mpu6050.update();
  while (mpu6050.getAngleZ() < 180)
  {
    mpu6050.update();
    Serial.println(mpu6050.getAngleZ());
  }
  leftSpeed = -120;
  rightSpeed = -120;
  steeringAngle = 107;
  drive(leftSpeed, rightSpeed, steeringAngle);
  // stop the car after it is 10 cm from the wall using the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  // while statemtnt to check the distance from the wall
  while (distanceCm > 10)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * SOUND_SPEED / 2;
  }
  // make the car turn 90 degrees to the right
  leftSpeed = 120;
  rightSpeed = 120;
  steeringAngle = 165;
  drive(leftSpeed, rightSpeed, steeringAngle);

  // check the car has rotated 90 degrees usig the gyroscope
  mpu6050.update();
  while (mpu6050.getAngleZ() > 90)
  {
    mpu6050.update();
    delay(100);
    Serial.println(mpu6050.getAngleZ());
  }
  // make the car reverse untill it is 10 cm from the wall
  leftSpeed = -120;
  rightSpeed = -120;
  steeringAngle = 107;
  drive(leftSpeed, rightSpeed, steeringAngle);

  delay(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  while (distanceCm > 10)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * SOUND_SPEED / 2;
  }
  // stop the car
  leftSpeed = 0;
  rightSpeed = 0;
  steeringAngle = 107;
  drive(leftSpeed, rightSpeed, steeringAngle);

  delay(10);
  // exit the program
  exit(0);
}