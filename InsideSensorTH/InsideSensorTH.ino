/*
This sketch uses the DHT22 to get Temprature and Humidity
readings. 
It then uses the ESP8266 to transmit those readings
to the MQTT server. A reading is performed every 10 minuts using Timer. 

  PGM:      InsideSensorTH
  Platform: ESP8266
  Ver:      1.0
  Date:     2016-04-13
  Requires:
  Hardware: Uses the DHT22 Temp and Humid sensor,and an ESP12.

  Notes: This is the stardard for wall powered TH, THL or THP sensors
*/
#include "Timer.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "ethernetSettings.h"
#include "DHT.h"

//This bit sets up the onboard voltage read
extern "C" {
#include "user_interface.h"
uint16 readvdd33(void);
}

#define DHTPIN                2
#define DHTTYPE               DHT22
#define ONE_MINUTE            60000
#define FIVE_MINUTE           300000
#define TEN_MINUTE            600000
#define THIRTY_MINUTE         1200000
#define SIXTY_MINUTE          3600000
#define SENSORID              "ESP04"//Change this as needed
#define eStr                  " getting invalid results from DHT"

char humidity[7],temperatureC[7],voltage[7];
bool dataFlag;

DHT dht(DHTPIN, DHTTYPE, 15);
WiFiClient wireless;
PubSubClient client(wireless);
Timer t;

void getData() {
  float h = dht.readHumidity();
  float c = dht.readTemperature();
  int v = 0.0;
  if (!isnan(c) || !isnan(h)) {   
    Serial.print(h);Serial.print(" | ");Serial.println(c);
    dtostrf(h,4,1,humidity);
    dtostrf(c,4,1,temperatureC);
    dtostrf(1/1000.0,4,2,voltage);
    dataFlag = true;
    return;
  }
  dataFlag = false;
  return;
}

 void sendMQTT()  {
  if(!client.connected()) {
    client.connect(SENSORID,"pi","raspberry");
    Serial.println("Connected");
    Serial.print("rc=");
    Serial.println(client.state());
  }
  if (dataFlag == true) {    
  /*--------------------------------------*/
  /*             Send to MQTT             */
  /*--------------------------------------*/
  String mqttString = "[\"";
  mqttString += SENSORID;
  mqttString += "\",{\"temperature\":\"";
  mqttString += temperatureC;
  mqttString += "\",\"humidity\":\"";
  mqttString += humidity;
  mqttString += "\",\"voltage\":\"";
  mqttString += voltage;
  mqttString += "\"}]";
  Serial.print("MQTT String: ");
  Serial.println(mqttString);
  char payload[100];
    mqttString.toCharArray(payload, 100);
    client.publish("Sensors",payload);
    char pload[50];
    String logString = "Sensor ";
    logString += SENSORID;
    logString += " Trasmission Successful";
    logString.toCharArray(pload, 50);
    client.publish("Log",pload);
  } else  {
    char pload[50];
    String logString = "Sensor ";
    logString += SENSORID;
    logString += eStr;
    logString.toCharArray(pload, 50);
    client.publish("Log", pload);    
  }
 }
void timeIsUp()
{
  Serial.println("TIME!");
  getData();
  sendMQTT();
}

void setup() 
{
  Serial.begin(9600);
  Serial.print(SENSORID);
  Serial.print(" is using Pin ");
  Serial.print(DHTPIN);
  Serial.println(" For DHT sensor.");
  Serial.print("Sensor is ");Serial.println(SENSORID);
  Serial.println("Timer is set for 30 Minutes");
  dht.begin();
  Serial.println("Looking for ");Serial.print(ssid);
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(server,1883);

  int tenMinutePause = t.every(THIRTY_MINUTE, timeIsUp);
  Serial.println("30 minute timmer setup");
  
//collect the first reading and send it.
  timeIsUp();
  delay(1000);
  Serial.println("Setup complete, first data point sent and starting timer loop");
}
  
void loop()
{
  t.update();
}
