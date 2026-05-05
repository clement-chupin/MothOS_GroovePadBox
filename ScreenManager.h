#ifndef ScreenManager_h
#define ScreenManager_h
#include "Tracker.h"
#include <U8g2lib.h>

class ScreenManager {
public:
  ScreenManager();
  void Update(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]);

private:
  int lastNoteBeat;
  int noteBeatTime;
  void UpdateMainScreen(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]);
  bool UpdateInstructionsScreen(Tracker &tracker, U8G2 &screen, char ledCommandOLED, int volumeBars[4], String noteChars[12]);
};

#endif