#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <MPU6050_tockn.h>
#include <iostream>
#include <string>

using namespace std;
#define FALSE = 0
#define TRUE = !FALSE
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal

MPU6050 mpu6050(Wire);

const int trigPin = 5;
const int echoPin = 18;
long duration;
float distanceCm;
long timer = 0;
int leftSpeed;
int rightSpeed;
int steeringAngle;
int data[3];

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

float temperature = 0;

// LED pins
const int reverseLights = 23;
const int leftIndicators = 19;
const int rightIndicators = 33;

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
  Wire.begin();
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
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(reverseLights, LOW);
      digitalWrite(leftIndicators, LOW);
      digitalWrite(rightIndicators, LOW);
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
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(leftIndicators, LOW);
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
    }
    else if (messageTemp == "false")
    {
      Serial.println("off");
      digitalWrite(rightIndicators, LOW);
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
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(leftSpeed & 0x000000FF));
    Wire.write((byte)((rightSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(rightSpeed & 0x000000FF));
    Wire.write((byte)((steeringAngle & 0x0000FF00) >> 8));
    Wire.write((byte)(steeringAngle & 0x000000FF));
    Wire.endTransmission();
  }

  // Setting the steering angle of the car
  if (String(topic) == "esp32/steeringAngle")
  {
    steeringAngle = messageTemp.toInt();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write((byte)((leftSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(leftSpeed & 0x000000FF));
    Wire.write((byte)((rightSpeed & 0x0000FF00) >> 8));
    Wire.write((byte)(rightSpeed & 0x000000FF));
    Wire.write((byte)((steeringAngle & 0x0000FF00) >> 8));
    Wire.write((byte)(steeringAngle & 0x000000FF));
    Wire.endTransmission();
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
}
