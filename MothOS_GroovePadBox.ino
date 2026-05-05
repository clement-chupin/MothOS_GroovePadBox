// IMPORTANT: comment out ISOLED below if not using an actual OLED.
#define ISOLED

#include <Arduino.h>
#include "HardwareConfig.h"
#include "esp_system.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_NeoPixel.h>
#include <ESP_I2S.h>
#include <WiFi.h>
#include "esp_bt.h"
#include "esp_bt_main.h"

// Color helpers for LED feedback.
static inline uint32_t trackColor(uint8_t track) {
  switch (track) {
    case 0: return 0xFF0000;
    case 1: return 0x00FF00;
    case 2: return 0x0000FF;
    case 3: return 0xFFFF00;
    default: return 0xFFFFFF;
  }
}

static inline uint32_t noteColor(uint8_t note) {
  uint8_t hue = (note * 255) / 16;
  if (hue < 85) {
    return ((uint32_t)(hue * 3) << 8) | 255;
  } else if (hue < 170) {
    hue -= 85;
    return (255U << 8) | (255 - hue * 3);
  } else {
    hue -= 170;
    return ((uint32_t)(hue * 3) << 16) | ((255 - hue) << 8);
  }
}

// Very early diagnostic pin blink.
static void earlyDiagnostic() {
  pinMode(LED_PIN_A, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN_A, HIGH);
    delay(100);
    digitalWrite(LED_PIN_A, LOW);
    delay(100);
  }
}

static const u8g2_cb_t* getDisplayRotation() {
  switch (DISPLAY_ROTATION_MODE) {
    case 1: return U8G2_R1;
    case 2: return U8G2_R2;
    case 3: return U8G2_R3;
    default: return U8G2_R0;
  }
}

#include "InputManager.h"
InputManager inputManager = InputManager();

#include "ScreenManager.h"
ScreenManager screenManager = ScreenManager();

#include "Tracker.h"
Tracker tracker = Tracker();

// clavier_v2-style button scanning through MCP23X17.
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;
Adafruit_MCP23X17 mcpExtra;
TwoWire buttonsWire = TwoWire(1);
static uint32_t g_prevMainMask = 0;
static uint32_t g_lastKeyMs = 0;
static uint32_t g_mainButtonMask = 0;
static uint32_t g_lastButtonScanMs = 0;
static uint32_t g_lastLedRenderMs = 0;
static uint32_t g_lastPotUpdateMs = 0;
static uint32_t g_lastAudioUs = 0;

// I2S sound.
I2SClass i2s;
const int sampleRate = AUDIO_SAMPLE_RATE;

// NeoPixel LED strip.
Adafruit_NeoPixel strip(LED_COUNT, STRIP_LED_PIN, NEO_RGB + NEO_KHZ800);

// OLED display allocated in setup to keep BSS lower.
static U8G2_SH1107_PIMORONI_128X128_F_HW_I2C* screen = nullptr;

TaskHandle_t Task2;
char ledCommandOLED;
int volumeBars[4] = {0, 0, 0, 0};
String noteChars[12];

#if USE_POTENTIOMETERS
const int kPotCount = 4;
const int kPotPins[kPotCount] = {
  POT_PIN_VOLUME,
  POT_PIN_REVERB,
  POT_PIN_DELAY,
  POT_PIN_PHASER
};
int potSmoothed[kPotCount] = {0, 0, 0, 0};
int potApplied[kPotCount] = {0, 0, 0, 0};
#endif

void Task2Loop(void* parameter);
static void setupPotentiometers();
static void updatePotentiometers();
static bool initButtonExpanders();
static uint32_t readMainButtonsMask();
static void refreshMainButtonsMask();
static char readMappedMothKey();
static int getButtonLedIndexSerpentine(int row, int col);
static void renderButtonLeds();
static inline int invertPotValue(int raw);

