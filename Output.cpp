/*
   Output (Arduino pin) class for Clock Module

   Handles output clock, gates delays, Euclidean rhythms etc.

*/

#include "Output.h"

#define MAX_LONG 2147483647

Output::Output(uint8_t p, uint8_t a)
{
  PIN = p;
  ANALOG_PIN = a;
  type = NO_OUTPUT;
  clockLength = NO_CLOCK;
  gateLength = NO_CLOCK;
  startDelayLength = NO_CLOCK;
  pwm_out = 0;
  d_out = true;
  swinging = false;
  gateOpen = true;
  t_gateOpen = 0;
  t_gateClose = 0;
  pwmPpqnCounter = 0;
  event = NO_EVENT;
  eventTime = 0;

  // Common sequences
  sequence = 0;
  sequenceB = 0;
  sequenceIndex = 0;
  sequenceLength = 0;

  // Euclidean rhythms
  euclideanSteps = 0;

  // Random triggers
  randomTriggerProbability = 100;
}

Output::~Output() {}

/*
   Reset output
*/
void Output::reset()
{
  event = NO_EVENT;
  eventTime = 0;
  gateOpen = true;
  if (type == EUCLIDEAN)
    sequenceIndex = sequenceLength - 2;
  if (type != VOLTAGE)
    pwm_out = 0;
  swinging = false;
  if (startDelayLength > 0)
    d_out = false;
  else
    d_out = true;

  if (type == SAW || type == SAW_INVERTED || type == SINE || type == VOLTAGE)
  {
    pwmPpqnCounter = 0;
    d_out = false;
    sequenceIndex = 0;
    if (type == VOLTAGE)
    {
      if (sequenceLength > 0)
        pwm_out = sequence >> sequenceIndex & 0xFF;
      else
        pwm_out = random(255) + 1;
    }
  }
  analogWrite(ANALOG_PIN, pwm_out);
}

/*
   Set output type
*/
void Output::setOutputType(uint8_t t)
{
  type = t;

  /*
     typedef enum OutputType {
    NO_OUTPUT       = 0,
    CLOCK           = 1,
    EUCLIDEAN       = 2,
    RANDOM_TRIGGERS = 3,
    SAW             = 4,
    SAW_INVERTED    = 5,
    SINE            = 6,
    VOLTAGE         = 7
  */

  if (type == SAW || type == SAW_INVERTED || type == SINE || type == VOLTAGE)
  {
    d_out = 0;
    pwm_out = 0;
  }
  else
  {
    pwm_out = 0;
  }
}

/**
   Clock length
*/
void Output::setClockLength(uint8_t c)
{
  clockLength = c;
}

/**
   Gate length
*/
void Output::setGateLength(uint8_t c)
{

  if (type == EUCLIDEAN || type == RANDOM_TRIGGERS)
  {
    c = CLOCK_1x256;
  }

  gateLength = c;

  if (type == SAW || type == SAW_INVERTED || type == SINE || type == VOLTAGE)
  {
    pwmPpqnCounter = 0;
  }
}

/**
   Start delay length
*/
void Output::setStartDelayLength(uint8_t c)
{
  startDelayLength = c;
  if (startDelayLength > 0)
    d_out = false;
  else
    d_out = true;
}

/***********************************************************

    EVENTS

*/

/*
   Setting event for GATE_OPEN and GATE_CLOSE
*/
void Output::setEvent(Event e, EventTime t)
{
  event = e;
  eventTime = t;
}

void Output::setDelayedEvent(Event e, EventTime t)
{
  EventTime et = handleEventTimeOverflow(t + CLOCK_LENGTH_TO_PPQN[startDelayLength]);
  setEvent(e, handleEventTimeOverflow(t + CLOCK_LENGTH_TO_PPQN[startDelayLength]));
}

void Output::setGateCloseEvent(EventTime t)
{
  setEvent(GATE_CLOSE, handleEventTimeOverflow(t + t_gateClose));
}

