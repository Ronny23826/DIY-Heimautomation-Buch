/*
Project:  PIR Motion dedector HC-SR501 and LED with MQTT und OTA update
Author:   Thomas Edlinger for www.edistechlab.com
Date:     Created 20.09.2021 
Version:  V1.0
IDE:      Arduino IDE 1.8.15
 
Required Board (Tools -> Board -> Boards Manager...)
 - Board: esp8266 by ESP8266 Community   V3.0.2  

Required libraries (sketch -> include library -> manage libraries)
 - PubSubClient by Nick ‘O Leary V2.8.0
 - ArduinoOTA by Juraj Andrassy V1.0.7
*/

#include <PubSubClient.h>
#include <ArduinoOTA.h>
//#include <WiFi.h>  // ESP32 only

#define wifi_ssid "Your_SSID"
#define wifi_password "Your_Password"
#define mqtt_server "MQTT_Server_IP"
#define mqtt_user "MQTT_username"         
#define mqtt_password "MQTT_PW"
#define ESPHostname "Autom_test"
String clientId = "Autom_test-"; 

#define motion_topic "motionsensor" 
#define led_topic "LED"
#define ledStatus_topic "LED_Status"

WiFiClient espClient;  
PubSubClient client(espClient);  

const int motionPin = 4;
const int ledPin = 5;
int pirState = LOW;
 
void setup() {
  Serial.begin(115200);
  setup_wifi();
  ArduinoOTA.setHostname(ESPHostname);
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
   
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
  pinMode(motionPin, INPUT); 
  pinMode(ledPin, OUTPUT);    
}
 
void loop(){
  if (!client.connected()) {  
    reconnect();  
  }
  client.loop();
  ArduinoOTA.handle();
  
  if (digitalRead(motionPin) == HIGH) {
    if (pirState == LOW) {
      Serial.println("Bewegung erkannt");
      client.publish(motion_topic, "ON");
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      Serial.println("Keine Bewegung");
      client.publish(motion_topic, "OFF");
      pirState = LOW;
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}  

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  if (String(topic) == led_topic) {
    if(messageTemp == "ON"){
      Serial.print("Changing LED to ON\n");
      digitalWrite(ledPin, HIGH);
      client.publish(ledStatus_topic, "ON");
      delay(200);
    }
    else if(messageTemp == "OFF"){
      Serial.print("Changing LED to OFF\n");
      digitalWrite(ledPin, LOW);
      client.publish(ledStatus_topic, "OFF"); 
      delay(200);
    }
  }
}
      
 void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(motion_topic, ESPHostname);
      // ... and resubscribe
      client.subscribe(led_topic);
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
   }
}
