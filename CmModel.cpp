#include "CmModel.h"
#include "CmHardware.h"

#define RANDOM_TRIGGER_PROBABILITY_CHANGE_STEP_SIZE 5

CmModel::CmModel()
{
  outputs[0] = &o0;
  outputs[1] = &o1;
  outputs[2] = &o2;
  outputs[3] = &o3;
  outputs[4] = &o4;
  outputs[5] = &o5;
  outputs[6] = &o6;
  outputs[7] = &o7;
}

void CmModel::initialize()
{
  BPM = DEFAULT_BPM;
  swing = DEFAULT_SWING;
  updateSwingTable();
  setupDefaultOutputs();
  resetOutputs();
  currentMode = MODE_BPM;
}

void CmModel::setupDefaultOutputs()
{

  for (int i; i < NUM_OUTPUTS; i++)
  {
    Output *o = outputs[i];
    o->setOutputType(CLOCK);
    o->setClockLength(CLOCK_1x4);
    o->setGateLength(CLOCK_1x32);
    o->setStartDelayLength(NO_CLOCK);
    o->setEuclideanSteps(DEFAULT_EUCLIDEAN_STEPS);
    o->setRandomTriggerProbability(DEFAULT_RANDOM_TRIGGER_PROBABILITY);
    o->generateSequence(DEFAULT_SEQUENCE_LENGTH);
  }

  /**
   * DEFAULTS
   */

  o0.setOutputType(CLOCK);
  o0.setClockLength(CLOCK_1x4);
  o0.setGateLength(CLOCK_1x8);

  o1.setOutputType(CLOCK);
  o1.setClockLength(CLOCK_1x4);
  o1.setGateLength(CLOCK_1x8);
  o1.setStartDelayLength(CLOCK_1x8);

  o2.setOutputType(CLOCK);
  o2.setClockLength(CLOCK_1x8);
  o2.setGateLength(CLOCK_1x16);

  o3.setOutputType(CLOCK);
  o3.setClockLength(CLOCK_1x16);
  o3.setGateLength(CLOCK_1x128);

  o4.setOutputType(EUCLIDEAN);
  o4.setClockLength(CLOCK_1x8);
  o4.setGateLength(CLOCK_1x128);
  o4.setEuclideanSteps(5);
  o4.setSequenceLength(8);
  o4.generateSequence(8);

  o5.setOutputType(EUCLIDEAN);
  o5.setClockLength(CLOCK_1x16);
  o5.setGateLength(CLOCK_1x128);
  o5.setEuclideanSteps(5);
  o5.setSequenceLength(11);
  o5.generateSequence(11);

  o6.setOutputType(VOLTAGE);
  o6.setClockLength(CLOCK_1x1);
  o6.setGateLength(CLOCK_1x1);
  o6.setSequenceLength(4);
  o6.generateSequence(4);

  o7.setOutputType(SINE);
  o7.setClockLength(CLOCK_2x1);
  o7.setGateLength(CLOCK_2x1);
}

void CmModel::updateSwingTable()
{
  swingTable[0] = 0;
  swingTable[1] = swing / 16;
  swingTable[2] = swing / 8;
  swingTable[3] = swing / 4;
  swingTable[4] = swing / 2;
  swingTable[5] = swing;

  if (swing > 0)
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      if (swingTable[i] < 1)
        swingTable[i] = 1;
    }
  }
}

void CmModel::resetOutputs()
{
  for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
  {
    resetOutput(outputs[i]);
  }
}

void CmModel::resetOutput(Output *o)
{

  o->reset();

  if (o->clockLength < CLOCK_LENGTH_SWINGABLE_LIMIT)
  {
    o->setDefaultGateTimesForSwingable();
  }
  else
  {
    o->setDefaultGateTimes();
  }

  if (o->startDelayLength > 0)
  {
    if (o->type == SAW || o->type == SAW_INVERTED || o->type == SINE || o->type == VOLTAGE)
      o->setDelayedEvent(PWM_EVENT, interruptCounter + PWM_EVENT_PPQN);
    else
      o->setDelayedEvent(GATE_OPEN, interruptCounter);
  }
  else
  {
    if (o->type == SAW || o->type == SAW_INVERTED || o->type == SINE || o->type == VOLTAGE)
    {
      o->setPwmEvent(interruptCounter + 1);
    }
    else
    {
      o->setGateCloseEvent(interruptCounter);
    }
  }
}

void CmModel::clockStopped()
{
  resetInterruptCounter();
  resetOutputs();
}

