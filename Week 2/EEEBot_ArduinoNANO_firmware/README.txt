//********************************************************//
//*  University of Nottingham                            *//
//*  Department of Electrical and Electronic Engineering *//
//*                                                      *//
//*  EEEBot Firmware Code for the Arduino NANO           *//
//*  UoN EEEBot 2022                                     *//
//*  Nat Dacombe                                         *//
//********************************************************//

A readme file is commonly posted alongside code in order to explain how to use it
Always read these before implementing or modifying code in order to understand what
you do should/shouldn't change.


DO NOT modify/delete any of the code from the EEEBot_ArduinoNANO_firmware sketch - 
it has been setup with everything you need for Session 2. Write your own code for 
the master ESP32 based off the way the slave Arduino NANO is set up. Use the example
code, both on Moodle and within the Arduino IDE under 'Files --> Examples --> Wire'
to do this.

You may only change the pin numbers relating to the motors if the wheels do not
spin in the correct direction i.e. forwards when a positive integer is sent and 
backwards when a negative integer is sent, and the encoder pin order so that the
count increments and decrements as desired.

1. The Arduino NANO slave address is set as 4 (0x04) - use this in your master code.
2. The only data that the Arduino NANO slave accepts is six bytes i.e. two bytes per
   signed integer. In your master code, send three signed integer values sequentially.
   See on 'Test Code for I2C Communication Between the ESP32 and Arduino NANO' on 
   Moodle for an example of how to do this. 
3. Three integer values are sent to the slave, in the following order: left motor
   speed, right motor speed and steering angle - failure to send these in the correct
   order will affect the EEEBot behaviour.
4. The two integer speed values that are sent to the slave should be between -255 and 
   255. If the value is outside of this range, it will be limited to -255 or 255; 
   whichever is closer. This relates to the PWM value that the motor will run on.
5. The first integer relates to the left motor, the second integer relates to the 
   right motor. Depending on the motor, a PWM value between 0 and ~150 won't be
   sufficient to turn the motor - it is reccommended you use PWM values of 200+.
5. A positive value indicates 'forwards' motion on that wheel, a negative value
   indicates a 'backwards' motion on that wheel.
6. The motors will run at a specified speed until instructed otherwise - send 0,0
   in the appropriate format to stop the vehicle.
7. The integer servo angle value should be between 0 and 180. If the value is outside
   of this range, it will be limited to 0 or 180, whichever is closer. This relates to
   the physical angle that the servo can 'turn to'.
8. On request, the Arduino NANO slave will send the current encoder count raw value of 
   both encoders to the master - it is NOT set to 0 when this happens, so you need to 
   account for this when working out how many encoder counts have passed.