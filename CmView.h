#ifndef CMVIEW_H
#define CMVIEW_H

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <Arduino.h>
#include "CmModel.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

class CmView
{
private:
  // Private constructor to achieve singleton pattern
  CmView();
  CmView(CmView const &);         // Copy disabled
  void operator=(CmView const &); // Assigment disabled

  CmModel *model;
  SSD1306AsciiWire oled;
  void updateDisplay_BPM();
  void updateDisplay_SWING();
  void updateDisplay_OUTPUT_LIST();
  void updateDisplay_OUTPUT_SETTINGS();
  void renderEditOutputFieldFromString(uint8_t n_row, char *f_name, char *f_value);
  void renderEditOutputFieldFromByte(uint8_t n_row, char *f_name, byte f_value);
  void renderStr(char *s);
  void renderValue(byte b);
  void renderNewline();

public:
  // Static method to get the instance
  static CmView *getInstance()
  {
    static CmView view;
    return &view;
  };

  void setModel(CmModel *model)
  {
    this->model = model;
  }

  CmModel *getModel()
  {
    return model;
  }

  /*
     Render display based on model data.
  */
  void render();

  void displaySplashScreen();
  void displayScreensaver();
};

#endif
