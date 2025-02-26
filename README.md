# SmartHome - MongoDB & MQTT Integration

## ✨ Projektbeschreibung
Dieses Projekt zielt darauf ab, eine SmartHome-Lösung zu entwickeln, die auf einer **MongoDB** zur Speicherung von Sensordaten und einem **MQTT-Broker** zur Kommunikation zwischen den Geräten basiert. 

## 🛠 Installation & Einrichtung
### Voraussetzungen
- Node.js & npm
- MongoDB (lokal oder in der Cloud)
- MQTT-Broker (z. B. Mosquitto oder HiveMQ)

### Installation
1. **Repository klonen**
   ```sh
   git clone https://github.com/dein-benutzername/dein-repository.git
   cd dein-repository
   ```

2. **Abhängigkeiten installieren**
   ```sh
   npm install
   ```

3. **Konfigurationsdatei anpassen**
   Erstelle eine `.env` Datei mit den folgenden Parametern:
   ```env
   MONGO_URI=mongodb://localhost:27017/smarthome
   MQTT_BROKER=mqtt://192.168.x.x:1883
   ```

4. **Server starten**
   ```sh
   npm start
   ```

## 📝 Features
- Speicherung und Abruf von Sensordaten in einer MongoDB-Datenbank
- Kommunikation über einen MQTT-Broker
- Skalierbare Architektur für zukünftige Erweiterungen
- Dashboard-Integration für Visualisierung (geplant)

## 🏆 Ziele
- Einfache Einrichtung und Erweiterbarkeit
- Echtzeit-Kommunikation zwischen IoT-Geräten
- Effiziente Datenverwaltung durch MongoDB

## ⚡ Nutzung
Nach der Installation kann das Projekt gestartet werden, um Sensordaten von MQTT-Clients zu empfangen und in der MongoDB-Datenbank zu speichern. 

## 💎 Mitwirkende
- **Dein Name** - Entwicklung & Wartung
- Beiträge sind willkommen! 

## 📝 Lizenz
Dieses Projekt steht unter der **MIT-Lizenz**.

## 🔄 Nächste Schritte
- Entwicklung eines Frontends zur Visualisierung der Sensordaten
- Implementierung einer Benutzerverwaltung für den Zugriff
- Optimierung der MQTT-Message-Handling-Performance
- Automatische Alarmierung bei definierten Sensorschwellen

## 🛠 Aktuelle Probleme
- Stabilität des MQTT-Brokers bei hoher Nachrichtenfrequenz
- Optimierung der Datenbankabfragen für bessere Performance
- Sicherstellung einer zuverlässigen Fehlerbehandlung bei Netzwerkproblemen

