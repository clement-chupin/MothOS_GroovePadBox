#pragma once

#include <Arduino.h>

// ==================== Keyboard Matrix ====================
// 4x4 matrix as requested for the keyboard.
constexpr uint8_t KEY_ROWS = 4;
constexpr uint8_t KEY_COLS = 4;

// Buttons are read from MCP23X17 expanders (same approach as clavier_v2).
constexpr int BUTTONS_I2C_SDA = 21;
constexpr int BUTTONS_I2C_SCL = 22;
constexpr uint8_t MCP1_ADDR = 0x21;
constexpr uint8_t MCP2_ADDR = 0x23;
constexpr uint8_t MCPEXTRA_ADDR = 0x26;

// Full 8x4 key matrix handling.
// Right half repeats left-half order (ABCD ABCD).
// Row order is inverted so physical top row maps to function keys.

// ==================== Audio I2S ====================
// Pins matching the actual keyboard hardware (same as clavier_v2)
constexpr int I2S_PIN_BCLK = 18;
constexpr int I2S_PIN_WS   = 19;
constexpr int I2S_PIN_DOUT = 5;
constexpr int AUDIO_SAMPLE_RATE = 16384;

// ==================== LED Row ====================
// Top row indicators. MothOS uses 4 channels (A/B/C/D) for modes/functions.
// Avoid GPIO 6..11 on ESP32 (flash bus).
constexpr uint8_t LED_PIN_A = 23;
constexpr uint8_t LED_PIN_B = 13;
constexpr uint8_t LED_PIN_C = 14;
constexpr uint8_t LED_PIN_D = 25;

// ==================== NeoPixel LED strip (clavier_v2 layout) ====================
constexpr uint8_t STRIP_LED_PIN = 23;  // Same GPIO as LED_PIN_A on clavier hardware
constexpr uint8_t ROWS = 4;
constexpr uint8_t COLS = 8;
constexpr uint8_t EXTRA_BUTTONS = 4;
constexpr uint16_t MAIN_BUTTONS = ROWS * COLS;
constexpr uint16_t TOTAL_BUTTONS = MAIN_BUTTONS + EXTRA_BUTTONS;
constexpr uint8_t LEDS_PER_BUTTON = 3;
constexpr uint16_t LED_COUNT = TOTAL_BUTTONS * LEDS_PER_BUTTON;
constexpr uint8_t STRIP_BRIGHTNESS = 200;

// ==================== Display ====================
// 0 = SSD1306 128x64, 1 = SH1107 128x128
constexpr uint8_t DISPLAY_DRIVER_SH1107 = 1;
// Rotation index: 0=R0, 1=R1, 2=R2, 3=R3 (mapped in MothOS.ino)
constexpr uint8_t DISPLAY_ROTATION_MODE = 0;
// U8X8_PIN_NONE literal value to avoid dependency order with U8g2 headers
constexpr int DISPLAY_RESET_PIN = 255;

// I2C pins for the display.
constexpr int DISPLAY_SCL_PIN = 27;
constexpr int DISPLAY_SDA_PIN = 26;
constexpr uint8_t DISPLAY_I2C_ADDR = 0x3C;

// ==================== Potentiometers ====================
// Set USE_POTENTIOMETERS to 0 to disable all analog controls.
#define USE_POTENTIOMETERS 1

constexpr int POT_PIN_VOLUME = 34;   // master volume
constexpr int POT_PIN_REVERB = 35;   // reverb amount (selected track)
constexpr int POT_PIN_DELAY  = 32;   // delay amount (selected track)
constexpr int POT_PIN_PHASER = 33;   // phaser amount (selected track)

// Smoothing and deadzone for stable analog control.
constexpr uint8_t POT_ALPHA_SHIFT = 3;   // 1/8 smoothing
constexpr int POT_DEADZONE = 12;

// Effect depth range mapped from pot (0-4095 → 0-3).
constexpr int POT_EFFECT_MAX = 3;

// Flash budget: keep only a lightweight subset of the SFX sample bank.
// Set to 1 to restore the full original SFX set if a larger app partition is available.
#define USE_FULL_SFX_BANK 0
