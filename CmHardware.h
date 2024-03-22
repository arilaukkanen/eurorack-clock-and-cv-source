#ifndef CmHardware_H
#define CmHardware_H

#include <Arduino.h>
#include "CmModel.h"
#include "CmView.h"
#include "Output.h"

class CmHardware
{
private:
  // Private constructor to achieve singleton pattern
  CmHardware();
  CmHardware(CmHardware const &);     // Copy disabled
  void operator=(CmHardware const &); // Assigment disabled

  CmModel *model;
  CmView *view;

  bool screensaver = false;

  uint32_t OCR1A_limit;

  void resetOutputPins()
  {
    digitalWrite(PIN_OUTPUT0, LOW);
    digitalWrite(PIN_OUTPUT1, LOW);
    digitalWrite(PIN_OUTPUT2, LOW);
    digitalWrite(PIN_OUTPUT3, LOW);
    digitalWrite(PIN_OUTPUT4, LOW);
    digitalWrite(PIN_OUTPUT5, LOW);
    digitalWrite(PIN_OUTPUT6, LOW);
    digitalWrite(PIN_OUTPUT7, LOW);
  }

  Output *outputs[8];

  bool buttonStatePrev = true;
  uint32_t lastButtonPressMillis = 0;
  uint32_t buttonDownMillis = 0;

  uint16_t stateRotaryA = 0;
  uint16_t stateRunButton = 0;

  uint32_t lastControlMillis = 0;

  void stopScreensaver();
  void splashFlash();

public:
  // Static method to get the instance
  static CmHardware *getInstance()
  {
    static CmHardware hw;
    return &hw;
  };

  void setModel(CmModel *model)
  {
    this->model = model;
  }

  void setView(CmView *view)
  {
    this->view = view;
  }

  CmModel *getModel()
  {
    return model;
  }

  void updateOCR1A_limit();

  void initialize();

  void runModule();
};

#endif
