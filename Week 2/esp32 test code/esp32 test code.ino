#include <Wire.h>
#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal

void setup()
{
    Serial.begin(9600);
    Wire.begin();
}

int leftSpeed, rightSpeed, steeringAngle;

void loop()
{
    leftSpeed = 255;
    rightSpeed = 255;
    steeringAngle = 107;
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(leftSpeed & 0x000000FF));
    Wire.write((byte)((rightSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(rightSpeed & 0x000000FF));
    Wire.write((byte)((steeringAngle & 0x0000FF00) >> 8));
    Wire.write((byte)(steeringAngle & 0x000000FF));
    Wire.endTransmission();
    delay(1000);
    leftSpeed = 100;
    rightSpeed = 100;
    steeringAngle = 40;
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(leftSpeed & 0x000000FF));
    Wire.write((byte)((rightSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(rightSpeed & 0x000000FF));
    Wire.write((byte)((steeringAngle & 0x0000FF00) >> 8));
    Wire.write((byte)(steeringAngle & 0x000000FF));
    Wire.endTransmission();
    delay(1000);
    leftSpeed = 100;
    rightSpeed = 100;
    steeringAngle = 160;
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(leftSpeed & 0x000000FF));
    Wire.write((byte)((rightSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(rightSpeed & 0x000000FF));
    Wire.write((byte)((steeringAngle & 0x0000FF00) >> 8));
    Wire.write((byte)(steeringAngle & 0x000000FF));
    Wire.endTransmission();
    delay(1000);
}