void Output::setGateOpenEvent(EventTime t)
{

  if (type == EUCLIDEAN)
  {
    handleEuclideanGate();
  }
  else if (type == RANDOM_TRIGGERS)
  {
    handleRandomTriggersGate();
  }

  setEvent(GATE_OPEN, handleEventTimeOverflow(t + t_gateOpen));
}

void Output::setSwingGateOpenEvent(EventTime t, uint8_t swing)
{

  if (type == EUCLIDEAN)
  {
    handleEuclideanGate();
  }
  else if (type == RANDOM_TRIGGERS)
  {
    handleRandomTriggersGate();
  }

  if (swinging)
  {
    setEvent(GATE_OPEN, handleEventTimeOverflow(t + t_gateOpen - swing));
  }
  else
  {
    setEvent(GATE_OPEN, handleEventTimeOverflow(t + t_gateOpen + swing));
  }
  swinging = swinging ? false : true;
}

/*
   Handle interruptCounter restart from 0
*/
EventTime Output::handleEventTimeOverflow(EventTime t)
{
  if (t >= INTERRUPT_COUNTER_LIMIT)
  {
    int t_over = t - INTERRUPT_COUNTER_LIMIT;
    return t_over;
  }
  return t;
}

/*
   Analog events
*/
void Output::setPwmEvent(EventTime t)
{
  setEvent(PWM_EVENT, handleEventTimeOverflow(t));
}

void Output::handlePwmEvent(int t)
{

  pwmPpqnCounter++;

  int totalPpqn = pwmPpqnCounter * PWM_EVENT_PPQN;
  int mapped = 0;
  int mappedPpqn = totalPpqn;

  if (type == SINE)
  {

    double totalPpqnInRad = 2 * PI * totalPpqn / CLOCK_LENGTH_TO_PPQN[gateLength];
    double sin_out = cos(totalPpqnInRad); // cosine to let pwm_out start at 0

    int sin_out_int = (int)(sin_out * 255);

    mapped = map(sin_out_int, 255, -255, 0, 255);
    pwm_out = mapped;
  }
  else if (type == SAW || type == SAW_INVERTED)
  {

    if (totalPpqn > CLOCK_LENGTH_TO_PPQN[gateLength])
      mappedPpqn = 0; // Or fullcycleppqn instead of 0 to stay high...
    if (type == SAW_INVERTED)
      mapped = map(mappedPpqn, 0, CLOCK_LENGTH_TO_PPQN[gateLength], 255, 0);
    else
      mapped = map(mappedPpqn, 0, CLOCK_LENGTH_TO_PPQN[gateLength], 0, 255);

    pwm_out = mapped;
  }
  else if (type == VOLTAGE)
  {

    if (totalPpqn >= CLOCK_LENGTH_TO_PPQN[clockLength])
    {
      if (sequenceLength == 0)
      {
        pwm_out = random(255) + 1;
      }
      else
      {
        sequenceIndex++;
        if (sequenceIndex >= sequenceLength)
          sequenceIndex = 0;
        pwm_out = sequence >> sequenceIndex & 0xFF;
      }
    }
  }

  if (totalPpqn >= CLOCK_LENGTH_TO_PPQN[clockLength] - 1)
  {
    pwmPpqnCounter = 0;
  }
}

/***********************************************************

    GATE TIMES

*/

/*
   Default gate times
*/
void Output::setDefaultGateTimes()
{
  t_gateClose = CLOCK_LENGTH_TO_PPQN[gateLength];
  if (CLOCK_LENGTH_TO_PPQN[gateLength] == CLOCK_LENGTH_TO_PPQN[clockLength])
  {
    t_gateClose--;
  }
  t_gateOpen = CLOCK_LENGTH_TO_PPQN[clockLength] - t_gateClose;
}

void Output::setDefaultGateTimesForSwingable()
{
  t_gateOpen = CLOCK_LENGTH_TO_PPQN[clockLength] - CLOCK_LENGTH_TO_PPQN[gateLength];
  t_gateClose = CLOCK_LENGTH_TO_PPQN[gateLength];
}

/***********************************************************

    EUCLIDEAN RHYTHMS

*/

/*
   Euclidean steps.
   k = num of active steps in sequence
*/
void Output::setEuclideanSteps(int k)
{
  this->euclideanSteps = k;
}

