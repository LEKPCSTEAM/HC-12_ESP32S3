/*
  MCU: ESP32S3 mini
  Module: HC-12 SI4463 wireless serial module 433Mhz
*/

#include <FastLED.h>
#include <HardwareSerial.h>

#define LED_PIN 48
#define NUM_LEDS 1
#define POWER_PIN 0
#define BUZZER_PIN 2

#define DEVICE_ID "DEVICE01"

CRGB leds[NUM_LEDS];
HardwareSerial HC12(2);

enum MessageType {
  COLOR = 0x01,
  NUMBER = 0x02,
  STATUS_MSG = 0x03,
  TEXT  = 0x04
};

int melody[] = {
  330, 330, 330, 330, 330, 330, 330, 392, 262, 294, 330,
  349, 349, 349, 349, 349, 330, 330, 330, 330, 294, 294, 330, 294, 392
};

int durations[] = {
  4, 4, 2, 4, 4, 2, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2
};

bool checkChecksum(byte* data, int len, byte checksum) {
  byte calculatedChecksum = 0xAA;  // Header
  calculatedChecksum ^= data[0];   // message_type
  calculatedChecksum ^= data[1];   // data_length

  byte payloadSum = 0;
  for (int i = 2; i < len; i++) {
    payloadSum += data[i];
  }
  calculatedChecksum ^= payloadSum;

  printf("Calculated checksum: 0x%02X\n", calculatedChecksum);
  printf("Received checksum: 0x%02X\n", checksum);

  return (calculatedChecksum == checksum);
}

void playJingleBells() {
  int totalNotes = sizeof(melody) / sizeof(int);
  unsigned long startTime = millis();

  for (int i = 0; i < totalNotes; i++) {
    if (millis() - startTime >= 500) break;

    int noteDuration = 1000 / durations[i];
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration * 1.3);
    noTone(BUZZER_PIN);
  }
}

void setup() {
  printf("Initializing system...\n");
  HC12.begin(9600, SERIAL_8N1, 13, 14);  // RX, TX
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  pinMode(POWER_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  printf("System initialized and ready!\n");
}

void receiveData() {
  static byte incomingData[100];
  static int dataIndex = 0;
  static unsigned long lastByteTime = 0;

  if (dataIndex > 0 && millis() - lastByteTime > 1000) {
    printf("Packet timeout! Resetting buffer.\n");
    dataIndex = 0;
  }

  while (HC12.available() > 0) {
    byte incomingByte = HC12.read();
    lastByteTime = millis();

    if (dataIndex >= sizeof(incomingData)) {
      dataIndex = 0;
      printf("Buffer overflow! Resetting buffer.\n");
      continue;
    }

    incomingData[dataIndex] = incomingByte;
    dataIndex++;

    if (dataIndex > 5 && incomingData[0] == 0xAA && incomingData[dataIndex - 1] == 0x55) {
      byte messageType = incomingData[1];
      byte dataLength = incomingData[2];

      if (dataIndex == dataLength + 5) {
        byte checksum = incomingData[dataIndex - 2];

        char id[9];
        for (int i = 3; i < 11; i++) {
          id[i - 3] = incomingData[i];
        }
        id[8] = '\0';

        if (checkChecksum(incomingData + 1, dataIndex - 3, checksum)) {
          String val = "";
          for (int i = 11; i < dataIndex - 2; i++) {
            val += (char)incomingData[i];
          }
          printf("Payload: %s\n", val.c_str());
          handlePacket(messageType, id, val.c_str());
        } else {
          printf("Checksum error!\n");
        }
      } else {
        printf("Invalid packet length!\n");
      }
      dataIndex = 0;
    }
  }
}

void handlePacket(uint8_t messageType, const char* id, const char* payload) {
  if (strlen(id) != 8 || String(id) != DEVICE_ID) {
    printf("Invalid ID: %s\n", id);
    return;
  }

  switch (messageType) {
    case COLOR:
      Serial.printf("สี: %s จาก %s\n", payload, id);
      setColor(payload);
      break;
    case NUMBER:
      Serial.printf("ตัวเลข: %s จาก %s\n", payload, id);
      break;
    case STATUS_MSG:
      Serial.printf("สถานะ: %s จาก %s\n", payload, id);
      break;
    case TEXT:
      Serial.printf("ข้อความ: %s จาก %s\n", payload, id);
      break;
    default:
      Serial.printf("Unknown message type: 0x%02X\n", messageType);
      break;
  }
}

void setColor(String val) {
  if (val == "red") {
    leds[0] = CRGB::Red;
    FastLED.show();
    playJingleBells();
    leds[0] = CRGB::Black;
    FastLED.show();
  } else if (val == "green") {
    leds[0] = CRGB::Green;
    FastLED.show();
    playJingleBells();
    leds[0] = CRGB::Black;
    FastLED.show();
  } else if (val == "blue") {
    leds[0] = CRGB::Blue;
    FastLED.show();
    playJingleBells();
    leds[0] = CRGB::Black;
    FastLED.show();
  } else if (val == "off") {
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}

void loop() {
  receiveData();
}
