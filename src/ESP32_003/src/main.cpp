#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Timer.h>
#include <time.h>
#include <Relais.h>
#include <ArduinoJson.h>

#define DHTPIN 4
#define DHTTYPE DHT22 
#define ONBOARDLED 2
#define fan_PIN 32 //aktuell zum testen 2 sonst natürlich den realen pin


// WLAN-Daten
const char* ssid = "MagentaWLAN-YN0F";
const char* password = "42942579710037909837";

// MQTT-Broker-Daten
const char* mqtt_server = "192.168.2.150"; // IP-Adresse deines Brokers (z. B. Raspberry Pi)
const int mqtt_port = 1883;
const char* mqtt_topic_temperatur = "bathroom/esp32_003/sensor/data";


// MQTT-Client
WiFiClient espClient;
PubSubClient client(espClient);

Timer mainTimer;
Timer dataFetchTimer;
Timer publishDataTimer;
Timer motorAnTimer;
DHT dht(DHTPIN, DHTTYPE);
Relais fan(fan_PIN);

// Zeitzoneneinstellungen
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600; // Zeitzone: GMT+1 (für MEZ)
const int daylightOffset_sec = 3600; // Sommerzeit

enum states{INIT, IDLE,  MOTOR_EIN, SLEEP};
states actualState = INIT;
bool bEntry = true;
bool bDataFetch = true;
bool bPublishData = true;
bool bAutoControl = true;
bool previousFanState = fan.getState();

float airHumidity;
float upperCalculatedHumidity; 
float lowerCalculatedHumidity = 70; //defaultwert
float airTemperature;
float joernRoomAirHumidity;

// Funktionen
void main_sm();
void setup_wifi() {
  delay(10);
  Serial.println("Verbindung mit WLAN wird hergestellt...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit WLAN!");

    // Zeit initialisieren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Zeit-Synchronisation abgeschlossen.");
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbindung zum MQTT-Broker wird hergestellt...");
    if (client.connect("ESP32_003Client")) {  // Eindeutiger Client-Name
      Serial.println("Verbunden!");
      client.subscribe("bathroom/esp32_003/control/automaticMode"); //abboniert die entsprechende Topic
      client.subscribe("joernRoom/esp32_001/sensor/data");

    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" Versuche in 5 Sekunden erneut...");
      delay(5000);
    }
  }
}

// Callback-Funktion für die Verarbeitung von Nachrichten
void callback(char* topic, byte* payload, unsigned int length) {
  // Stelle sicher, dass die Nachricht ein Nullterminierungszeichen enthält
  payload[length] = '\0';

  String message = String((char*)payload);

  if (String(topic) == "bathroom/esp32_003/control/automaticMode") {
    if (message == "true") {
      bAutoControl = true;  
      Serial.println("on empfangen");  
    } else if (message == "false") {
      bAutoControl = false;
      Serial.println("off empfangen");  
    }
  }
  if (String(topic) == "joernRoom/esp32_001/sensor/data") {
    // JSON parsen
    StaticJsonDocument<256> doc; // JSON-Dokument mit ausreichend Speicher
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("Fehler beim Parsen der JSON-Daten: ");
        Serial.println(error.c_str());
        return;
    }

    // Werte aus JSON extrahieren

    if (doc.containsKey("airHumidity")) {
        joernRoomAirHumidity = doc["airHumidity"];
    }
    Serial.println(joernRoomAirHumidity);
  }
}

String getFormattedTimestamp() {
  time_t now = time(nullptr);  // Aktuelle Zeit abrufen
  struct tm timeinfo;
  char buffer[30];

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Fehler beim Abrufen der lokalen Zeit!");
    return "";
  }

  // Zeit formatieren (ISO 8601 Format)
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

