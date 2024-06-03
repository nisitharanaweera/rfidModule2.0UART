#include <Arduino.h>
#include "MFRC522.h"

MFRC522 rfid;

void setup() {
  Serial.begin(115200);        // Initialize serial communication for the monitor
  rfid.begin(&Serial2);        // Initialize RFID module on Serial2 (GPIO 16 and 17 for RX and TX)
  Serial.println("Waiting for RFID card...");
}

void loop() {
  if (rfid.available()) {
    rfid.readCardSerial();
    byte *cardSerial = rfid.getCardSerial();
    Serial.print("Card Serial: ");
    for (int i = 0; i < 5; i++) {
      Serial.print(cardSerial[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  delay(2000);
}