/*
   Euclidean rhythm generator.
   Generates Euclidean rhythms using Bresenham algorithm.
*/
int Output::generateEuclideanRhythm(uint8_t k, uint8_t n)
{
  int euclidean = 0;
  double slope = (double)n / (double)k;
  double pattern_index = 0;
  for (int i = 0; i < k; i++)
  {
    int index = (int)ceil(pattern_index);
    int bit_index = n - index - 1;
    euclidean |= 1 << bit_index;
    pattern_index += slope;
  }
  return euclidean;
}

void Output::handleEuclideanGate()
{
  // Open gate only if euclidean step is true
  gateOpen = (sequence >> sequenceIndex) & 1 == 1 ? true : false;
  sequenceIndex -= 1;
  if (sequenceIndex == 255)
  {
    sequenceIndex = sequenceLength + sequenceIndex;
  }
}

/***********************************************************

    COMMON SEQUENCES

*/
void Output::generateSequence(byte len)
{

  this->sequenceLength = len;

  if (type == VOLTAGE)
  {
    if (sequenceLength > MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH)
      sequenceLength = MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH;
    if (sequenceLength > 0)
    {
      sequence = random(MAX_LONG);
      pwm_out = sequence >> sequenceIndex & 0xFF;
      sequenceIndex = 0;
    }
    else
    {
      pwm_out = random(255) + 1;
    }
  }
  else if (type == RANDOM_TRIGGERS)
  {
    sequence = 0;
    sequenceB = 0;
    if (len > 0)
    {
      sequence = generateRandomTriggerSequence(randomTriggerProbability, sequenceLength);
      sequenceB = generateRandomTriggerSequence(randomTriggerProbability, sequenceLength);
    }
  }
  else if (type == EUCLIDEAN)
  {
    sequenceIndex = sequenceLength - 2; // Reading bits... leftmost bit is index n-1, rightmost is 0, first step alreay done
    sequence = generateEuclideanRhythm(euclideanSteps, sequenceLength);
  }
}

int Output::generateTemporarySequence(uint8_t stype, uint8_t len)
{
  int s = 0;
  if (stype == VOLTAGE)
  {
    if (len > MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH)
      len = MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH;
    if (len > 0)
    {
      s = random(MAX_LONG);
    }
  }
  else if (stype == RANDOM_TRIGGERS)
  {
    if (len > 0)
    {
      if (len > 16)
        len = 16;
      s = generateRandomTriggerSequence(randomTriggerProbability, len);
    }
  }
  return s;
}

int Output::generateTemporaryEuclideanSequence(uint8_t k, uint8_t n)
{
  return generateEuclideanRhythm(k, n);
}

void Output::setSequence(int s)
{
  this->sequence = s;
}

void Output::setSequence(int s, int sB)
{
  this->sequence = s;
  this->sequenceB = sB;
}

void Output::setSequenceLength(byte len)
{
  this->sequenceLength = len;
}

/***********************************************************

    RANDOM TRIGGERS

*/
void Output::setRandomTriggerProbability(int p)
{
  randomTriggerProbability = p;
}

int Output::generateRandomTriggerSequence(byte probability, byte length)
{
  // calculates a new random trigger sequence, each step has prob p to trigger
  int s = 0;
  for (int i = 0; i < length /* + 1 */; i++)
  {
    bool trigger = random(100) < probability ? true : false;
    if (trigger)
      s |= 1 << i;
  }
  return s;
}

void Output::handleRandomTriggersGate()
{
  if (sequenceLength == 0)
  {
    gateOpen = random(100) < randomTriggerProbability ? true : false;
  }
  else
  {
    if (sequenceIndex < 16)
    {
      gateOpen = (sequence >> sequenceIndex) & 1 == 1 ? true : false;
    }
    else
    {
      gateOpen = (sequenceB >> sequenceIndex - 16) & 1 == 1 ? true : false;
    }
    sequenceIndex++;
    if (sequenceIndex >= sequenceLength)
    {
      sequenceIndex = 0;
    }
  }
}
