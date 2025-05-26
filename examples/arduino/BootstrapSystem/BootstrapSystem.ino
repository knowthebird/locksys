#include <Arduino.h>
#include "locksys.h"

#define LED_PIN 13

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
    // pass
}
