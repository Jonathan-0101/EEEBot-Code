#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

long timer = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop()
{

  mpu6050.update();
  if (millis() - timer > 1000)
  {
    Serial.println("=======================================================");
    Serial.print("temp : ", mpu6050.getTemp());
    Serial.print("accX : ", mpu6050.getAccX());
    Serial.print("\taccY : ", mpu6050.getAccY());
    Serial.println("\taccZ : ", mpu6050.getAccZ());

    Serial.print("gyroX : ", mpu6050.getGyroX());
    Serial.print("\tgyroY : ", mpu6050.getGyroY());
    Serial.println("\tgyroZ : ", mpu6050.getGyroZ());

    Serial.print("accAngleX : ", mpu6050.getAccAngleX());
    Serial.println("\taccAngleY : ", mpu6050.getAccAngleY());

    Serial.print("gyroAngleX : ", mpu6050.getGyroAngleX());
    Serial.print("\tgyroAngleY : ", mpu6050.getGyroAngleY());
    Serial.println("\tgyroAngleZ : ", mpu6050.getGyroAngleZ());

    Serial.print("angleX : ", mpu6050.getAngleX());
    Serial.print("\tangleY : ", mpu6050.getAngleY());
    Serial.println("\tangleZ : ", mpu6050.getAngleZ());
    Serial.println("=======================================================\n");
    timer = millis();
  }
}