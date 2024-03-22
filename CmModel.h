#ifndef CMMODEL_H
#define CMMODEL_H

#include <Arduino.h>
#include "Output.h"
#include "Resources.h"

static volatile Output o0(PIN_OUTPUT0, NO_ANALOG_OUTPUT);
static volatile Output o1(PIN_OUTPUT1, NO_ANALOG_OUTPUT);
static volatile Output o2(PIN_OUTPUT2, NO_ANALOG_OUTPUT);
static volatile Output o3(PIN_OUTPUT3, NO_ANALOG_OUTPUT);
static volatile Output o4(PIN_OUTPUT4, PIN_ANALOG4);
static volatile Output o5(PIN_OUTPUT5, PIN_ANALOG5);
static volatile Output o6(PIN_OUTPUT6, PIN_ANALOG6);
static volatile Output o7(PIN_OUTPUT7, PIN_ANALOG7);

class CmModel
{
private:
  // Private constructor to achieve singleton pattern
  CmModel();
  CmModel(CmModel const &);        // copy disabled
  void operator=(CmModel const &); // assigment disabled

  /* Private methods */
  void updateSwingTable();
  void resetOutputs();
  void resetOutput(Output *o);
  void setupDefaultOutputs();
  void resetInterruptCounter()
  {
    interruptCounter = 0;
  };

  void prepareOutputSettingsChange();
  void outputSettingsValueChange(int8_t modifier);
  void outputSettingsValueChangeEuclidean(int8_t modifier);
  void outputSettingsValueChangeRandomTriggers(int8_t modifier);
  void outputSettingsValueChangeVoltage(int8_t modifier);
  void outputSettingsValueChangeGateSineSaw(int8_t modifier);

  void bpmChange(int8_t modifier);
  void swingChange(int8_t modifier);

public:
  volatile int interruptCounter = 0;

  volatile Output *outputs[NUM_OUTPUTS];

  byte BPM;
  byte swing;
  volatile byte swingTable[6];

  volatile bool clockRunning = false;

  volatile bool outputChangesReadyForCommit = false;
  volatile bool swingChangeReadyForCommit = false;

  /* Display data: one view, multiple pages */
  uint8_t currentMode;
  byte currentRow = 0;
  byte currentOutput = 0;
  bool viewChanged = true;
  bool typeChanged = false;

  volatile bool renderView = false;

  uint8_t editType = 0;
  uint8_t editClockLength = 0;
  uint8_t editGateLength = 0;
  uint8_t editStartDelayLength = 0;
  int editSequence = 0;
  int editSequenceB = 0;
  byte editRandomTriggerProbability = 0;
  byte editEuclideanSteps = 0;
  byte editSequenceLength = 0;

  // Static method to get the instance
  static CmModel *getInstance()
  {
    static CmModel model;
    return &model;
  };

  void initialize();
  void clockStopped();

  void handleButton();
  void handleButtonLongPress();
  void handleRotary(bool increment);

  void commitOutputSettingsChange();
  void commitSwingChange();
};

#endif