void CmModel::handleButton()
{
  /*
    Rotary button press handler
  */
  switch (currentMode)
  {
  case MODE_BPM:
    currentMode = MODE_SWING;
    viewChanged = true;
    break;

  case MODE_SWING:
    currentMode = MODE_BPM;
    viewChanged = true;
    break;

  case MODE_OUTPUT_LIST:
    /* Change to edit mode, current selected output */
    /* Handle changes on one output at a time */
    if (outputChangesReadyForCommit)
      return;
    currentMode = MODE_OUTPUT_SETTINGS;
    viewChanged = true;
    prepareOutputSettingsChange();
    break;

  case MODE_OUTPUT_SETTINGS:
    currentRow++;
    if ((editType == VOLTAGE && currentRow > 2) || currentRow > 3)
    {
      currentMode = MODE_OUTPUT_LIST;
      viewChanged = true;
      currentRow = currentOutput;
      if (clockRunning)
        outputChangesReadyForCommit = true;
      else
        commitOutputSettingsChange();
    }
    break;
  }
}

void CmModel::handleButtonLongPress()
{
  switch (currentMode)
  {
  case MODE_BPM:
  case MODE_SWING:
    currentMode = MODE_OUTPUT_LIST;
    viewChanged = true;
    currentRow = 0;
    break;

  case MODE_OUTPUT_LIST:
    currentMode = MODE_BPM;
    viewChanged = true;
    break;

  case MODE_OUTPUT_SETTINGS:
    /* Back up one row or cancel edits on first row */
    currentRow--;
    if (currentRow == 255)
    {
      currentMode = MODE_OUTPUT_LIST;
      viewChanged = true;
      currentRow = currentOutput;
    }
    break;

  default:
    break;
  }
}

void CmModel::handleRotary(bool increment)
{

  int8_t modifier = increment ? 1 : -1;

  switch (currentMode)
  {
  case MODE_BPM:
    bpmChange(modifier);
    break;

  case MODE_SWING:
    swingChange(modifier);
    break;

  case MODE_OUTPUT_LIST:
    currentRow = currentRow + modifier;
    if (currentRow == 255)
      currentRow = 0;
    else if (currentRow > 7)
      currentRow = 7;
    break;

  case MODE_OUTPUT_SETTINGS:
    outputSettingsValueChange(modifier);
    break;
  }
}

/***********************************************

  OUTPUT SETTINGS CHANGE

*/

void CmModel::prepareOutputSettingsChange()
{
  currentOutput = currentRow;
  currentRow = 0;
  editType = outputs[currentOutput]->type;
  editClockLength = outputs[currentOutput]->clockLength;
  editGateLength = outputs[currentOutput]->gateLength;
  editStartDelayLength = outputs[currentOutput]->startDelayLength;
  editEuclideanSteps = outputs[currentOutput]->euclideanSteps;
  editSequenceLength = outputs[currentOutput]->sequenceLength;
  editRandomTriggerProbability = outputs[currentOutput]->randomTriggerProbability;
}

void CmModel::outputSettingsValueChange(int8_t modifier)
{
  switch (currentRow)
  {
  case 0:
    /* Type */
    editType = editType + modifier;
    if (editType == 8)
      editType = 7;
    if (editType == 0)
      editType = 1;
    if (currentOutput < 4 && editType > 3)
    {
      editType = 3;
    }
    typeChanged = true;
    break;
  case 1:
    /* Clock */
    editClockLength = editClockLength + modifier;
    if (editClockLength > NUM_CLOCKS)
      editClockLength = NUM_CLOCKS;
    if (editClockLength == 0)
      editClockLength = 1;
    if (editType == VOLTAGE)
      editGateLength = editClockLength;
    break;

  default:
    break;
  }

  if (editType == EUCLIDEAN)
    outputSettingsValueChangeEuclidean(modifier);
  else if (editType == RANDOM_TRIGGERS)
    outputSettingsValueChangeRandomTriggers(modifier);
  else if (editType == VOLTAGE)
    outputSettingsValueChangeVoltage(modifier);
  else
    outputSettingsValueChangeGateSineSaw(modifier);
}

void CmModel::outputSettingsValueChangeEuclidean(int8_t modifier)
{
  switch (currentRow)
  {
  case 2:
    editSequenceLength = editSequenceLength + modifier;
    if (editSequenceLength > MAX_EUCLIDEAN_LENGTH)
      editSequenceLength = MAX_EUCLIDEAN_LENGTH;
    if (editSequenceLength == 0)
      editSequenceLength = 1;
    break;
  case 3:
    editEuclideanSteps = editEuclideanSteps + modifier;
    if (editEuclideanSteps > MAX_EUCLIDEAN_LENGTH)
      editEuclideanSteps = MAX_EUCLIDEAN_LENGTH;
    if (editEuclideanSteps == 0)
      editEuclideanSteps = 1;
    break;
  }
  editSequence = outputs[currentOutput]->generateTemporaryEuclideanSequence(editEuclideanSteps, editSequenceLength);
}

