#include <Arduino.h>
#include "CmHardware.h"

#include <avr/wdt.h>

#define CPU_FREQ 16000000
#define PRESCALER 8
#define RUN_BUTTON_PIN A0
#define BUTTON_PIN 7
#define RANDOM_SEED_PIN 9
#define ROTARY_A_PIN 0
#define ROTARY_B_PIN 1

/*

  0       Rotary A
  1       Rotary B
  2       OLED
  3  PWM  OLED
  4       output 2
  5  PWM  pwm output TOIMII
  6  PWM  pwm output TOIMII
  7       button
  8       output 1
  9  PWM
  10 PWM
  11 PWM  pwm output
  12      output 0
  13 PWM  pwm output
  A0      run button
  A1      output 3
  A2      output 4
  A3      output 5
  A4      output 6
  A5      output 7

*/

CmHardware::CmHardware()
{
}

void CmHardware::initialize()
{
  pinMode(RUN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ROTARY_A_PIN, INPUT_PULLUP);
  pinMode(ROTARY_B_PIN, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(CLOCK_INPUT, INPUT);
  randomSeed(RANDOM_SEED_PIN);
  resetOutputPins();
  updateOCR1A_limit();
  noInterrupts();
  TCCR1A = 0; // Clear TIMER1 registers.
  TCCR1B = 0;
  TIFR1 = 0;
  TIMSK1 = 0;
  OCR1A = (int)OCR1A_limit; // Set counter limit
  TCCR1B |= (1 << WGM12);   // Set time compare CTC 4 mode on.
  TCCR1B |= (1 << CS11);    // Set prescaler to 8
  TIMSK1 |= (1 << OCIE1A);  // Enable counter output compare interrupt.
  interrupts();
  splashFlash();
}

void CmHardware::splashFlash()
{
  view->displaySplashScreen();

  /*
     12
     8
     4
     A1..A5

  */

  uint8_t pins[] = {12, 8, 4, A1, A2, A3, A4, A5};

  for (uint8_t num = 0; num < 3; num++)
  {
    for (uint8_t i : pins)
    {
      digitalWrite(i, HIGH);
      delay(50);
    }
    for (uint8_t i : pins)
    {
      digitalWrite(i, LOW);
      delay(50);
    }
  }
  delay(1000);
  view->render();
}

void CmHardware::updateOCR1A_limit()
{

  uint8_t &BPM = model->BPM;

  double iHz = (double)BPM * (double)PPQN * (double)0.0166666666666666666667;
  OCR1A_limit = (double)CPU_FREQ / (double)PRESCALER / iHz;

  noInterrupts();
  OCR1A = (int)OCR1A_limit;
  interrupts();
}

/********************************************************************

       MAIN CONTROLLER LOOP

*/
void CmHardware::runModule()
{
  Serial.println("CmHardware::runModule()");

  /* Watchdog: Reset module if code hangs up for 2 seconds */
  wdt_enable(WDTO_2S);

  while (true)
  {

    wdt_reset();

    /*
       Start/stop button
    */
    bool runButtonState = !digitalRead(RUN_BUTTON_PIN);

    if (!runButtonState && model->clockRunning)
    {
      model->clockStopped();
      model->clockRunning = runButtonState;
      resetOutputPins();
    }

    bool clockInputState = digitalRead(CLOCK_INPUT);

    if (clockInputState)
    {
      model->clockRunning = runButtonState;
    }

    /*
       Rotary rotation
    */
    stateRotaryA = (stateRotaryA << 1) | digitalRead(ROTARY_A_PIN) | 0xe000;
    if (stateRotaryA == 0xf000)
    {

      stateRotaryA = 0x0000;
      volatile CmModel *model = CmModel::getInstance();
      volatile CmView *view = CmView::getInstance();
      bool rotaryDirection = LOW;
      if (digitalRead(ROTARY_B_PIN))
        rotaryDirection = HIGH;

      if (screensaver)
      {
        stopScreensaver();
      }
      else
      {
        model->handleRotary(rotaryDirection);
        view->render();
      }
      lastControlMillis = millis();
    }

    /*
       Rotary button
    */
    bool buttonState = digitalRead(BUTTON_PIN);
    if (!buttonState && buttonStatePrev)
    {
      /* Button pressed */
      uint32_t now = millis();
      if (now - lastButtonPressMillis > BUTTON_PRESS_THRESHOLD)
      {
        buttonDownMillis = now;
      }
    }
    if (buttonState && !buttonStatePrev)
    {
      /* Button released */
      uint32_t now = millis();
      if (screensaver)
      {
        stopScreensaver();
      }
      else
      {

        if (now - buttonDownMillis > BUTTON_LONG_PRESS_MILLIS)
        {
          model->handleButtonLongPress();
        }
        else if (now - lastButtonPressMillis > BUTTON_PRESS_THRESHOLD)
        {
          model->handleButton();
          lastButtonPressMillis = now;
        }
        view->render();
      }
      lastControlMillis = millis();
    }
    buttonStatePrev = buttonState;

    /*
       Check if view rendering is needed
    */
    if (model->renderView)
    {
      view->render();
      model->renderView = false;
    }

    /*
       Screensaver
    */
    if (millis() - lastControlMillis > SCREENSAVER_ACTIVATION_MILLIS)
    {
      screensaver = true;
      if (millis() % SCREENSAVER_UPDATE_MILLIS == 0)
      {
        view->displayScreensaver();
      }
    }
  }
}

void CmHardware::stopScreensaver()
{
  screensaver = false;
  model->viewChanged = true;
  view->render();
}

/********************************************************************

       TIMER INTERRUPT HANDLER

*/

ISR(TIMER1_COMPA_vect)
{

  volatile CmModel *model = CmModel::getInstance();

  if (!model->clockRunning)
    return;

  volatile int &i_c = model->interruptCounter;

  /*
     PIN 12: PORTD PD6
     PIN 8:  PORTB PB4
     PIN 4:  PORTD PD4
     PIN A1: PORTF PF6
  */

  /*
      Update output states
  */
  model->outputs[0]->d_out ? PORTD |= (1 << PD6) : PORTD &= ~(1 << PD6);
  model->outputs[1]->d_out ? PORTB |= (1 << PB4) : PORTB &= ~(1 << PB4);
  model->outputs[2]->d_out ? PORTD |= (1 << PD4) : PORTD &= ~(1 << PD4);
  model->outputs[3]->d_out ? PORTF |= (1 << PF6) : PORTF &= ~(1 << PF6); // A1
  model->outputs[4]->d_out ? PORTF |= (1 << PF5) : PORTF &= ~(1 << PF5); // A2
  model->outputs[5]->d_out ? PORTF |= (1 << PF4) : PORTF &= ~(1 << PF4); // A3
  model->outputs[6]->d_out ? PORTF |= (1 << PF1) : PORTF &= ~(1 << PF1); // A4
  model->outputs[7]->d_out ? PORTF |= (1 << PF0) : PORTF &= ~(1 << PF0); // A5

  /* A1, A2, A3, A4, A5 */
  /* etc rest of pins */

  if (DEBUG_INTERRUPT)
  {
    if (i_c % DEBUG_INTERRUPT_DIVIDER == 0)
    {
      Serial.print(F(" Int "));
      Serial.print(i_c);
      Serial.print(F("   \t : "));
      for (Output *o : model->outputs)
      {
        if (o->type == SAW || o->type == SAW_INVERTED || o->type == SINE || o->type == VOLTAGE)
          Serial.print(o->pwm_out);
        else
          Serial.print(o->d_out ? F("X    ") : F("-    "));
      }
      Serial.println();
    }
  }

  /*
     Increment PPQN/interrupt counter
  */
  if (++i_c == INTERRUPT_COUNTER_LIMIT)
    i_c = 0;

  /*
    Commit output/swing changes at start of a bar
  */
  if (model->outputChangesReadyForCommit && i_c % PPQN_BAR == 0)
  {
    model->commitOutputSettingsChange();
  }
  if (model->swingChangeReadyForCommit && i_c % PPQN_BAR == 0)
  {
    model->commitSwingChange();
  }

  /*
     Calculate new gate values for next cycle
  */
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {

    Output *o = model->outputs[i];

    if (i_c == o->eventTime)
    {

      switch (o->event)
      {

      case GATE_CLOSE:
        o->d_out = false;
        o->pwm_out = 0;
        if (o->clockLength < CLOCK_LENGTH_SWINGABLE_LIMIT)
        {
          uint8_t swing = model->swingTable[o->clockLength];
          o->setSwingGateOpenEvent(i_c, swing);
        }
        else
          o->setGateOpenEvent(i_c);
        break;
      case GATE_OPEN:
        o->pwm_out = 0;
        if (o->gateOpen)
          o->d_out = true;
        o->setGateCloseEvent(i_c);
        break;
      case PWM_EVENT:
        analogWrite(o->ANALOG_PIN, o->pwm_out);
        o->handlePwmEvent(i_c);
        o->setPwmEvent(i_c + PWM_EVENT_PPQN);
        break;
      }
    }
  }
}
