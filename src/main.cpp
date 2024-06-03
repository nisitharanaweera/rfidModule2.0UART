#include <Arduino.h>

//------------------MFRC522 register ---------------
#define COMMAND_WAIT        0x02
#define COMMAND_READBLOCK   0x03
#define COMMAND_WRITEBLOCK  0x04
#define MFRC522_HEADER      0xAB

#define STATUS_ERROR        0
#define STATUS_OK           1

#define MIFARE_KEYA         0x00
#define MIFARE_KEYB         0x01

class MFRC522 {
  public:
    MFRC522();
    void begin(HardwareSerial *serial);
    void wait();
    bool available();
    void readCardSerial();
    byte *getCardSerial();
    bool getBlock(byte block, byte keyType, byte *key, byte *returnBlock);
    bool writeBlock(byte block, byte keyType, byte *key, byte *data);
    bool communicate(byte command, byte *sendData, byte sendDataLength, byte *returnData, byte *returnDataLength);
  private:
    void write(byte value);
    byte read();
    HardwareSerial *_Serial;
    byte cardSerial[10];  // Adjust size based on your specific RFID card serial number length
};

MFRC522::MFRC522() {
    _Serial = NULL;
}

void MFRC522::begin(HardwareSerial *serial) {
    _Serial = serial;
    _Serial->begin(9600);
    wait();
}

void MFRC522::wait() {
    _Serial->write(COMMAND_WAIT);
}

bool MFRC522::available() {
    return (_Serial->available() > 0);
}

void MFRC522::readCardSerial() {
    for (int i = 0; i < sizeof(cardSerial); ) {
        if (available()) {
            cardSerial[i] = read();
            i++;
        }
    }
}

byte *MFRC522::getCardSerial() {
    return cardSerial;
}

bool MFRC522::getBlock(byte block, byte keyType, byte *key, byte *returnBlock) {    
    byte sendData[8] = {
        block,      // block
        keyType,    // key type
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // key
    };
    byte returnBlockLength;

    for (int i = 0; i < 6; ++i) {
        sendData[2 + i] = key[i];
    }

    return communicate(
        COMMAND_READBLOCK,  // command
        sendData,           // sendData
        0x0A,               // length
        returnBlock,        // returnData
        &returnBlockLength  // returnDataLength
    );
}

bool MFRC522::writeBlock(byte block, byte keyType, byte *key, byte *data) {    
    byte sendData[24] = {
        block,      // block
        keyType,    // key type
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // key
        0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // Data
    };
    byte returnBlock[3];
    byte returnBlockLength;

    for (int i = 0; i < 6; ++i) {
        sendData[2 + i] = key[i];
    }

    for (int i = 0; i < 16; ++i) {
        sendData[8 + i] = data[i];
    }

    byte result = communicate(
        COMMAND_WRITEBLOCK, // command
        sendData,           // sendData
        0x1A,               // length
        returnBlock,        // returnData
        &returnBlockLength  // returnDataLength
    );

    return result;
}

bool MFRC522::communicate(byte command, byte *sendData, byte sendDataLength, byte *returnData, byte *returnDataLength) {
    // Send instruction to MFRC522
    write(MFRC522_HEADER);      // Header
    write(sendDataLength);      // Length (Length + Command + Data)
    write(command);             // Command

    for (int i = 0; i < sendDataLength - 2; i++) {
        write(sendData[i]);     // Data
    }

    // Read response to MFRC522
    while (!available());
    byte header = read();           // Header
    while (!available());
    *returnDataLength = read();     // Length (Length + Command + Data)
    while (!available());
    byte commandResult = read();    // Command result

    for (int i = 0; i < *returnDataLength - 2; i++) {
        if (available()) {
            returnData[i] = read(); // Data
        }
    }

    // Return
    if (command != commandResult) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

void MFRC522::write(byte value) {
    _Serial->write(value);
}

byte MFRC522::read() {
    return _Serial->read();
}

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
    for (int i = 0; i < 10; i++) {  // Assuming a 10-byte card serial
      Serial.print(cardSerial[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  delay(3000);
}
