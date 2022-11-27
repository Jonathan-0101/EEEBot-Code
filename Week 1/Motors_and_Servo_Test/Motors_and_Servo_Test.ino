//********************************************************//
//*  University of Nottingham                            *//
//*  Department of Electrical and Electronic Engineering *//
//*  Nat Dacombe & Alex Ottway                           *//
//*  UoN EEEBot 2022                                     *//
//*  Motor & Servo Basic Test Code                       *//
//********************************************************//

// ASSUMPTION: Channel A is LEFT, Channel B is RIGHT

// Use this code to correctly assign the four pins to move the car forwards, backwards, clockwise & counter-clockwise
// You first need to change the pin numbers for the four motor 'IN' pins below and then decide which go HIGH and LOW in 
// each of the movements, stopMotors has been done for you
// ** marks where you need to insert the pin number or state

// feel free to modify this code to test existing or new functions

#include <Servo.h>    //include the servo library
#define servoPin 4
Servo myservo;        // create servo object to control a servo
float steeringAngle;  // variable to store the servo position

#define enA 5   //EnableA command line - should be a PWM pin
#define enB 6   //EnableB command line - should be a PWM pin

//name the motor control pins - replace the CHANGEME with your pin number, digital pins do not need the 'D' prefix whereas analogue pins need the 'A' prefix
#define INa A0  //Channel A direction 
#define INb A1 //Channel A direction 
#define INc A2  //Channel B direction 
#define INd A3  //Channel B direction 

byte speedSetting = 0;  //initial speed = 0


void setup() {
  // put your setup code here, to run once:

  myservo.attach(servoPin);  //attach our servo object to pin D4
  //the Servo library takes care of defining the PinMode declaration (libraries/Servo/src/avr/Servo.cpp line 240)

  //configure the motor control pins as outputs
  pinMode(INa, OUTPUT);
  pinMode(INb, OUTPUT);
  pinMode(INc, OUTPUT);
  pinMode(INd, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);   

  //initialise serial communication
  Serial.begin(9600);
  Serial.println("Arduino Nano is Running"); //sanity check

  // speedSetting = 255;
  speedSetting = 150;
  motors(speedSetting, speedSetting); //make a call to the "motors" function and provide it with a value for each of the 2 motors - can be different for each motor - using same value here for expedience
  Serial.print("Motor Speeds: ");
  Serial.println(speedSetting); 
}


void loop() {
  moveSteering();
  delay(500);
  goForwards();
  delay(3000);
  stopMotors();
  delay(500);
  goBackwards();
  delay(3000);
  stopMotors();
  delay(500);
  goClockwise();
  delay(3000);
  stopMotors();
  delay(500);
  goAntiClockwise();
  delay(3000);
  stopMotors();
  delay(500);
}


void motors(int leftSpeed, int rightSpeed) {
  //set individual motor speed
  //direction is set separately

  analogWrite(enA, leftSpeed);
  analogWrite(enB, rightSpeed);
}

void moveSteering() {

  //you may need to change the maximum and minimum servo angle to have the largest steering motion
  int maxAngle = 220;
  int minAngle = 20;
  myservo.write(109);
  for (steeringAngle = minAngle; steeringAngle <= maxAngle; steeringAngle += 1) { //goes from minAngle to maxAngle (degrees)
    //in steps of 1 degree
    myservo.write(steeringAngle);   //tell servo to go to position in variable 'steeringAngle'
    delay(15);                      //waits 15ms for the servo to reach the position
  }
  for (steeringAngle = maxAngle; steeringAngle >= -minAngle; steeringAngle -= 1) { // goes from maxAngle to minAngle (degrees)
    myservo.write(steeringAngle);   //tell servo to go to position in variable 'steeringAngle'
    delay(15);                      //waits 15 ms for the servo to reach the position
  }
  myservo.write(109);
  delay(5); 
}


//for each of the below function, two of the 'IN' variables must be HIGH, and two LOW in order to move the wheels - use a trial and error approach to determine the correct combination for your EEEBot
void goForwards() {
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}

void goBackwards() {
  digitalWrite(INa, LOW);
  digitalWrite(INb, HIGH);
  digitalWrite(INc, LOW);
  digitalWrite(INd, HIGH);
}

void goClockwise() {
  myservo.write(80);
  delay(5);
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  digitalWrite(INc, HIGH);
  digitalWrite(INd, LOW);
}

void goAntiClockwise() {
  myservo.write(160);
  delay(5);
  digitalWrite(INa, LOW);
  digitalWrite(INb, HIGH);
  digitalWrite(INc, LOW);
  digitalWrite(INd, HIGH);
  myservo.write(109);
}

void stopMotors() {
  digitalWrite(INa, LOW);
  digitalWrite(INb, LOW);
  digitalWrite(INc, LOW);
  digitalWrite(INd, LOW);
}