/*
 * Ayoub Raji
 * Magnetic door sensor managed by esp8266
 * Low Energy version
 * Wifi disconnects when the door is closed
 * 
 */
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <MQTTClient.h>

const char* ssid     = "xxx";
const char* password = "xxx";
 
WiFiClient WiFiclient;
MQTTClient client;

const int magneticSensorPin = 5;   
const int ledPin =  16;    

// variables for the magnetic sensor
int actualDoorState = 0;     
int previousDoorState = 0;

String messageToSend = "";
  
WiFiClientSecure clientsecure;

bool Start = false;

void WifiConnecting()
{
  //while the connection is not estabilished
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");  
  }
  Serial.println("");
  Serial.print("Wifi connected! My IP address is ");
  Serial.println(WiFi.localIP());
}

void WifiDisconnect()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(1); 
  Serial.print("Wifi Disconnected!");
}

void WifiWakeUp()
{
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password); 
  WifiConnecting();
  Serial.print("Waked up!");
  reconnect();
}

void connect(){

  WifiConnecting();
  
  Serial.println("Connecting to MQTT broker...");
  client.begin("broker.shiftr.io", WiFiclient);
  client.subscribe("/doorSensor");
  client.connect("nodemcu","try","try");
}

void reconnect(){
  Serial.println("Reconnecting to MQTT broker...");
  client.connect("nodemcu","try","try");
}

void setup() {
  //setup serial
  Serial.begin(115200);
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  
  // initialize the magnetic sensor pin as an input:
  pinMode(magneticSensorPin, INPUT_PULLUP);

  //----Wifi connection----//
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //Set the ESP8266 as a WiFi client
  WiFi.mode(WIFI_STA);
  
  //Begin the connection
  WiFi.begin(ssid, password);
  
  connect();

  //led off and door closed for the init
  digitalWrite(ledPin, HIGH);
  previousDoorState = digitalRead(magneticSensorPin);

  if (previousDoorState == HIGH) 
      {
        // turn the LED on:
        digitalWrite(ledPin, LOW);
        Serial.println("Opened");
        messageToSend = "opened";
      } 
      else 
      {
        // turn the LED off:
        digitalWrite(ledPin, HIGH);
        Serial.println("Closed");
        messageToSend = "closed";
      } 
  client.publish("/doorSensor", messageToSend);
  
  delay(500);
  WifiDisconnect();
}

void loop() {
  // read the state of the door state value:
  actualDoorState = digitalRead(magneticSensorPin);

  // check if the doorState has changed from the last time
  if(actualDoorState != previousDoorState)
  {
      //It is necessary to wake up the wifi
      Serial.println(WiFi.status());
      if(WiFi.status() != WL_CONNECTED)
      {
        Serial.println("Trying to wake up the wifi... ");
        WifiWakeUp();
      }
      
      while(!client.connected())
      {
        reconnect();
        delay(500);
      }
      
      if (actualDoorState == HIGH) 
      {
        // turn the LED on:
        digitalWrite(ledPin, LOW);
        Serial.println("Opened");
        messageToSend = "opened";

      } 
      else 
      {
        // turn the LED off:
        digitalWrite(ledPin, HIGH);
        Serial.println("Closed");
        messageToSend = "closed";
      } 

      //---MQTT publish---//     
      client.publish("/doorSensor", messageToSend);

      //If it's closed I turn off the wifi, to reduce current consumption
      if(WiFi.status() == WL_CONNECTED && actualDoorState == LOW)
      {
        delay(500);
        Serial.println("Disconnecting... ");
        WifiDisconnect();
      }
  }

  //update variables
  previousDoorState = actualDoorState;
  delay(10);
}