void CmModel::outputSettingsValueChangeRandomTriggers(int8_t modifier)
{
  switch (currentRow)
  {
  case 2:
    editRandomTriggerProbability = editRandomTriggerProbability + modifier * RANDOM_TRIGGER_PROBABILITY_CHANGE_STEP_SIZE;
    if (editRandomTriggerProbability > 250)
      editRandomTriggerProbability = 0;
    if (editRandomTriggerProbability > 100)
      editRandomTriggerProbability = 100;
    break;
  case 3:
    editSequenceLength = editSequenceLength + modifier;
    if (editSequenceLength == 1)
      editSequenceLength = 2;
    if (editSequenceLength > 32)
      editSequenceLength = 32;
    break;
  }
  editSequence = outputs[currentOutput]->generateTemporarySequence(RANDOM_TRIGGERS, editSequenceLength);
  editSequenceB = outputs[currentOutput]->generateTemporarySequence(RANDOM_TRIGGERS, editSequenceLength);
}

void CmModel::outputSettingsValueChangeVoltage(int8_t modifier)
{
  switch (currentRow)
  {
  case 2:
    editSequenceLength = editSequenceLength + modifier;
    if (editSequenceLength == 255)
      editSequenceLength = 0;
    if (editSequenceLength == MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH + 1)
      editSequenceLength = MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH;
    break;
  }
  editSequence = outputs[currentOutput]->generateTemporarySequence(VOLTAGE, editSequenceLength);
}

void CmModel::outputSettingsValueChangeGateSineSaw(int8_t modifier)
{
  switch (currentRow)
  {
  case 2:
    /* Gate */
    editGateLength = editGateLength + modifier;
    if (editGateLength > NUM_CLOCKS)
      editGateLength = NUM_CLOCKS;
    if (editGateLength == 0)
      editGateLength = 1;
    break;
  case 3:
    /* Delay */
    editStartDelayLength = editStartDelayLength + modifier;
    if (editStartDelayLength == 255)
      editStartDelayLength = 0;
    if (editStartDelayLength > NUM_CLOCKS)
      editStartDelayLength = NUM_CLOCKS;

    break;
  }
}

/*
  Commit output settings changes. To be called on 1st beat of a bar, i.e every 4th beat.
*/
void CmModel::commitOutputSettingsChange()
{

  printFreeMem();

  // Set output values
  outputChangesReadyForCommit = false;

  Output *o = outputs[currentOutput];

  if (editType == EUCLIDEAN || editType == RANDOM_TRIGGERS)
    editGateLength = CLOCK_1x128;

  o->setOutputType(editType);
  o->setClockLength(editClockLength);
  o->setGateLength(editGateLength);
  if (editType != VOLTAGE)
    o->setStartDelayLength(editStartDelayLength);
  if (editType == EUCLIDEAN)
  {
    o->setEuclideanSteps(editEuclideanSteps);
  }
  else if (editType == RANDOM_TRIGGERS)
    o->setRandomTriggerProbability(editRandomTriggerProbability);
  if (editType == EUCLIDEAN || editType == VOLTAGE)
  {
    o->setSequence(editSequence);
    o->setSequenceLength(editSequenceLength);
  }
  else if (editType == RANDOM_TRIGGERS)
  {
    o->setSequence(editSequence, editSequenceB);
    o->setSequenceLength(editSequenceLength);
  }

  resetOutput(o);
  renderView = true;
}

/***********************************************

  BPM/SWING CHANGE

*/

void CmModel::bpmChange(int8_t modifier)
{
  BPM = BPM + modifier;
  if (BPM < MIN_BPM)
    BPM = MIN_BPM;
  else if (BPM > MAX_BPM)
    BPM = MAX_BPM;
  CmHardware *hw = CmHardware::getInstance();
  hw->updateOCR1A_limit();
}

void CmModel::swingChange(int8_t modifier)
{
  swing = swing + modifier;
  if (swing == 255)
    swing = 0;
  else if (swing > MAX_SWING)
    swing = MAX_SWING;
  updateSwingTable();
  swingChangeReadyForCommit = true;
  if (!clockRunning)
    commitSwingChange();
}

void CmModel::commitSwingChange()
{
  swingChangeReadyForCommit = false;
  resetOutputs();
  renderView = true;
}
