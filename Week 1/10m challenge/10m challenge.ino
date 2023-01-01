// include encoder library
#include <Encoder.h>
#include <Servo.h> //include the servo library

#define servoPin 4
Servo myservo;       // create servo object to control a servo
float steeringAngle; // variable to store the servo position

#define enA 5 // EnableA command line - should be a PWM pin
#define enB 6 // EnableB command line - should be a PWM pin

// name the motor control pins - replace the CHANGEME with your pin number, digital pins do not need the 'D' prefix whereas analogue pins need the 'A' prefix
#define INa A0 // Channel A direction
#define INb A1 // Channel A direction
#define INc A2 // Channel B direction
#define INd A3 // Channel B direction

byte speedSetting = 0; // initial speed = 0

// change these two numbers (if needed) to enable pins connected to encoder
Encoder encLeft(2, 11);  // enable pins with interrupt capability
Encoder encRight(3, 12); // enable pins with interrupt capability

long oldPositionLeft = -999;
long oldPositionRight = -999;

float distanceLeft;
float distanceRight;

void setup()
{
  Serial.begin(9600);
  Serial.println("Digital Trundle Wheel Challenge: ");

  // put your setup code here, to run once:

  myservo.attach(servoPin); // attach our servo object to pin D4
  // the Servo library takes care of defining the PinMode declaration (libraries/Servo/src/avr/Servo.cpp line 240)

  // configure the motor control pins as outputs
  pinMode(INa, OUTPUT);
  pinMode(INb, OUTPUT);
  pinMode(INc, OUTPUT);
  pinMode(INd, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  // initialise serial communication
  Serial.begin(9600);
  Serial.println("Arduino Nano is Running"); // sanity check

  // speedSetting = 255;
  speedSetting = 255;
  motors(speedSetting, speedSetting); // make a call to the "motors" function and provide it with a value for each of the 2 motors - can be different for each motor - using same value here for expedience
  Serial.print("Motor Speeds: ");
  Serial.println(speedSetting);
}

void loop()
{
  myservo.write(107);
  delay(50);
  goForwards();
  long newPositionLeft = encLeft.read();

  // check if encoder has moved
  if (newPositionLeft != oldPositionLeft)
  {
    oldPositionLeft = newPositionLeft;

    // edit the code below to calculate the distance moved, +1 increment = (diameter*pi)/encoder count per revolution
    distanceLeft = newPositionLeft * 0.7853981634;
    // ***

    // output distance to the serial monitor
    Serial.print("Distance left(cm): ");
    Serial.println(distanceLeft);
  }
  long newPositionRight = encRight.read();

  // check if encoder has moved
  if (newPositionRight != oldPositionRight)
  {
    oldPositionRight = newPositionRight;

    // edit the code below to calculate the distance moved, +1 increment = (diameter*pi)/encoder count per revolution
    distanceRight = newPositionRight * 0.7853981634;
    // ***

    // output distance to the serial monitor
    Serial.print("Distance right(cm): ");
    Serial.println(distanceRight);
  }
  if (distanceLeft > 1000)
  {
    stopMotors();
    delay(100);
    stopMotors();
    exit(1);
  }
}

void motors(int leftSpeed, int rightSpeed)
{
  // set individual motor speed
  // direction is set separately

  analogWrite(enA, leftSpeed);  // left
  analogWrite(enB, rightSpeed); // right
}

void moveSteering()
{

  // you may need to change the maximum and minimum servo angle to have the largest steering motion
  int maxAngle = 220;
  int minAngle = 20;
  myservo.write(107);
  for (steeringAngle = minAngle; steeringAngle <= maxAngle; steeringAngle += 1)
  { // goes from minAngle to maxAngle (degrees)
    // in steps of 1 degree
    myservo.write(steeringAngle); // tell servo to go to position in variable 'steeringAngle'
    delay(15);                    // waits 15ms for the servo to reach the position
  }
  for (steeringAngle = maxAngle; steeringAngle >= -minAngle; steeringAngle -= 1)
  {                               // goes from maxAngle to minAngle (degrees)
    myservo.write(steeringAngle); // tell servo to go to position in variable 'steeringAngle'
    delay(15);                    // waits 15 ms for the servo to reach the position
  }
  myservo.write(10);
  delay(5);
}

// for each of the below function, two of the 'IN' variables must be HIGH, and two LOW in order to move the wheels - use a trial and error approach to determine the correct combination for your EEEBot
void goForwards()
{
  myservo.write(107);
  delay(30);
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}

void stopMotors()
{
  digitalWrite(INa, LOW);
  digitalWrite(INb, LOW);
  digitalWrite(INc, LOW);
  digitalWrite(INd, LOW);
}