#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <MPU6050_tockn.h>
#include <iostream>
#include <string>
#include <Adafruit_NeoPixel.h>

using namespace std;
#define FALSE = 0
#define TRUE = !FALSE
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define I2C_SLAVE_ADDR 0x04
#define leftPixelStripPin 17
#define rightPixelStripPin 25
#define NUM_PIXELS 8

Adafruit_NeoPixel leftPixelStrip(NUM_PIXELS, leftPixelStripPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightPixelStrip(NUM_PIXELS, rightPixelStripPin, NEO_GRB + NEO_KHZ800);
MPU6050 mpu6050(Wire);
#define reverseLights 23
#define leftIndicators 19
#define rightIndicators 33
#define trigPin 5
#define echoPin 18

// Defining variables
int data[3];
int leftSpeed;
long duration;
long timer = 0;
int rightSpeed;
float distanceCm;
int steeringAngle;
float temperature = 0;
int allLightsCheck = 0;
int leftIndicatorsCheck = 0;
int rightIndicatorsCheck = 0;

// Network credentials
const char *ssid = "tonyTable";
const char *password = "brazillostlol";

// MQTT server address
const char *mqtt_server = "192.168.137.115";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Setting up the script
void setup()
{
  Serial.begin(9600);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(reverseLights, OUTPUT);
  pinMode(leftIndicators, OUTPUT);
  pinMode(rightIndicators, OUTPUT);
  leftPixelStrip.begin();
  rightPixelStrip.begin();
  Wire.begin();
}

// Drive function
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

// WiFi connection setup
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Funtion to act on data received from MQTT server (Node-RED)
void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Turning all the lights on/off
  if (String(topic) == "esp32/allLights")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "true")
    {
      Serial.println("on");
      digitalWrite(reverseLights, HIGH);
      digitalWrite(leftIndicators, HIGH);
      digitalWrite(rightIndicators, HIGH);
      allLightsCheck = 1;
      leftPixelStrip.fill(leftPixelStrip.Color(255, 50, 0));
      rightPixelStrip.fill(rightPixelStrip.Color(255, 50, 0));
      leftPixelStrip.show();
      rightPixelStrip.show();
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(reverseLights, LOW);
      digitalWrite(leftIndicators, LOW);
      digitalWrite(rightIndicators, LOW);
      leftPixelStrip.clear();
      rightPixelStrip.clear();
    }
  }

  // Turning the left indicator lights on/off
  if (String(topic) == "esp32/leftIndicators")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "true")
    {
      Serial.println("on");
      digitalWrite(leftIndicators, HIGH);
      leftIndicatorsCheck = 1;
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(leftIndicators, LOW);
      leftIndicatorsCheck = 0;
      leftIndicators.clear();
    }
  }

  // Turning the right indicator lights on/off
  if (String(topic) == "esp32/rightIndicators")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "true")
    {
      Serial.println("on");
      digitalWrite(rightIndicators, HIGH);
      rightIndicatorsCheck = 1;
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(rightIndicators, LOW);
      rightIndicatorsCheck = 0;
      rightIndicators.clear();
    }
  }

  // Turning the reverse lights on/off
  if (String(topic) == "esp32/reverseLights")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "true")
    {
      Serial.println("on");
      digitalWrite(reverseLights, HIGH);
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(reverseLights, LOW);
    }
  }

  // Setting the speed of the car
  if (String(topic) == "esp32/speed")
  {
    leftSpeed = messageTemp.toInt();
    rightSpeed = messageTemp.toInt();
    drive(leftSpeed, rightSpeed, steeringAngle);
  }

  // Setting the steering angle of the car
  if (String(topic) == "esp32/steeringAngle")
  {
    steeringAngle = messageTemp.toInt();
    drive(leftSpeed, rightSpeed, steeringAngle);
  }
}

void reconnect()
{
  // Loop until reconnected to MQTT server
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");

      // Subscribing to the topics
      client.subscribe("esp32/allLights");
      client.subscribe("esp32/reverseLights");
      client.subscribe("esp32/leftIndicators");
      client.subscribe("esp32/rightIndicators");
      client.subscribe("esp32/speed");
      client.subscribe("esp32/steeringAngle");
      char tempString[8];
      leftSpeed = 0;
      rightSpeed = 0;
      dtostrf(107, 1, 2, tempString);
      Serial.println(tempString);

      // Publish the initial speed and steering angle
      client.publish("esp32/initialSteering", tempString);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Main loop
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  mpu6050.update();
  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
    temperature = mpu6050.getTemp();
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    // Sending the temperature to the MQTT server
    client.publish("esp32/temperature", tempString);
  }
  // Checking if the indicators should be running
  if (leftIndicatorsCheck == 1)
  {
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
    {
      leftPixelStrip.setPixelColor(pixel, leftPixelStrip.Color(255, 50, 0));
      leftPixelStrip.show();
      delay(150);
    }
  }
  if (rightIndicatorsCheck == 1)
  {
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
    {
      rightPixelStrip.setPixelColor(pixel, rightPixelStrip.Color(255, 50, 0));
      rightPixelStrip.show();
      delay(150);
    }
  }
}
