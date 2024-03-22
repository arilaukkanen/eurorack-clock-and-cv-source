/*
   Enums etc. used in Clock Module

*/
#ifndef CMRESOURCES_H
#define CMRESOURCES_H

/***
  General
*/
#define BUTTON_PRESS_THRESHOLD 50
#define BUTTON_LONG_PRESS_MILLIS 300
#define SCREENSAVER_ACTIVATION_MILLIS 900000
#define SCREENSAVER_UPDATE_MILLIS 60000

/***
   Settings limits
*/
#define MAX_BPM 200
#define MIN_BPM 30
#define MAX_SWING 30
#define MAX_EUCLIDEAN_LENGTH 32
#define MAX_RANDOM_VOLTAGE_SEQUENCE_LENGTH 32
#define NUM_CLOCKS 21
#define NUM_TYPES 8
#define CLOCK_LENGTH_SWINGABLE_LIMIT 6
#define NUM_OUTPUTS 8

/***
   Settings defaults
*/
#define DEFAULT_BPM 100
#define DEFAULT_SWING 0
#define DEFAULT_SEQUENCE_LENGTH 8
#define DEFAULT_EUCLIDEAN_STEPS 5
#define DEFAULT_RANDOM_TRIGGER_PROBABILITY 50

/***
   Pins
*/
#define NO_ANALOG_OUTPUT 255
#define PIN_OUTPUT0 12
#define PIN_OUTPUT1 8
#define PIN_OUTPUT2 4
#define PIN_OUTPUT3 A1
#define PIN_OUTPUT4 A2
#define PIN_OUTPUT5 A3
#define PIN_OUTPUT6 A4
#define PIN_OUTPUT7 A5
#define PIN_ANALOG4 5
#define PIN_ANALOG5 6
#define PIN_ANALOG6 11
#define PIN_ANALOG7 13
#define CLOCK_INPUT 9

/***
   Debug flags
*/
#define DEBUG_INTERRUPT false
#define DEBUG_VIEW false
#define DEBUG_INTERRUPT_DIVIDER 6

/*
   Timing constants
*/

#define PPQN 192
const uint16_t PROGMEM INTERRUPT_COUNTER_LIMIT = PPQN * 4 * 64;
const uint8_t PROGMEM PWM_EVENT_PPQN = 6;
const uint16_t PROGMEM PPQN_BAR = PPQN * 4;
static const char *SPACE = " ";
static const char *SPACE2 = "  ";
static const char *ROW_INDICATOR = ">";
static const char *ROW_INDICATOR_COMMIT = "!";
static const char *CHAR_L = "L";
static const char *CHAR_N = "n";
static const char *CHAR_K = "k";
static const char *CHAR_P = "p";
static const char *ROW_TYPE = "Type      ";
static const char *ROW_CLOCK = "Clock     ";
static const char *ROW_GATE = "Gate      ";
static const char *ROW_DELAY = "Delay     ";
static const char *ROW_STEPS = "Steps     ";
static const char *ROW_LENGTH = "Length    ";
static const char *ROW_PROB = "Prob      ";
static const char *ROW_SEQUENCE = "Sequence  ";

/*******************************************************************

   Types & conversion tables

*/

const uint16_t CLOCK_LENGTH_TO_PPQN[] = {
    /* 0  NO_DELAY*/ 0,
    /* 1  1x256   */ PPQN >> 6,
    /* 2  1x128   */ PPQN >> 5,
    /* 3  1x64    */ PPQN >> 4,
    /* 4  1x32    */ PPQN >> 3,
    /* 5  1x16    */ PPQN >> 2,
    /* 6  1x16D   */ (PPQN >> 2) + (PPQN >> 3),
    /* 7  1x8     */ PPQN >> 1,
    /* 8  1x8D    */ (PPQN >> 1) + (PPQN >> 2),
    /* 9  1x4     */ PPQN >> 0,
    /* 10  1x4D   */ (PPQN >> 0) + (PPQN >> 1),
    /* 11  1x2    */ PPQN << 1,
    /* 12  1x2D   */ (PPQN << 1) + (PPQN >> 0),
    /* 13  1x1    */ PPQN << 2,
    /* 14  1x1D   */ (PPQN << 2) + (PPQN << 1),
    /* 15 2x1     */ PPQN << 3,
    /* 16 3x1     */ (PPQN << 3) + (PPQN << 2),
    /* 17 4x1     */ PPQN << 4,
    /* 18 6x1     */ (PPQN << 4) + (PPQN << 3),
    /* 19 8x1     */ PPQN >> 5,
    /* 20 12x1    */ (PPQN >> 5) + (PPQN >> 4),
    /* 21 16x1    */ PPQN >> 6,
    /* 22 24x1    */ (PPQN >> 6) + (PPQN >> 5),
    /* 23 32x1    */ PPQN >> 7,
    /* 24 48x1    */ (PPQN >> 7) + (PPQN >> 6),
    /* 25 64x1    */ PPQN >> 8

};

