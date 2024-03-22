#include "CmView.h"
#include "Output.h"
#include <Arduino.h>

#define DEFAULT_FONT Stang5x7
// #define DEFAULT_FONT Iain5x7
#define BIG_NUMBER_FONT Verdana_digits_24
// #define BIG_NUMBER_FONT lcdnums12x16

CmView::CmView()
{
  if (DEBUG_VIEW)
    Serial.println(F("CmView::CmView()"));

  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&SH1106_128x64, I2C_ADDRESS);

  oled.setFont(DEFAULT_FONT);
}

void CmView::render()
{

  /*
     Display update
  */

  if (model->viewChanged)
  {
    oled.clear();
  }

  if (DEBUG_VIEW)
    Serial.println(F("---------------------"));

  oled.setFont(DEFAULT_FONT);

  switch (model->currentMode)
  {
  case MODE_BPM:
    updateDisplay_BPM();
    break;
  case MODE_SWING:
    updateDisplay_SWING();
    break;
  case MODE_OUTPUT_LIST:
    updateDisplay_OUTPUT_LIST();
    break;
  case MODE_OUTPUT_SETTINGS:
    updateDisplay_OUTPUT_SETTINGS();
    break;
  }

  if (DEBUG_VIEW)
  {
    Serial.println(F("---------------------"));
    Serial.print("Mode: ");
    Serial.println(model->currentMode);
    Serial.print("type: ");
    Serial.println(model->editType);
    Serial.println();
  }

  if (model->viewChanged)
  {
    model->viewChanged = false;
  }
}

void CmView::updateDisplay_BPM()
{
  if (DEBUG_VIEW)
  {
    Serial.println(F("BPM"));
    Serial.println(model->BPM);
  }

  oled.setCursor(54, 6);

  oled.setFont(Iain5x7);
  oled.println(F("BPM"));
  oled.setFont(BIG_NUMBER_FONT);
  if (model->BPM == 99)
  {
    oled.setCursor(0, 2);
    oled.clearToEOL();
  }
  uint8_t pos = 45;
  if (model->BPM > 99)
    pos = pos - 5;
  oled.setCursor(pos, 2);
  oled.print(model->BPM);
  oled.clearToEOL();
  oled.setFont(DEFAULT_FONT);
}

void CmView::updateDisplay_SWING()
{
  if (DEBUG_VIEW)
  {
    Serial.println(F("Swing"));
    Serial.println(model->swing);
  }

  oled.setCursor(50, 6);
  oled.setFont(Iain5x7);
  oled.println(F("Swing"));
  oled.setFont(BIG_NUMBER_FONT);
  if (model->swing == 9)
  {
    oled.setCursor(0, 2);
    oled.clearToEOL();
  }
  uint8_t pos = 54;
  if (model->swing > 9)
    pos = pos - 6;
  oled.setCursor(pos, 2);
  oled.print(model->swing);
  oled.clearToEOL();
  oled.setFont(DEFAULT_FONT);
}

void CmView::updateDisplay_OUTPUT_LIST()
{
  byte &currentRow = model->currentRow;

  /* REFERENCE
    123456789012345678901

    1 Gate  256  256  256
    2 Gate    8   16    0
    3 Gate    4    8    0
    4 Trig    8  P70   L8
    5 Volt    1   L4
    6 Eucl   16   K7  N16
    7 Eucl    8  K11  N15
    8 SawF    2    2    0
    9 Gate  256  256  256
    -----XXXXX-----XXXXX

  */

  oled.setFont(DEFAULT_FONT);

  oled.setCursor(0, 0);

  for (int i = 0; i < 8; i++)
  {
    OutputType type = model->outputs[i]->type;

    renderValue(i + 1);
    renderStr(SPACE);

    /* Indicate changes which are not yet active/committed */

    if (currentRow == i)
    {
      if (model->outputChangesReadyForCommit)
        renderStr(ROW_INDICATOR_COMMIT);
      else
        renderStr(ROW_INDICATOR);
    }
    else
      renderStr(SPACE);

    renderStr(TYPE_TO_STR[type]);
    renderStr(CLOCK_TO_STR[model->outputs[i]->clockLength]);

    if (type == EUCLIDEAN)
    {
      if (model->outputs[i]->euclideanSteps < 10)
        renderStr(SPACE2);
      else
        renderStr(SPACE);
      renderStr(CHAR_K);
      renderValue(model->outputs[i]->euclideanSteps);
      renderStr(SPACE2);
      if (model->outputs[i]->sequenceLength < 10)
        renderStr(SPACE);
      renderStr(CHAR_N);
      renderValue(model->outputs[i]->sequenceLength);
      renderNewline();
    }
    else if (type == RANDOM_TRIGGERS)
    {
      if (model->outputs[i]->randomTriggerProbability < 10)
        renderStr(SPACE);
      if (model->outputs[i]->randomTriggerProbability < 100)
        renderStr(SPACE);
      renderStr(CHAR_P);
      renderValue(model->outputs[i]->randomTriggerProbability);
      renderStr(SPACE2);
      if (model->outputs[i]->sequenceLength < 10)
        renderStr(SPACE);
      renderStr(CHAR_L);
      renderValue(model->outputs[i]->sequenceLength);
      renderNewline();
    }
    else if (type == VOLTAGE)
    {
      if (model->outputs[i]->sequenceLength < 10)
        renderStr(SPACE2);
      else
        renderStr(SPACE);
      renderStr(CHAR_L);
      renderValue(model->outputs[i]->sequenceLength);
      renderNewline();
    }
    else
    {
      renderStr(CLOCK_TO_STR[model->outputs[i]->gateLength]);
      renderStr(CLOCK_TO_STR[model->outputs[i]->startDelayLength]);
      renderNewline();
    }
  }
}

