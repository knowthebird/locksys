#include <Arduino.h>
#include "locksys.h"

#define LED_PIN 13
#define MAX_INPUT 64

char username[MAX_INPUT];
char passphrase[MAX_INPUT];
char input_buffer[MAX_INPUT];

enum InputState {
  WAITING_USERNAME,
  WAITING_PASSWORD
};

InputState state = WAITING_USERNAME;
size_t index = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  locksys_init();

  Serial.println("Access Control Ready.");
  Serial.print("Enter username: ");
}

void blinkSuccess() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void blinkError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

void handleInputLine() {
  input_buffer[index] = '\0';
  index = 0;

  if (state == WAITING_USERNAME) {
    strncpy(username, input_buffer, MAX_INPUT);
    Serial.print("Enter passphrase: ");
    state = WAITING_PASSWORD;
  } else if (state == WAITING_PASSWORD) {
    strncpy(passphrase, input_buffer, MAX_INPUT);
    status_t result = locksys_open_lock(username, passphrase);

    if (result == STATUS_OK) {
      Serial.println("Access granted.");
      blinkSuccess();
    } else if (result == STATUS_ERR_AUTH || result == STATUS_ERR_NOT_FOUND) {
      Serial.println("Authentication Failed.");
      blinkError();
    } else if (result == STATUS_ERR_PERM_LOCKED) {
      Serial.println("Account permanently locked.");
      blinkError();
    } else if (result == STATUS_ERR_THROTTLED) {
      Serial.println("Login temporarily disabled. Please wait.");
      blinkError();
    } else {
      Serial.print("Internal Error. Code: ");
      Serial.println(result);
      blinkError();
    }

    Serial.print("Enter username: ");
    state = WAITING_USERNAME;
  }
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (index > 0) {
        handleInputLine();
      }
    } else if (index < MAX_INPUT - 1) {
      input_buffer[index++] = c;
    }
  }
}
