#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Timer.h>
#include <time.h>

#define DHTPIN 4
#define DHTTYPE DHT22 
#define ONBOARDLED 2


// WLAN-Daten
const char* ssid = "MagentaWLAN-YN0F";
const char* password = "42942579710037909837";

// MQTT-Broker-Daten
const char* mqtt_server = "192.168.2.150"; // IP-Adresse deines Brokers (z. B. Raspberry Pi)
const int mqtt_port = 1883;
const char* mqtt_topic_temperatur = "balcony/esp32_002/sensor/data";


// MQTT-Client
WiFiClient espClient;
PubSubClient client(espClient);

Timer mainTimer;
DHT dht(DHTPIN, DHTTYPE);

// Zeitzoneneinstellungen
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600; // Zeitzone: GMT+1 (für MEZ)
const int daylightOffset_sec = 3600; // Sommerzeit

enum states{INIT, UPDATE_DATA, SLEEP};
states actualState = INIT;
bool bEntry = true;

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
    if (client.connect("ESP32Client")) {  // Eindeutiger Client-Name
      Serial.println("Verbunden!");
      client.subscribe("balcony/esp32_002/control/led"); //abboniert die entsprechende Topic

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

  if (String(topic) == "balcony/esp32_002/control/led") {
    if (message == "on") {
      digitalWrite(ONBOARDLED, HIGH);  // LED einschalten
      Serial.println("LED eingeschaltet");
    } else if (message == "off") {
      digitalWrite(ONBOARDLED, LOW);   // LED ausschalten
      Serial.println("LED ausgeschaltet");
    }
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
  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //onExit
  if(true){
    bEntry = true;
    actualState = UPDATE_DATA;


  }
  break;

  /*
  ************************************************
  */
 case UPDATE_DATA:
  //on entry
  if(bEntry == true){
    bEntry = false;
    mainTimer.start(1000);
    
    
    float airHumidity = dht.readHumidity();
    float temperature = dht.readTemperature(); //temperatur in Grad lesen
  

    // Aktuellen Timestamp abrufen
    String timestamp = getFormattedTimestamp();

    // MQTT-Nachricht senden
    char payload[200];
    snprintf(payload, sizeof(payload), "{\"timestamp\":\"%s\",\"temperatur\":%.2f,\"airHumidity\":%.2f}",
                 timestamp.c_str(), temperature, airHumidity);
    client.publish(mqtt_topic_temperatur, payload);
    Serial.println("Daten gesendet: ");
    Serial.println(payload);

  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  

  //onExit
  if(mainTimer.isFinished()){
    bEntry = true;
    actualState = SLEEP;
  }
  break;

 /*
  ************************************************
  */
 case SLEEP:
  //on entry
  if(bEntry == true){
    bEntry = false;
    mainTimer.start(1000);
    //esp_deep_sleep(10000000); //braucht ca. 30 Sekunden zum Starten
    

  }
  //do
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  

  //onExit
  if(mainTimer.isFinished()){
    bEntry = true;
    actualState = UPDATE_DATA;
  }
  break;

}
  
}