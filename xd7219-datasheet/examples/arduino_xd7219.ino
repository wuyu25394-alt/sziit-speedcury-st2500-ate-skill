// XD7219 / XL7219 / MAX7219 — 8-digit 7-segment display demo
//
// Wiring (5 V Arduino Uno/Nano):
//   XD7219 V+   -> 5V
//   XD7219 GND  -> GND   (both GND pins: 4 and 9)
//   XD7219 DIN  -> D11 (MOSI)
//   XD7219 CLK  -> D13 (SCK)
//   XD7219 LOAD -> D10
//   XD7219 ISET -> 9.53k resistor -> 5V   (~40 mA segment current)
//   10uF electrolytic + 100nF ceramic between V+ and GND, close to the chip.
//
// Note: at V+ = 5 V, VIH is 3.5 V. A 3.3 V MCU needs a level shifter
// (e.g. 74HCT245) between the MCU and DIN/CLK/LOAD.

#include <SPI.h>

constexpr uint8_t LOAD_PIN = 10;
constexpr uint8_t NUM_DIGITS = 8;

// Decode-mode character codes for '0'..'9' (0x00-0x09).
// Segment patterns would be 0x7E, 0x30, ... but those are only for no-decode mode.
const uint8_t FONT_DIGIT[10] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};

void writeReg(uint8_t addr, uint8_t data) {
  digitalWrite(LOAD_PIN, LOW);
  SPI.transfer(addr);
  SPI.transfer(data);
  digitalWrite(LOAD_PIN, HIGH);
  delayMicroseconds(1);           // tCSW >= 50 ns
}

void initDisplay(uint8_t intensity = 0x07) {
  writeReg(0x0F, 0x00);           // display test off
  writeReg(0x09, 0xFF);           // Code-B decode for all 8 digits
  writeReg(0x0B, 0x07);           // scan all 8 digits
  writeReg(0x0A, intensity & 0x0F);
  for (uint8_t d = 0; d < NUM_DIGITS; d++) writeReg(d + 1, 0x0F);  // 0x0F = blank (decode mode)
  writeReg(0x0C, 0x01);           // normal operation (last)
}

void clearDisplay() {
  for (uint8_t d = 0; d < NUM_DIGITS; d++) writeReg(d + 1, 0x0F);  // 0x0F, not 0x00 ("0")
}

// Show a non-negative number, right aligned. Returns false if it doesn't fit.
bool showNumber(uint32_t value, bool leadingZeros = false) {
  uint8_t digits[NUM_DIGITS];
  for (uint8_t i = 0; i < NUM_DIGITS; i++) {
    digits[NUM_DIGITS - 1 - i] = value % 10;
    value /= 10;
  }
  if (value != 0) return false;

  bool started = leadingZeros;
  for (uint8_t i = 0; i < NUM_DIGITS; i++) {
    if (!started && digits[i] == 0 && i != NUM_DIGITS - 1) {
      writeReg(i + 1, 0x0F);      // blank leading zero (decode mode)
    } else {
      started = true;
      writeReg(i + 1, FONT_DIGIT[digits[i]]);
    }
  }
  return true;
}

void setBrightness(uint8_t level) {   // 0..15 -> 1/32..31/32 duty
  writeReg(0x0A, level & 0x0F);
}

void shutdown() { writeReg(0x0C, 0x00); }
void wake()     { writeReg(0x0C, 0x01); }

void setup() {
  pinMode(LOAD_PIN, OUTPUT);
  digitalWrite(LOAD_PIN, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz, mode 0

  initDisplay(0x07);
  showNumber(12345678UL);
}

void loop() {
  // Count 0..99999999 once per second
  static uint32_t n = 0;
  static uint32_t last = 0;
  if (millis() - last >= 1000) {
    last = millis();
    if (!showNumber(n)) n = 0;
    n++;
  }
}
