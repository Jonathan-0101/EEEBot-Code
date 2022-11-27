//****************************************//
//* Example Code for Sending             *//
//* Signed Integers over I2C             *//
//* ESP32 (Master) to Arduino (Slave)    *//
//*                                      *//
//* Slave Code                           *//
//*                                      *//
//* UoN 2022 - Nat Dacombe               *//
//****************************************//

// read through all of the code and the comments before asking for help
// research 'two's compliment' if this is not familiar to you as it is used to represented signed (i.e. positive and negative) values

#include <Wire.h>
#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal

void setup()
{
  Wire.begin(I2C_SLAVE_ADDR);   // join i2c bus #4 - on the Arduino NANO the default I2C pins are A4 (SDA), A5 (SCL)
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop()
{
  delay(100);
}

// this function executes whenever data is received from master device
void receiveEvent(uint16_t howMany)
{
  if(howMany != 4)  // for 2 16-bit numbers, the data will be 4 bytes long - anything else is an error
  {
    emptyBuffer();
    return;
  }
  
  int16_t x = 0;
  int16_t y = 0;
  
  uint8_t x16_9 = Wire.read();  // receive bits 16 to 9 of x (one byte)
  uint8_t x8_1 = Wire.read();   // receive bits 8 to 1 of x (one byte)
  uint8_t y16_9 = Wire.read();   // receive bits 16 to 9 of y (one byte)
  uint8_t y8_1 = Wire.read();   // receive bits 8 to 1 of y (one byte)

  x = (x16_9 << 8) | x8_1; // combine the two bytes into a 16 bit number
  y = (y16_9 << 8) | y8_1; // combine the two bytes into a 16 bit number

  Serial.print("x: ");
  Serial.print(x);
  Serial.print("\t");
  Serial.print("y: ");
  Serial.println(y);
}

// function to clear the I2C buffer
void emptyBuffer(void)
{
  Serial.println("Error: I2C Byte Size Mismatch");
  while(Wire.available())
  {
    Wire.read();
  }
}
