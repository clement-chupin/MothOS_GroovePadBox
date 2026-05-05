#pragma once

#include <Arduino.h>
#include "HardwareConfig.h"

// ============================================================
// IO_MAPPING.H - Hardware abstraction for MothOS
// Maps MothOS instrument layout to hardware (buttons, LEDs, display)
// ============================================================

// MothOS uses a different layout than clavier_v2:
// - 4 tracks (columns) x 16 notes per track (rows)
// - vs clavier_v2: 8 columns x 4 rows = 32 keys + 4 extra

// Button layout mapping (how MothOS buttons map to MCP expanders)
// MothOS: 4 columns (tracks) x 16 rows (notes) = 64 buttons
// Hardware: 2 MCP23X17 (32 buttons each) + mcpExtra (4 buttons)

struct ButtonMapping {
  uint8_t mcpIndex;  // 0=mcp1, 1=mcp2, 2=mcpExtra
  uint8_t pin;       // pin on that MCP (0-15)
};

// Track which MCP pin belongs to which MothOS button
// Format: [track][note] -> mcpIndex, pin
static constexpr ButtonMapping buttonMap[4][16] = {
  // Track 0
  {{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7},
   {0,8}, {0,9}, {0,10}, {0,11}, {0,12}, {0,13}, {0,14}, {0,15}},
  // Track 1
  {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7},
   {1,8}, {1,9}, {1,10}, {1,11}, {1,12}, {1,13}, {1,14}, {1,15}},
  // Track 2 (reserved or extended)
  {{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7},
   {0,8}, {0,9}, {0,10}, {0,11}, {0,12}, {0,13}, {0,14}, {0,15}},
  // Track 3 (reserved or extended)
  {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7},
   {1,8}, {1,9}, {1,10}, {1,11}, {1,12}, {1,13}, {1,14}, {1,15}}
};

// LED layout (3 LEDs per button for RGB feedback)
// LED strip has 136 pixels total (4 tracks x 8 cols x 3 LED + 4 extra x 3 LED + 4 extra)
// MothOS uses: 4 x 16 = 64 keys x 3 = 192 LEDs minimum (but we have 136)
// Simplified: Light only center LED and use adjacent for effects

struct LedMapping {
  uint16_t pixelStart;  // First pixel for this button's LEDs
  uint8_t pixelCount;   // Number of pixels (1 or 3)
};

// LED mapping for each MothOS button
// Reuse hardware LED positions efficiently
static constexpr LedMapping ledMap[4][16] = {
  // Track 0 (32 pixels: 0-31)
  {{0,3}, {3,3}, {6,3}, {9,3}, {12,3}, {15,3}, {18,3}, {21,3},
   {24,3}, {27,3}, {30,3}, {33,3}, {36,3}, {39,3}, {42,3}, {45,3}},
  // Track 1 (32 pixels: 48-79)
  {{48,3}, {51,3}, {54,3}, {57,3}, {60,3}, {63,3}, {66,3}, {69,3},
   {72,3}, {75,3}, {78,3}, {81,3}, {84,3}, {87,3}, {90,3}, {93,3}},
  // Track 2 (32 pixels: 96-127)
  {{96,3}, {99,3}, {102,3}, {105,3}, {108,3}, {111,3}, {114,3}, {117,3},
   {120,3}, {123,3}, {126,3}, {129,3}, {132,3}, {135,3}, {0,3}, {3,3}},
  // Track 3 (reserved)
  {{0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1},
   {8,1}, {9,1}, {10,1}, {11,1}, {12,1}, {13,1}, {14,1}, {15,1}}
};

// Color scheme for display feedback
inline uint32_t trackColor(uint8_t track) {
  switch (track) {
    case 0: return 0xFF0000;  // Red
    case 1: return 0x00FF00;  // Green
    case 2: return 0x0000FF;  // Blue
    case 3: return 0xFFFF00;  // Yellow
    default: return 0xFFFFFF;
  }
}

inline uint32_t noteColor(uint8_t note, uint8_t intensity) {
  // Heatmap: low notes = cool, high notes = warm
  uint8_t hue = (note * 255) / 16;
  if (hue < 85) {
    // Blue to Cyan
    return (0 << 16) | (hue * 3 << 8) | (255 << 0);
  } else if (hue < 170) {
    // Cyan to Green
    hue -= 85;
    return (0 << 16) | (255 << 8) | ((255 - hue * 3) << 0);
  } else {
    // Green to Red
    hue -= 170;
    return ((hue * 3) << 16) | ((255 - hue) << 8) | (0 << 0);
  }
}

// Helper to get button state from MCP bitmask
inline bool getButtonStateFromMask(uint32_t mask1, uint32_t mask2, uint8_t track, uint8_t note) {
  const ButtonMapping& m = buttonMap[track][note];
  if (m.mcpIndex == 0) return !(mask1 & (1 << m.pin));
  if (m.mcpIndex == 1) return !(mask2 & (1 << m.pin));
  return false;
}