float getHystersisValue(){
  float hysteresisValue = 80;
  return hysteresisValue;
}

 //! Memberfunktion
  /*!
    \brief liest Sensordaten aus und speichert diese in globale Variablen. Kann in Zukunft auch mittelwerte bilden.
    \param Zeitintervall in dem die Daten abgefragt werden
    \return void    
  */
void fetchSensorData(int intervalTime = 3000){
  
  if(bDataFetch){
    dataFetchTimer.start(intervalTime);
    bDataFetch = false;
  }
  
  if(dataFetchTimer.isFinished()){
    airHumidity = dht.readHumidity();
    airTemperature = dht.readTemperature();
    bDataFetch = true;

  }
  
}

 //! Memberfunktion
  /*!
    \brief ließt globale Variablen und published dies Daten über MQTT
    \param Zeitintervall in dem die Daten abgefragt werden
    \return void    
  */
void publishData(int intervalTime = 3000){
  
  if(bPublishData){
    publishDataTimer.start(intervalTime);
    bPublishData = false;
  }
  
  if(publishDataTimer.isFinished()){
    bPublishData = true;
    String timestamp = getFormattedTimestamp();

    // MQTT-Nachricht senden
    char payload[200];
    snprintf(payload, sizeof(payload), "{\"timestamp\":\"%s\",\"temperatur\":%.2f,\"airHumidity\":%.2f}",
                 timestamp.c_str(), airTemperature, airHumidity);
    client.publish(mqtt_topic_temperatur, payload);
    Serial.println("Daten gesendet: ");
    Serial.println(payload);

  if (fan.getState() != previousFanState)
  {
    previousFanState = fan.getState();
        // MQTT-Nachricht senden
    char payload2[200]; // Puffer für die JSON-Nachricht
    
    // Erstelle die JSON-Nachricht
    snprintf(payload2, sizeof(payload2), "{\"fan\": %s,\"upperAirHumidityLimit\": %f,\"lowerAirHumidityLimit\": %f}", fan.getState() ? "true" : "false", upperCalculatedHumidity, lowerCalculatedHumidity);

    // Nachricht veröffentlichen
    client.publish("bathroom/esp32_003/status", payload2);
  }

  }
  
}

//! Memberfunktion
  /*!
    \brief berechnet basierend auf der Luftfeuchtigkeit joernRoomHumidity die schwellenwerte
  
    \return void    
  */
void calculateHysteresisHumidityValues(){
  upperCalculatedHumidity = 90;
  lowerCalculatedHumidity = joernRoomAirHumidity + 20;
}

void setup() {
  Serial.begin(115200);  
  pinMode(ONBOARDLED, OUTPUT);
  Serial.println("Setup abgeschlossen");
}

void loop() {
 main_sm();
}


void main_sm() {
 
switch(actualState){
  case INIT:
  //on entry
  if(bEntry == true){
    bEntry = false;
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    dht.begin();
    getHystersisValue();
    fan.Off();

  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  fetchSensorData();
  publishData();

  //onExit
  if(true){
    bEntry = true;
    actualState = IDLE;


  }
  break;
 /*
  ************************************************
  */
 case IDLE:
  //on entry
  if(bEntry == true){
    bEntry = false; 
    fan.Off();
  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  calculateHysteresisHumidityValues();
  fetchSensorData();
  publishData();
  

  //onExit
  if(airHumidity > upperCalculatedHumidity && bAutoControl){
    bEntry = true;
    actualState = MOTOR_EIN;
  }
  break;
  /*
  ************************************************
  */
 case MOTOR_EIN:
  //on entry
  if(bEntry == true){
    bEntry = false;
    motorAnTimer.start(300000); // 5 Minuten
    fan.On();   
   
  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  calculateHysteresisHumidityValues();
  fetchSensorData();
  publishData();

  //onExit
  if(motorAnTimer.isFinished() || airHumidity < lowerCalculatedHumidity || !bAutoControl){
    bEntry = true;
    actualState = IDLE;
    fan.Off();
  }
  break;


 

}
  
}