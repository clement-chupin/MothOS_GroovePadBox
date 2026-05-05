/*Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>
#include "ScreenManager.h"
#include "Tracker.h"

ScreenManager::ScreenManager() {
  lastNoteBeat = -1;
  noteBeatTime = 0;
}

void ScreenManager::Update(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]) {
  bool isShowingInstructions = UpdateInstructionsScreen(tracker, screen, ledCommandOLED, volumeBars, noteChars);
  if (!isShowingInstructions) {
    UpdateMainScreen(tracker, screen, ledCommandOLED, volumeBars, noteChars);
    if (tracker.hintTime > 0) {
      screen.setColorIndex(0);
      screen.drawBox(0, 0, 126, 22);
      screen.setColorIndex(1);
      screen.drawRFrame(0, 0, 126, 22, 2);
      tracker.hintTime--;
      screen.drawStr(4, 16, tracker.hint);
    }
  }
}

bool ScreenManager::UpdateInstructionsScreen(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]) {
  (void)tracker;
  (void)volumeBars;
  (void)noteChars;

  screen.setFont(u8g2_font_6x13_tf);
  const int cellW = 32;
  const int cellH = 32;
  const int textX = 5;
  const int textY = 19;

  if (ledCommandOLED == 'D') {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        screen.drawRFrame(x * cellW, y * cellH, cellW - 1, cellH - 1, 3);
      }
    }
    screen.drawStr(cellW * 0 + textX, cellH * 0 + textY, "NLNG");
    screen.drawStr(cellW * 1 + textX, cellH * 0 + textY, "NMED");
    screen.drawStr(cellW * 2 + textX, cellH * 0 + textY, "NSRT");
    screen.drawStr(cellW * 3 + textX, cellH * 0 + textY, "REC");
    screen.drawStr(cellW * 0 + textX, cellH * 1 + textY, "BPM1");
    screen.drawStr(cellW * 1 + textX, cellH * 1 + textY, "BPM2");
    screen.drawStr(cellW * 2 + textX, cellH * 1 + textY, "BPM3");
    screen.drawStr(cellW * 3 + textX, cellH * 1 + textY, "BPM4");
    screen.drawStr(cellW * 0 + textX, cellH * 2 + textY, "CPAT");
    screen.drawStr(cellW * 1 + textX, cellH * 2 + textY, "PPAT");
    screen.drawStr(cellW * 2 + textX, cellH * 2 + textY, "PALL");
    screen.drawStr(cellW * 3 + textX, cellH * 2 + textY, "N/SP");
    screen.drawStr(cellW * 0 + textX, cellH * 3 + textY, "RSTS");
    screen.drawStr(cellW * 1 + textX, cellH * 3 + textY, "RSTL");
    screen.drawStr(cellW * 2 + textX, cellH * 3 + textY, "MVOL");
    screen.drawStr(cellW * 3 + textX, cellH * 3 + textY, "PTSN");
  } else if (ledCommandOLED == 'C') {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        screen.drawRFrame(x * cellW, y * cellH, cellW - 1, cellH - 1, 3);
      }
    }
    screen.drawStr(cellW * 0 + textX, cellH * 0 + textY, "TRK1");
    screen.drawStr(cellW * 1 + textX, cellH * 0 + textY, "TRK2");
    screen.drawStr(cellW * 2 + textX, cellH * 0 + textY, "TRK3");
    screen.drawStr(cellW * 3 + textX, cellH * 0 + textY, "TRK4");
    screen.drawStr(cellW * 0 + textX, cellH * 1 + textY, "CLT1");
    screen.drawStr(cellW * 1 + textX, cellH * 1 + textY, "CLT2");
    screen.drawStr(cellW * 2 + textX, cellH * 1 + textY, "CLT3");
    screen.drawStr(cellW * 3 + textX, cellH * 1 + textY, "CLT4");
    screen.drawStr(cellW * 0 + textX, cellH * 2 + textY, "PAT1");
    screen.drawStr(cellW * 1 + textX, cellH * 2 + textY, "PAT2");
    screen.drawStr(cellW * 2 + textX, cellH * 2 + textY, "PAT3");
    screen.drawStr(cellW * 3 + textX, cellH * 2 + textY, "PAT4");
    screen.drawStr(cellW * 0 + textX, cellH * 3 + textY, "CLP1");
    screen.drawStr(cellW * 1 + textX, cellH * 3 + textY, "CLP2");
    screen.drawStr(cellW * 2 + textX, cellH * 3 + textY, "CLP3");
    screen.drawStr(cellW * 3 + textX, cellH * 3 + textY, "CLP4");
  } else if (ledCommandOLED == 'B') {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        screen.drawRFrame(x * cellW, y * cellH, cellW - 1, cellH - 1, 3);
      }
    }
    screen.drawStr(cellW * 0 + textX, cellH * 0 + textY, "MUTE");
    screen.drawStr(cellW * 1 + textX, cellH * 0 + textY, "VOL");
    screen.drawStr(cellW * 2 + textX, cellH * 0 + textY, "OVDR");
    screen.drawStr(cellW * 3 + textX, cellH * 0 + textY, "SOLO");
    screen.drawStr(cellW * 0 + textX, cellH * 1 + textY, "ENV1");
    screen.drawStr(cellW * 1 + textX, cellH * 1 + textY, "ENV2");
    screen.drawStr(cellW * 2 + textX, cellH * 1 + textY, "ENV3");
    screen.drawStr(cellW * 3 + textX, cellH * 1 + textY, "LOOP");
    screen.drawStr(cellW * 0 + textX, cellH * 2 + textY, "ECHO");
    screen.drawStr(cellW * 1 + textX, cellH * 2 + textY, "CHRD");
    screen.drawStr(cellW * 2 + textX, cellH * 2 + textY, "WOOS");
    screen.drawStr(cellW * 3 + textX, cellH * 2 + textY, "PTCH");
    screen.drawStr(cellW * 0 + textX, cellH * 3 + textY, "NOFX");
    screen.drawStr(cellW * 1 + textX, cellH * 3 + textY, "LOWP");
    screen.drawStr(cellW * 2 + textX, cellH * 3 + textY, "RTRG");
    screen.drawStr(cellW * 3 + textX, cellH * 3 + textY, "WOBB");
  } else if (ledCommandOLED == 'A') {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        screen.drawRFrame(x * cellW, y * cellH, cellW - 1, cellH - 1, 3);
      }
    }
    screen.drawStr(cellW * 0 + textX, cellH * 0 + textY, "OCT0");
    screen.drawStr(cellW * 1 + textX, cellH * 0 + textY, "OCT1");
    screen.drawStr(cellW * 2 + textX, cellH * 0 + textY, "OCT2");
    screen.drawStr(cellW * 3 + textX, cellH * 0 + textY, "OCT3");
    screen.drawStr(cellW * 0 + textX, cellH * 1 + textY, "INS6");
    screen.drawStr(cellW * 1 + textX, cellH * 1 + textY, "INS7");
    screen.drawStr(cellW * 2 + textX, cellH * 1 + textY, "INS8");
    screen.drawStr(cellW * 3 + textX, cellH * 1 + textY, "INS9");
    screen.drawStr(cellW * 0 + textX, cellH * 2 + textY, "INS2");
    screen.drawStr(cellW * 1 + textX, cellH * 2 + textY, "INS3");
    screen.drawStr(cellW * 2 + textX, cellH * 2 + textY, "INS4");
    screen.drawStr(cellW * 3 + textX, cellH * 2 + textY, "INS5");
    screen.drawStr(cellW * 0 + textX, cellH * 3 + textY, "DRMS");
    screen.drawStr(cellW * 1 + textX, cellH * 3 + textY, "SFX");
    screen.drawStr(cellW * 2 + textX, cellH * 3 + textY, "INS0");
    screen.drawStr(cellW * 3 + textX, cellH * 3 + textY, "INS1");
  } else {
    return false;
  }
  return true;
}

void ScreenManager::UpdateMainScreen(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]) {
  (void)ledCommandOLED;

  char buffa[8];
  if (tracker.lastNoteTrackIndex == lastNoteBeat) {
    if (noteBeatTime > 0) {
      noteBeatTime--;
    }
  } else {
    lastNoteBeat = tracker.lastNoteTrackIndex;
    noteBeatTime = 200;
  }

  screen.drawFrame(0, 0, 128, 128);
  screen.drawLine(0, 24, 127, 24);
  screen.drawLine(0, 66, 127, 66);
  screen.drawLine(74, 0, 74, 66);

  screen.setFont(u8g2_font_6x13_tf);
  screen.drawStr(6, 14, tracker.oledInstString);

  snprintf(buffa, sizeof(buffa), "TR:%d", tracker.selectedTrack + 1);
  screen.drawStr(6, 30, buffa);

  snprintf(buffa, sizeof(buffa), "OC:%d", tracker.voices[tracker.selectedTrack].octave);
  screen.drawStr(6, 44, buffa);

  screen.drawStr(6, 58, tracker.voices[tracker.selectedTrack].samplerMode ? "SAMP" : "NOTE");

  screen.setFont(u8g2_font_logisoso20_tf);
  if (noteBeatTime > 0) {
    snprintf(buffa, sizeof(buffa), "%d", tracker.lastNoteTrackIndex);
    screen.drawStr(84, 23, buffa);
  }

  char stepBuf[12];
  if (!tracker.pressedOnce) {
    snprintf(stepBuf, sizeof(stepBuf), "READY");
  } else {
    int step = tracker.trackIndex % tracker.patternLength;
    snprintf(stepBuf, sizeof(stepBuf), "%02d/%d", step, tracker.patternLength);
  }
  screen.drawStr(78, 52, stepBuf);

  screen.setFont(u8g2_font_6x13_tf);
  snprintf(buffa, sizeof(buffa), "PAT:%d/4", tracker.currentPattern + 1);
  screen.drawStr(78, 63, buffa);

  for (int i = 0; i < 4; i++) {
    int val = tracker.lastSamples[i] / 110;
    val = abs(val);
    if (val > 20) {
      val = 20;
    }
    if (volumeBars[i] < val) {
      volumeBars[i] = val;
    } else {
      volumeBars[i] -= 1;
      if (volumeBars[i] < 0)
        volumeBars[i] = 0;
    }
    int x0 = 4 + i * 31;
    int y0 = 74;
    bool selected = (tracker.selectedTrack == i);

    if (selected) {
      screen.drawRBox(x0, y0, 28, 50, 3);
      screen.setDrawColor(0);
    } else {
      screen.drawRFrame(x0, y0, 28, 50, 3);
    }

    snprintf(buffa, sizeof(buffa), "T%d", i + 1);
    screen.setFont(u8g2_font_6x13_tf);
    screen.drawStr(x0 + 7, y0 + 12, buffa);

    char buff[4];
    bool showNote = false;
    int dnote = 0;
    int doct = 0;
    for (int r = 0; r < 3; r++) {
      int trackIndex = tracker.trackIndex - r;
      if (trackIndex < 0) {
        trackIndex += tracker.patternLength;
      }
      int note = tracker.tracks[i][trackIndex] - 1;
      int oct = tracker.trackOctaves[i][trackIndex];
      if (note < 0) {

      } else {
        dnote = note;
        doct = oct;
        showNote = true;
      }
    }
    if (showNote) {
      screen.setFont(u8g2_font_5x7_tf);
      String(noteChars[dnote] + String(doct)).toCharArray(buff, 4);
      screen.drawStr(x0 + 5, y0 + 22, buff);
    } else {
      screen.setFont(u8g2_font_5x7_tf);
      screen.drawStr(x0 + 6, y0 + 22, "--");
    }

    int meterHeight = volumeBars[i] * 2;
    if (meterHeight > 32) meterHeight = 32;
    if (meterHeight > 0) {
      screen.drawBox(x0 + 10, y0 + 46 - meterHeight, 8, meterHeight);
    }

    int noteAtStep = tracker.tracks[i][tracker.trackIndex];
    if (noteAtStep > 0) {
      screen.drawDisc(x0 + 22, y0 + 10, 2);
    }

    if (selected) {
      screen.setDrawColor(1);
    }
  }
}