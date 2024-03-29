#include <Wire.h>

#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal

MPU6050 mpu6050(Wire);

long receivedValue1;
long receivedValue2;
byte receivedByte1;
byte receivedByte2;

void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

void loop()
{
    // Requesting encoder count from arduino
    Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(byte) * 2);
    if (Wire.available() >= sizeof(byte) * 2)
    {
        // reading the data
        byte receivedByte1 = Wire.read();
        byte receivedByte2 = Wire.read();
        // converting the data to a distance
        float leftDistance = receivedByte1 * 0.7853981634;
        float rightDistance = receivedByte2 * 0.7853981634;
        // printing the data
        Serial.print("Left: ");
        Serial.print(leftDistance);
        Serial.print(" Right: ");
        Serial.println(rightDistance);
    }
    delay(1000);
}