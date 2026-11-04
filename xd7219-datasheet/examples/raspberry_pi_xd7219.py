#!/usr/bin/env python3
"""XD7219 / MAX7219 8-digit 7-segment display demo on Raspberry Pi.

Wiring (5 V logic!):
    XD7219 V+   -> 5V  (Pi 5V pin)
    XD7219 GND  -> GND (both GND pins)
    XD7219 DIN  -> MOSI (GPIO10, pin 19)
    XD7219 CLK  -> SCLK (GPIO11, pin 23)
    XD7219 LOAD -> CE0  (GPIO8,  pin 24)
    XD7219 ISET -> 9.53k -> 5V

The Pi's 3.3 V SPI output is below the XD7219 VIH (3.5 V at V+ = 5 V).
Use a 74HCT245 / TXB0104 level shifter, or power the module from 3.3 V
(out of spec but often works) and accept reduced brightness.

Enable SPI first:  sudo raspi-config  -> Interface -> SPI -> Yes
Test:              python3 raspberry_pi_xd7219.py
"""

import time

import spidev

# Decode-mode character codes for '0'..'9' (0x00-0x09).
FONT_DIGIT = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09]
NUM_DIGITS = 8

spi = spidev.SpiDev()
spi.open(0, 0)              # bus 0, CE0
spi.max_speed_hz = 1_000_000
spi.mode = 0                # CPOL=0, CPHA=0 -> latch on CS rising edge


def write_reg(addr: int, data: int) -> None:
    spi.xfer2([addr & 0xFF, data & 0xFF])   # CE0 low for 16 bits, high after


def init_display(intensity: int = 0x07) -> None:
    write_reg(0x0F, 0x00)                   # display test off
    write_reg(0x09, 0xFF)                   # Code-B decode all digits
    write_reg(0x0B, 0x07)                   # scan 8 digits
    write_reg(0x0A, intensity & 0x0F)       # brightness
    for d in range(NUM_DIGITS):
        write_reg(d + 1, 0x0F)              # 0x0F = blank in decode mode
    write_reg(0x0C, 0x01)                   # normal operation


def show_number(value: int, leading_zeros: bool = False) -> None:
    if value < 0:
        value = 0
    digits = [0] * NUM_DIGITS
    for i in range(NUM_DIGITS - 1, -1, -1):
        digits[i] = value % 10
        value //= 10
    started = leading_zeros
    for i, digit in enumerate(digits):
        if not started and digit == 0 and i != NUM_DIGITS - 1:
            write_reg(i + 1, 0x0F)          # blank leading zero
        else:
            started = True
            write_reg(i + 1, FONT_DIGIT[digit])


def main() -> None:
    init_display(0x07)
    n = 0
    while True:
        show_number(n)
        n = (n + 1) % 100_000_000
        time.sleep(1)


if __name__ == "__main__":
    main()
