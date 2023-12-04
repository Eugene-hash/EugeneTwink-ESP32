#include <FastLED.h>
#include <BluetoothSerial.h>
#include <EEPROM.h>

#include "CONST.h"
#include "CRT.h"

#define NUM_LEDS 100
#define PIN_LEDS 14

struct POSITION {
  uint16_t x;
  uint16_t y;
};

CRGB leds[NUM_LEDS];
BluetoothSerial Bluetooth;

uint32_t timer = millis();
uint8_t bright = 100, mode = 0;
bool isWork = true, isAutoplay = false, isRandom = false;
POSITION positions[NUM_LEDS];

void cameraOpen();
void calibration();

void setup() {
  FastLED.addLeds<WS2811, PIN_LEDS, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(CRT[bright]);

  Bluetooth.begin("ESP32");
  EEPROM.begin(NUM_LEDS << 2);

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    positions[i].x = EEPROM.readUShort((i << 2) + 0);
    positions[i].y = EEPROM.readUShort((i << 2) + 0);
  }
}

uint8_t beatSum = 0;
void loop() {
  if (isWork && millis() - 15 > timer) {
    timer = millis();

    if (isAutoplay) {
      uint8_t beatA = beatsin8(17, 0, 255);
      uint8_t beatB = beatsin8(13, 0, 255);
      beatSum = (beatA + beatB) >> 1;
    } else beatSum += 4;

    switch (mode) {
      case 0:
        {
          for (uint8_t i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(beatSum + positions[i].x + positions[i].y, 255, 255);
        }
        break;

      case 1:
        {
          for (uint8_t i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(beatSum + positions[i].x, 255, 255);
        }
        break;

      case 2:
        {
          for (uint8_t i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(beatSum + positions[i].y, 255, 255);
        }
        break;
    }

    FastLED.show();
  }

  if (Bluetooth.available()) {
    int data = Bluetooth.read();
    switch (data) {
      case MAIN_MODE:
        while (Bluetooth.available() < 1);
        mode = Bluetooth.read();
        break;

      case MAIN_BRIGHT:
        while (Bluetooth.available() < 1);
        bright = Bluetooth.read();
        FastLED.setBrightness(CRT[bright]);
        break;

      case MAIN_SWITCH:
        while (Bluetooth.available() < 2);
        switch (Bluetooth.read()) {
          case SWITCH_POWER:
            isWork = Bluetooth.read();
            if (!isWork) {
              fill_solid(leds, NUM_LEDS, CRGB::Black);
              FastLED.show();
            }
            break;

          case SWITCH_RANDOM:
            isRandom = Bluetooth.read();
            break;

          case SWITCH_AUTOPLAY:
            isAutoplay = Bluetooth.read();
            break;
        }

      case MAIN_SETTING:
        Bluetooth.write(isWork);
        Bluetooth.write(bright);
        Bluetooth.write(isRandom);
        Bluetooth.write(isAutoplay);
        break;

      case CAMERA_OPEN:
        cameraOpen();
    }
  }
}

void cameraOpen() {
  fill_solid(leds, NUM_LEDS, CRGB::Cyan);
  FastLED.setBrightness(CRT[15]);
  FastLED.show();

  bool flagWhile = true;
  while (flagWhile) {
    if (Bluetooth.available()) {
      uint8_t data = Bluetooth.read();

      switch (data) {
        case CAMERA_SCAN_START:
          calibration();
          break;

        case CAMERA_CLOSE:
          flagWhile = false;
          break;
      }
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(CRT[bright]);
  FastLED.show();
}

void calibration() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(CRT[100]);

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();

    Bluetooth.write(CAMERA_SCAN_NEXT);
    while (Bluetooth.available() < 4);

    positions[i].x = Bluetooth.read() << 8;
    positions[i].y |= Bluetooth.read() & 0xff;
    positions[i].x = Bluetooth.read() << 8;
    positions[i].y |= Bluetooth.read() & 0xff;

    EEPROM.writeUShort((i << 2) + 0, positions[i].x);
    EEPROM.writeUShort((i << 2) + 2, positions[i].y);

    leds[i] = CRGB::Black;
  }

  Bluetooth.write(CAMERA_SCAN_STOP);
  EEPROM.commit();

  fill_solid(leds, NUM_LEDS, CRGB::Cyan);
  FastLED.setBrightness(CRT[15]);
  FastLED.show();
}