typedef enum Event
{
  NO_EVENT = 0,
  GATE_OPEN = 1,
  GATE_CLOSE = 2,
  PWM_EVENT = 3
};

typedef int EventTime;

typedef enum OutputType
{
  NO_OUTPUT = 0,
  CLOCK = 1,
  EUCLIDEAN = 2,
  RANDOM_TRIGGERS = 3,
  SAW = 4,
  SAW_INVERTED = 5,
  SINE = 6,
  VOLTAGE = 7
};

static const char *TYPE_TO_STR[] = {
    "----",
    "Gate",
    "Eucl",
    "Trig",
    "SawR",
    "SawF",
    "Sine",
    "Volt"};

static const char *TYPE_TO_LONG_STR[] = {
    "-",
    "Gate",
    "Euclidean",
    "Triggers",
    "Saw/rise",
    "Saw\\fall",
    "Sine",
    "Voltages"};

typedef enum ClockLength
{
  NO_CLOCK = 0,
  CLOCK_1x256 = 1,
  CLOCK_1x128 = 2,
  CLOCK_1x64 = 3,
  CLOCK_1x32 = 4,
  CLOCK_1x16 = 5,
  CLOCK_1x16D = 6,
  CLOCK_1x8 = 7,
  CLOCK_1x8D = 8,
  CLOCK_1x4 = 9,
  CLOCK_1x4D = 10,
  CLOCK_1x2 = 11,
  CLOCK_1x2D = 12,
  CLOCK_1x1 = 13,
  CLOCK_1x1D = 14,
  CLOCK_2x1 = 15,
  CLOCK_3x1 = 16,
  CLOCK_4x1 = 17,
  CLOCK_6x1 = 18,
  CLOCK_8x1 = 19,
  CLOCK_12x1 = 20,
  CLOCK_16x1 = 21
};

static const char *CLOCK_TO_STR[] = {
    "     ",
    " 256 ",
    " 128 ",
    "  64 ",
    "  32 ",
    "  16 ",
    "  16.",
    "   8 ",
    "   8.",
    "   4 ",
    "   4.",
    "   2 ",
    "   2.",
    "   1 ",
    "  1x.",
    "  2x ",
    "  3x ",
    "  4x ",
    "  6x ",
    "  8x ",
    " 12x ",
    " 16x "
    //  " 24x "
    //  " 32x "
    //  " 48x ",
    //  " 64x "
};

static const char *CLOCK_TO_LONG_STR[] = {
    "-",
    "1/256",
    "1/128",
    "1/64",
    "1/32",
    "1/16",
    "1/16.",
    "1/8",
    "1/8.",
    "1/4",
    "1/4.",
    "1/2",
    "1/2.",
    "1/1",
    "1/1.",
    "2/1",
    "3/1",
    "4/1",
    "6/1",
    "8/1",
    "12/1",
    "16/1"
    //  "24/1"
    //  "32/1"
    //  "48/1",
    //  "64/1"
};

typedef enum Mode
{
  MODE_BPM = 0,
  MODE_SWING = 1,
  MODE_OUTPUT_LIST = 2,
  MODE_OUTPUT_SETTINGS = 3
};

/*
   Memory usage debug tool
*/
// #ifdef __arm__
//// should use uinstd.h to define sbrk but Due causes a conflict
// extern "C" char* sbrk(int incr);
// #else  // __ARM__
// extern char *__brkval;
// #endif  // __arm__
//
// static int freeMemory() {
//   char top;
// #ifdef __arm__
//   return &top - reinterpret_cast<char*>(sbrk(0));
// #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
//   return &top - __brkval;
// #else  // __arm__
//   return __brkval ? &top - __brkval : &top - __malloc_heap_start;
// #endif  // __arm__
// }

static void printFreeMem()
{
  //  Serial.print(F("Free mem: "));
  //  Serial.print(freeMemory());
  //  Serial.println(F(" bytes"));
}

#endif
