#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Timer.h>
#include <time.h>
#include <ArduinoJson.h>

#define DHTPIN 4
#define DHTTYPE DHT22 
#define ONBOARDLED 2
#define S_SOILHUMIDITY_1 35
#define S_SOILHUMIDITY_2 32
#define S_SOILHUMIDITY_3 33
#define S_SOILHUMIDITY_4 34

// WLAN-Daten
const char* ssid = "MagentaWLAN-YN0F";
const char* password = "42942579710037909837";

// MQTT-Broker-Daten
const char* mqtt_server = "192.168.2.150"; // IP-Adresse deines Brokers (z. B. Raspberry Pi)
const int mqtt_port = 1883;
const char* mqtt_topic_temperatur = "joernRoom/esp32_001/sensor/data";
const char* mqtt_topic_status = "joernRoom/esp32_001/status/";

// MQTT-Client
WiFiClient espClient;
PubSubClient client(espClient);

Timer mainTimer;
Timer MQTTTimer;
DHT dht(DHTPIN, DHTTYPE);

// JSON-Daten
int lowerLimit = 100; // startwerte
int upperLimit = 100;
int targetValue = 1800;

// Sensorwerte
float soilHumidity_1;
float soilHumidity_2;
float soilHumidity_3;
float soilHumidity_4;
float airHumidity;
float temperature ; 

// Zeitzoneneinstellungen
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600; // Zeitzone: GMT+1 (für MEZ)
const int daylightOffset_sec = 3600; // Sommerzeit

enum states{INIT, UPDATE_DATA, SLEEP, MOTOR1_EIN};
states actualState = INIT;
bool bEntry = true;
bool autoWateringON;

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
    if (client.connect("ESP32_001Client")) {  // Eindeutiger Client-Name
      Serial.println("Verbunden!");
      client.subscribe("joernRoom/esp32_001/control/led"); //abboniert die entsprechende Topic
      client.subscribe("joernRoom/esp32_001/control/hysteresis"); //abboniert die entsprechende Topic
      client.subscribe("joernRoom/esp32_001/control/autoWatering"); //abboniert die entsprechende Topic

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

  if (String(topic) == "joernRoom/esp32_001/control/led") {
    if (message == "on") {
      digitalWrite(ONBOARDLED, HIGH);  // LED einschalten
      Serial.println("LED eingeschaltet");
    } else if (message == "off") {
      digitalWrite(ONBOARDLED, LOW);   // LED ausschalten
      Serial.println("LED ausgeschaltet");
    }
  }
  if (String(topic) == "joernRoom/esp32_001/control/autoWatering") {
    if (message == "true") {
      autoWateringON =true;
      Serial.println("autoWatering true");
    } else if (message == "false") {
      autoWateringON = false;
      Serial.println("autoWatering false");
    }
  }
  if (String(topic) == "joernRoom/esp32_001/control/hysteresis") {
    // JSON-Payload in einen String konvertieren
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.println(message);

   // JSON-Daten parsen
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

if (error) {
    Serial.print("Fehler beim Parsen der JSON-Daten: ");
    Serial.println(error.c_str());
    return;
}

    // JSON-Daten extrahieren
    lowerLimit = doc["lowerLimit"];
    upperLimit = doc["upperLimit"];
    targetValue = doc["targetValue"];

    // Debug-Ausgabe
    Serial.print("Lower Limit: ");
    Serial.println(lowerLimit);
    Serial.print("Upper Limit: ");
    Serial.println(upperLimit);
    Serial.print("Target Value: ");
    Serial.println(targetValue);
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

void updateSensorValues(){
    soilHumidity_1 = analogRead(S_SOILHUMIDITY_1);
    soilHumidity_2 = analogRead(S_SOILHUMIDITY_2);
    soilHumidity_3 = analogRead(S_SOILHUMIDITY_3);
    soilHumidity_4 = analogRead(S_SOILHUMIDITY_4);
    airHumidity = dht.readHumidity();
    temperature = dht.readTemperature(); //temperatur in Grad lesen


}
void setup() {
  Serial.begin(115200);  
  pinMode(ONBOARDLED, OUTPUT);
  pinMode(S_SOILHUMIDITY_1, INPUT);
  pinMode(S_SOILHUMIDITY_2, INPUT);
  pinMode(S_SOILHUMIDITY_3, INPUT);
  pinMode(S_SOILHUMIDITY_4, INPUT);
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
    Serial.println("INIT");
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
    mainTimer.start(2000);
    updateSensorValues();
    Serial.println("UPDATE_DATA");
    
    // Serial.print("soilHumidity_1");
    // Serial.println(soilHumidity_1);
    // Serial.print("Temperatur");
    // Serial.println(temperature);
    // Serial.print("airHumidity");
    // Serial.println(airHumidity);


    // Aktuellen Timestamp abrufen
    String timestamp = getFormattedTimestamp();

    // MQTT-Nachricht senden
    char payload[200];
    snprintf(payload, sizeof(payload), "{\"timestamp\":\"%s\",\"temperatur\":%.2f,\"airHumidity\":%.2f,\"soilHumidity_1\":%.2f,\"soilHumidity_2\":%.2f,\"soilHumidity_3\":%.2f,\"soilHumidity_4\":%.2f}",
                 timestamp.c_str(), temperature, airHumidity, soilHumidity_1, soilHumidity_2, soilHumidity_3, soilHumidity_4);
    client.publish(mqtt_topic_temperatur, payload);
    Serial.println("Daten gesendet: ");
    Serial.println(payload);

  }
  //do

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  updateSensorValues();

  //onExit
  if(mainTimer.isFinished()){
    bEntry = true;
    actualState = SLEEP;
  }
  if(autoWateringON && (soilHumidity_1 > upperLimit)){
    bEntry = true;
    actualState = MOTOR1_EIN;
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
    Serial.println("SLEEP");
    

  }
  //do
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  updateSensorValues();

  //onExit
  if(mainTimer.isFinished()){
    bEntry = true;
    actualState = UPDATE_DATA;
  }
  break;

  /*
  ************************************************
  */
 case MOTOR1_EIN:
  //on entry
  if(bEntry == true){
    bEntry = false;
    mainTimer.start(30000);
    digitalWrite(ONBOARDLED, HIGH);  
    Serial.println("MOTOR1_EIN");

    // MQTT-Nachricht senden
    char payload[200]; // Puffer für die JSON-Nachricht
    bool pumpState = true; // Beispielwert für die Pumpe (true = an, false = aus)

    // Erstelle die JSON-Nachricht
    snprintf(payload, sizeof(payload), "{\"Pump\": %s}", pumpState);

    // Nachricht veröffentlichen
    client.publish("joernRoom/esp32_001/status/pumpOn", payload);
  }
  //do
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  updateSensorValues();

  //onExit
  if(mainTimer.isFinished() || !autoWateringON || (soilHumidity_1 < lowerLimit)){
    bEntry = true;
    actualState = UPDATE_DATA;
    digitalWrite(ONBOARDLED, LOW);  

   // MQTT-Nachricht senden
    char payload[200]; // Puffer für die JSON-Nachricht
    bool pumpState = false; // Beispielwert für die Pumpe (true = an, false = aus)

    // Erstelle die JSON-Nachricht
    snprintf(payload, sizeof(payload), "{\"Pump\": %s}", pumpState);

    // Nachricht veröffentlichen
    client.publish("joernRoom/esp32_001/status/pumpOn", payload);
  }
  break;

}
  
}