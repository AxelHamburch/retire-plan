#include <Arduino.h>

// Pins
constexpr int BTN_Pin4 = 4;   // Taster für Start und Finish (GPIO4)
constexpr int OUT_PINS[] = {12,13,14,15,16,17,18,19}; // 12..18 = Reihenfolge, 19 = special
constexpr size_t NUM_PINS = sizeof(OUT_PINS)/sizeof(OUT_PINS[0]);
constexpr size_t PIN19_IDX = NUM_PINS - 1;

// Timing
constexpr unsigned long STEP_DELAY = 2000;    // 2s between steps
constexpr unsigned long PIN19_ON_MS = 5000;   // Pin19 on time
constexpr unsigned long BLINK_DELAY = 500;    // Blink half-period (1Hz)
constexpr unsigned long SEQUENTIAL_OFF_MS = 200; // Delay for sequential turn-off
constexpr unsigned long DEBOUNCE_MS = 30;     // Button debounce delay

// Zustand
enum class State { Idle, Sequencing, Pin19OnBlinking, Finished };
State state = State::Idle;

// Hilfsfunktionen
inline void pinOn(size_t idx){ digitalWrite(OUT_PINS[idx], LOW); }   // inverted logic: LOW = ON
inline void pinOff(size_t idx){ digitalWrite(OUT_PINS[idx], HIGH); } // HIGH = OFF

// Zusätzliche Hilfsfunktionen
inline void setAllPins(bool on, bool includingPin19 = false) {
    size_t limit = includingPin19 ? NUM_PINS : NUM_PINS - 1;
    for(size_t i = 0; i < limit; i++) {
        on ? pinOn(i) : pinOff(i);
    }
}

// Non-blocking delay helper
bool hasTimeElapsed(unsigned long startTime, unsigned long duration) {
    return (millis() - startTime) >= duration;
}

void setup(){
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\nretire started..");

  pinMode(BTN_Pin4, INPUT_PULLUP);

  for(size_t i=0;i<NUM_PINS;i++){
    pinMode(OUT_PINS[i], OUTPUT);
    pinOff(i); // all off (HIGH)
  }
  // start state: only pin 12 on
  pinOn(0);
}

// simple debounced read (blocking small time)
bool isPressed(int pin){
  if(digitalRead(pin)!=LOW) return false;
  delay(DEBOUNCE_MS);
  return (digitalRead(pin)==LOW);
}

// Warten auf Tastenänderung mit Timeout
bool waitForButton(bool waitForPress, unsigned long timeout_ms = 0) {
    unsigned long startTime = millis();
    while(true) {
        if(timeout_ms && hasTimeElapsed(startTime, timeout_ms)) return false;
        bool pressed = isPressed(BTN_Pin4);
        if(waitForPress == pressed) return true;
        delay(10);
    }
}

void loop(){
  switch(state){
    case State::Idle:
      // wait for start button
      if(isPressed(BTN_Pin4)){
        Serial.println("Start gedrückt: starte Sequenz 13..18");
        state = State::Sequencing;
      }
      break;

    case State::Sequencing: {
      // run steps 13..18 (OUT_PINS[1]..OUT_PINS[6])
      for(size_t i = 1; i < NUM_PINS-1; i++) {
        pinOn(i);
        unsigned long t0 = millis();
        while(!hasTimeElapsed(t0, STEP_DELAY)) { delay(10); }
      }
      Serial.println("Sequenz fertig: warte auf BTN_Pin4 für Pin19");
      
      // Warte auf Tastenloslassen und erneutes Drücken
      waitForButton(false); // Warte auf Loslassen
      waitForButton(true);  // Warte auf Drücken
      
      Serial.println("BTN_Pin4 erneut gedrückt: Pin19 an, warte 5s");
      pinOn(PIN19_IDX);
      
      unsigned long t0 = millis();
      while(!hasTimeElapsed(t0, PIN19_ON_MS)) { delay(10); }
      state = State::Pin19OnBlinking;
      break;
    }

    case State::Pin19OnBlinking: {
      // blink pins 12..18 five times at 1Hz
      Serial.println("Blink 12..18 x5");
      for(int b = 0; b < 5; b++) {
        setAllPins(true);  // Alle Pins an (außer Pin19)
        unsigned long t0 = millis();
        while(!hasTimeElapsed(t0, BLINK_DELAY)) delay(10);
        
        setAllPins(false); // Alle Pins aus (außer Pin19)
        t0 = millis();
        while(!hasTimeElapsed(t0, BLINK_DELAY)) delay(10);
      }
      
      // Turn on all pins 12-18 one last time
      setAllPins(true);
      delay(BLINK_DELAY);
      
      // Sequentially turn off pins 12-18
      Serial.println("Schalte Pins 12-18 nacheinander aus");
      for(size_t i = 0; i < NUM_PINS-1; i++) {
        pinOff(i);
        delay(SEQUENTIAL_OFF_MS);
      }
      
      Serial.println("Blink fertig: Pin19 bleibt an. Warte auf BTN_START zum Neustart");
      state = State::Finished;
      break;
    }

    case State::Finished:
      // Warte auf Tastendruck für Reset auf Pin12
      if(isPressed(BTN_Pin4)) {
        // Alle Pins aus (inklusive Pin19), dann Pin12 an
        setAllPins(false, true);
        pinOn(0);
        Serial.println("Grundstellung (Pin12) eingenommen");
        
        // Warte auf Loslassen des Tasters
        waitForButton(false);
        
        // Zurück zum Idle-Zustand, bereit für nächsten Durchlauf
        Serial.println("System bereit für neue Sequenz");
        state = State::Idle;
      }
      break;
  }

  delay(20);
}