void setup() {
  earlyDiagnostic();

  Serial.begin(115200);
  delay(250);
  WiFi.mode(WIFI_OFF);
  esp_bt_controller_disable();
  analogReadResolution(12);

  static const char* noteNames[12] = {"C_", "C#", "D_", "D#", "E_", "F_", "F#", "G_", "G#", "A_", "A#", "B"};
  for (int i = 0; i < 12; i++) noteChars[i] = noteNames[i];

  strip.begin();
  strip.setBrightness(STRIP_BRIGHTNESS);
  strip.clear();
  strip.show();

  Wire.begin(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN, 400000);
  Wire.setClock(400000);
  Wire.setTimeOut(20);
  delay(20);

  screen = new U8G2_SH1107_PIMORONI_128X128_F_HW_I2C(getDisplayRotation(), DISPLAY_RESET_PIN);
  screen->setI2CAddress((uint8_t)(DISPLAY_I2C_ADDR << 1));
  screen->setBusClock(400000);
  screen->begin();
  screen->setFont(u8g2_font_6x12_tf);
  screen->clearBuffer();
  screen->drawFrame(0, 0, 128, 128);
  screen->drawStr(20, 50, "MothOS");
  screen->drawStr(10, 68, "Booting...");
  screen->sendBuffer();

  if (!initButtonExpanders()) {
    while (1) {
      delay(250);
    }
  }

  i2s.setPins(I2S_PIN_BCLK, I2S_PIN_WS, I2S_PIN_DOUT);
  if (!i2s.begin(I2S_MODE_STD, sampleRate, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    while (1) {
      delay(250);
    }
  }

#if USE_POTENTIOMETERS
  setupPotentiometers();
#endif

#ifdef ISOLED
  xTaskCreatePinnedToCore(
    Task2Loop,
    "Core 0 task",
    10000,
    NULL,
    1,
    &Task2,
    0);
#endif

  g_lastAudioUs = micros();
}

void loop() {
  refreshMainButtonsMask();

  inputManager.UpdateInput(readMappedMothKey());
  char trackCommand = inputManager.trackCommand;
  int trackCommandArgument = inputManager.trackCommandArgument;
  char ledCommand = inputManager.ledCommand;

  if (ledCommand != ' ') {
    ledCommandOLED = ledCommand;
  }

  if (trackCommand != ' ') {
    tracker.SetCommand(trackCommand, trackCommandArgument);
  }

#if USE_POTENTIOMETERS
  if (millis() - g_lastPotUpdateMs >= 8) {
    g_lastPotUpdateMs = millis();
    updatePotentiometers();
  }
#endif

  // Feed I2S continuously based on elapsed microseconds.
  const uint32_t nowUs = micros();
  uint32_t elapsedUs = nowUs - g_lastAudioUs;
  uint32_t samplesToWrite = (elapsedUs * (uint32_t)sampleRate) / 1000000UL;
  if (samplesToWrite == 0) samplesToWrite = 1;
  if (samplesToWrite > 64) samplesToWrite = 64;
  g_lastAudioUs += (samplesToWrite * 1000000UL) / (uint32_t)sampleRate;

  int16_t frameBuf[64 * 2];
  for (uint32_t s = 0; s < samplesToWrite; s++) {
    tracker.UpdateTracker();

    int32_t boosted = (int32_t)tracker.sample * 2;
    if (boosted > 32767) boosted = 32767;
    if (boosted < -32768) boosted = -32768;
    int16_t sampleOut = (int16_t)boosted;

    frameBuf[s * 2 + 0] = sampleOut;
    frameBuf[s * 2 + 1] = sampleOut;
  }
  i2s.write((uint8_t*)frameBuf, samplesToWrite * sizeof(int16_t) * 2);

  if (millis() - g_lastLedRenderMs >= 16) {
    g_lastLedRenderMs = millis();
    renderButtonLeds();
  }

  inputManager.EndFrame();
}

static void setupPotentiometers() {
#if USE_POTENTIOMETERS
  for (int i = 0; i < kPotCount; i++) {
    pinMode(kPotPins[i], INPUT);
    analogSetPinAttenuation(kPotPins[i], ADC_11db);
    int v = invertPotValue(analogRead(kPotPins[i]));
    potSmoothed[i] = v;
    potApplied[i] = v;
  }
#endif
}

static inline int invertPotValue(int raw) {
  raw = constrain(raw, 0, 4095);
  return 4095 - raw;
}

static void updatePotentiometers() {
#if USE_POTENTIOMETERS
  for (int i = 0; i < kPotCount; i++) {
    int raw = invertPotValue(analogRead(kPotPins[i]));
    int delta = raw - potSmoothed[i];
    potSmoothed[i] += (delta >> POT_ALPHA_SHIFT);

    if (abs(potSmoothed[i] - potApplied[i]) > 2) {
      potApplied[i] = potSmoothed[i];
    }
  }

  tracker.ApplyPotControls(
    potApplied[0],
    potApplied[1],
    potApplied[2],
    potApplied[3]);
#endif
}

void Task2Loop(void* parameter) {
  for (;;) {
    if (!screen) {
      delay(100);
      continue;
    }
    screen->clearBuffer();
    screenManager.Update(tracker, *screen, ledCommandOLED, volumeBars, noteChars);
    screen->sendBuffer();
    delay(120);
  }
}

static bool initButtonExpanders() {
  buttonsWire.begin(BUTTONS_I2C_SDA, BUTTONS_I2C_SCL, 400000);
  buttonsWire.setTimeOut(20);
  delay(20);

  if (!mcp1.begin_I2C(MCP1_ADDR, &buttonsWire)) return false;
  if (!mcp2.begin_I2C(MCP2_ADDR, &buttonsWire)) return false;
  if (!mcpExtra.begin_I2C(MCPEXTRA_ADDR, &buttonsWire)) return false;

  for (int i = 0; i < 16; i++) {
    mcp1.pinMode(i, INPUT_PULLUP);
    mcp2.pinMode(i, INPUT_PULLUP);
    mcpExtra.pinMode(i, INPUT_PULLUP);
  }

  g_mainButtonMask = readMainButtonsMask();
  g_prevMainMask = g_mainButtonMask;
  return true;
}

static uint32_t readMainButtonsMask() {
  uint16_t b1 = mcp1.readGPIOAB();
  uint16_t b2 = mcp2.readGPIOAB();
  uint32_t mask = 0;

  for (int i = 0; i < 32; i++) {
    bool raw = (i < 16) ? !(b1 & (1 << i)) : !(b2 & (1 << (i - 16)));
    int row = i / 8;
    int col = 7 - (i % 8);
    int key = row * 8 + col;
    if (raw) mask |= (1UL << key);
  }
  return mask;
}

static void refreshMainButtonsMask() {
  uint32_t nowMs = millis();
  if (nowMs - g_lastButtonScanMs < 5) return;
  g_lastButtonScanMs = nowMs;
  g_mainButtonMask = readMainButtonsMask();
}

static char readMappedMothKey() {
  // 4 logical columns repeated on both halves: ABCD ABCD.
  static const char mothKeyMap[16] = {
    'M', 'N', 'O', 'P',
    'I', 'J', 'K', 'L',
    'E', 'F', 'G', 'H',
    'A', 'B', 'C', 'D'
  };

  uint32_t nowMs = millis();
  uint32_t latest = g_mainButtonMask;
  uint32_t rising = latest & (~g_prevMainMask);
  g_prevMainMask = latest;

  if (nowMs - g_lastKeyMs < 25) return 0;

  // Physical row 3 is top on this wiring, so invert row mapping.
  for (int pos = 0; pos < (int)(ROWS * COLS); pos++) {
    if (!((rising >> pos) & 0x1U)) continue;
    int row = pos / COLS;
    int col = pos % COLS;
    int effCol = col % KEY_COLS;
    int mapRow = (ROWS - 1 - row);
    int keyIndex = mapRow * KEY_COLS + effCol;

    if (keyIndex >= 0 && keyIndex < 16) {
      g_lastKeyMs = nowMs;
      return mothKeyMap[keyIndex];
    }
  }
  return 0;
}

static int getButtonLedIndexSerpentine(int row, int col) {
  int ledsPerRow = COLS * LEDS_PER_BUTTON;
  int buttonInRow = (row % 2 == 0) ? col : (COLS - 1 - col);
  return (row * ledsPerRow) + (buttonInRow * LEDS_PER_BUTTON);
}

static void renderButtonLeds() {
  strip.clear();

  for (int pos = 0; pos < (int)(ROWS * COLS); pos++) {
    if (!((g_mainButtonMask >> pos) & 0x1U)) continue;

    int row = pos / COLS;
    int col = pos % COLS;
    int effCol = col % KEY_COLS;
    int mapRow = (ROWS - 1 - row);
    int keyIndex = mapRow * KEY_COLS + effCol;

    int ledFirst = getButtonLedIndexSerpentine(row, col);
    if (ledFirst >= LED_COUNT) continue;

    uint32_t color = (keyIndex < 4) ? trackColor(keyIndex) : noteColor((uint8_t)(keyIndex - 4));
    for (int px = 0; px < LEDS_PER_BUTTON; px++) {
      int ledIdx = ledFirst + px;
      if (ledIdx < LED_COUNT) strip.setPixelColor(ledIdx, color);
    }
  }

  if (tracker.tempoBlink > 0) {
    strip.setPixelColor(0, trackColor(tracker.selectedTrack));
  }

  strip.show();
}