void CmView::updateDisplay_OUTPUT_SETTINGS()
{
  byte &currentOutput = model->currentOutput;
  byte &currentRow = model->currentRow;

  Output *current_output = model->outputs[currentOutput];
  OutputType type = current_output->type;

  oled.setFont(Arial_bold_14);

  if (model->viewChanged)
  {
    oled.setCursor(0, 0);
    oled.print(F("CHANNEL "));
    renderValue(currentOutput + 1);
  }

  oled.setFont(DEFAULT_FONT);

  // Render previous row indicator away
  for (int i = 2; i < 6; i++)
  {
    oled.setCursor(59, i);
    oled.print(SPACE);
  }

  if (currentRow == 0 || model->viewChanged)
    renderEditOutputFieldFromString(0, ROW_TYPE, TYPE_TO_LONG_STR[model->editType]);
  if (currentRow == 1 || model->viewChanged)
    renderEditOutputFieldFromString(1, ROW_CLOCK, CLOCK_TO_LONG_STR[model->editClockLength]);

  switch (model->editType)
  {

  case CLOCK:
  case SAW:
  case SAW_INVERTED:
  case SINE:
    if (currentRow == 2 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromString(2, ROW_GATE, CLOCK_TO_LONG_STR[model->editGateLength]);
    if (currentRow == 3 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromString(3, ROW_DELAY, CLOCK_TO_LONG_STR[model->editStartDelayLength]);
    break;

  case EUCLIDEAN:
    if (currentRow == 2 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromByte(2, ROW_LENGTH, model->editSequenceLength);
    if (currentRow == 3 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromByte(3, ROW_STEPS, model->editEuclideanSteps);
    break;

  case RANDOM_TRIGGERS:
    if (currentRow == 2 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromByte(2, ROW_PROB, model->editRandomTriggerProbability);
    if (currentRow == 3 || model->viewChanged || model->typeChanged)
      renderEditOutputFieldFromByte(3, ROW_SEQUENCE, model->editSequenceLength);
    break;

  case VOLTAGE:
    if (currentRow == 2 || model->viewChanged || model->typeChanged)
    {
      renderEditOutputFieldFromByte(2, ROW_SEQUENCE, model->editSequenceLength);
      renderEditOutputFieldFromString(3, SPACE, SPACE);
    }
    break;
  }
}

void CmView::renderEditOutputFieldFromString(uint8_t n_row, char *f_name, char *f_value)
{
  oled.setCursor(0, n_row + 2);
  renderStr(f_name);
  if (model->currentRow == n_row)
    renderStr(ROW_INDICATOR);
  else
    renderStr(SPACE);
  renderStr(f_value);
  renderNewline();
}

void CmView::renderEditOutputFieldFromByte(uint8_t n_row, char *f_name, byte f_value)
{
  oled.setCursor(0, n_row + 2);
  renderStr(f_name);
  if (model->currentRow == n_row)
    renderStr(ROW_INDICATOR);
  else
    renderStr(SPACE);
  renderValue(f_value);
  renderNewline();
}

void CmView::renderStr(char *str)
{
  if (DEBUG_VIEW)
    Serial.print(str);
  oled.print(str);
}

void CmView::renderValue(byte b)
{
  if (DEBUG_VIEW)
    Serial.print(b);
  oled.print(b);
}

void CmView::renderNewline()
{
  if (DEBUG_VIEW)
    Serial.println();
  oled.clearToEOL();
  oled.println();
}

void CmView::displaySplashScreen()
{
  oled.clear();
  oled.setFont(Arial_bold_14);
  oled.setCursor(26, 2);
  oled.print(F("ClockWork"));
  oled.setCursor(7, 5);
  oled.setFont(DEFAULT_FONT);
  oled.print(F("(c) Ari Laukkanen"));
}

void CmView::displayScreensaver()
{
  oled.clear();
  oled.setFont(Arial_bold_14);
  oled.setCursor(random(60), random(7));
  oled.print(F("ClockWork"));
  oled.setFont(DEFAULT_FONT);
}
