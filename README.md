# SmartHome - MongoDB & MQTT Integration

## ✨ Projektbeschreibung
SmartHome Projekt welches die Bodenfeuchtigkeit meiner Pflanzen misst, die Temperatur und Luftfeuchtigkeit an verschiedenen Stellen im Haus, sowie dem Balkon ermittelt und einen Badventilator basierend auf der Luftfeuchtigkeit ein und ausschaltet. Weiterhin sollen demnächst automatisch die Pflanzen bewässert werden und eine Klappe zum Schornstein im Bad für die Abluft geöffnet werden. 

## 🛠 Installation & Einrichtung
### Voraussetzungen
- MongoDB ist als noSQL Datenbank gewählt und läuft auf dem Raspberry pi
- MQTT-Broker ist als Kommunikationsmittel zwischen den einzelnen Teilnehmern im Einsatz. Der Broker läuft auf dem Raspberri Pi

### Installation

   ```

4. **Server starten**
- node-red startet den Server auf dem Raspberry pi
- unter der IP: 192.168.... erreichbar 

## 📝 Features
- Speicherung und Abruf von Sensordaten in einer MongoDB-Datenbank
- Kommunikation über einen MQTT-Broker 
- Dashboard-Integration für Visualisierung über NodeRed, Zugriff über mehrere Geräte

## 🏆 Ziele
- Überwachung der Bodenfeuchtigkeit der Pflanzen
- Überwachung der Lufttemperatur und Luftfeuchtigkeit
- kontrolle des Badventilators basierend auf der aktuellen Luftfeuchtigkeit

## ⚡ Nutzung
Aufrufen des NodeRed Dashboards und einsehen der Livedaten sowie der Daten aus der Vergangenheit. Zeitraum kann in 1h Schritten bis zu 24h in die Vergangenheit gewählt werden.



## 🔄 Nächste Schritte
- Testen der Hard und Software für den Ventilator
- einbinden einer Klappe im Bad, die automatisch geöffnet und geschlossen wird, basierend auf dem Zustand des Ventilators
- implementierung einer Automatischen Bewässerung basierend auf den jeweiligen Bodenfeuchtigkeitswerten
- implementierung für das Einstellen des Schwellenwertes über NodeRed für die Soll Bodenfeuchtigkeit

## 🛠 Aktuelle Probleme
- hohe Ladezeiten für das NodeRed Dashboard aufgrund zu hoher Datenabfragen an die Datenbank
- Zeitraum für die Vergangenen Messdaten nicht optimal einstellbar
- Schwellewert lässt sich nicht gut zuverlässig über das Dashboard einstellen
- 
