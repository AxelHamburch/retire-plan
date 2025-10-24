# ESP32 Sequentielles Pin-Steuerungsprogramm

## Funktionsbeschreibung

Dieses Programm steuert eine Sequenz von GPIO-Pins auf einem ESP32 mit folgender Logik:

### Grundzustand
- Pin 12 ist eingeschaltet (aktiv LOW)
- Alle anderen Pins (13-19) sind ausgeschaltet

### Ablaufsequenz
1. **Start (BTN_Pin4 drücken)**
   - Aktiviert sequentiell die Pins 13-18
   - Zwischen jedem Schritt 2 Sekunden Pause

2. **Pin 19 Aktivierung (BTN_Pin4 erneut drücken)**
   - Schaltet Pin 19 ein
   - Wartet 5 Sekunden

3. **Blinksequenz**
   - Pins 12-18 blinken 5 mal (1Hz)
   - Danach werden Pins 12-18 nacheinander mit 200ms Verzögerung ausgeschaltet
   - Pin 19 bleibt eingeschaltet

4. **Reset (BTN_Pin4 drücken)**
   - Alle Pins werden ausgeschaltet
   - Pin 12 wird eingeschaltet (Grundzustand)
   - System ist bereit für neue Sequenz

## Pinbelegung
- **BTN_Pin4 (GPIO4)**: Steuerungstaster
- **GPIO12-18**: Sequenzielle Ausgänge
- **GPIO19**: Spezialausgang

## Technische Details
- Invertierte Logik (LOW = EIN, HIGH = AUS)
- Entprellte Tastereingabe
- Serielle Debugausgaben (115200 Baud)