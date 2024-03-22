#ifndef OUTPUT_H
#define OUTPUT_H

#include <Arduino.h>
#include "Resources.h"

class Output
{
public:
  uint8_t PIN;
  uint8_t ANALOG_PIN;
  uint8_t type;
  uint8_t clockLength;
  uint8_t gateLength;
  uint8_t startDelayLength;
  bool swinging;
  byte pwm_out;
  bool d_out;
  bool gateOpen;
  uint16_t t_gateOpen;
  uint16_t t_gateClose;
  uint16_t pwmPpqnCounter;
  uint8_t event;
  uint16_t eventTime;
  int sequence; /* Common sequence placeholders 		*/
  int sequenceB;
  uint8_t sequenceIndex;  /* for euclid, triggers and     		*/
  uint8_t sequenceLength; /* voltage.                     		*/
  uint8_t euclideanSteps;
  uint8_t randomTriggerProbability;

  Output(uint8_t p, uint8_t a);
  ~Output();

  void reset();
  void setOutputType(uint8_t t);
  void setClockLength(uint8_t c);
  void setGateLength(uint8_t c);
  void setStartDelayLength(uint8_t c);
  void setEuclideanSteps(int k);
  void setRandomTriggerProbability(int p);
  void generateSequence(byte len);
  void setSequence(int s);
  void setSequence(int s, int sB);
  void setSequenceLength(byte len);
  void setDelayedEvent(Event e, EventTime t);
  void setGateCloseEvent(EventTime t);
  void setGateOpenEvent(EventTime t);
  void setSwingGateOpenEvent(EventTime t, uint8_t swing);
  void setPwmEvent(EventTime t);
  void handlePwmEvent(int t);
  void setDefaultGateTimesForSwingable();
  void setDefaultGateTimes();
  int generateEuclideanRhythm(uint8_t k, uint8_t n);
  int generateRandomTriggerSequence(byte probability, byte length);
  int generateTemporarySequence(uint8_t stype, uint8_t len);
  int generateTemporaryEuclideanSequence(uint8_t k, uint8_t n);

private:
  void setEvent(Event e, EventTime t);
  EventTime handleEventTimeOverflow(EventTime t);
  void handleEuclideanGate();
  void handleRandomTriggersGate();
};

#endif
