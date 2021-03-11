/**
 * Controller for Dell XPS 720 Front LEDs
 *
 * Author: Brian Andress
 * Site: https://github.com/brainthinks
 */

#include <EEPROM.h>

// -----------------------------------------------------------------------------
//
// DATA
//
// -----------------------------------------------------------------------------

/**
 * Dell Wires --> Arduino Pins
 */

// Front Top LEDs
const byte DPIN_TOP_PWR   = 4; // 5v
const byte DPIN_TOP_RED   = 5; // ground
const byte DPIN_TOP_GREEN = 6; // ground
const byte DPIN_TOP_BLUE  = 7; // ground

// Front Bottom LEDs
const byte DPIN_BOT_PWR   = 8;  // 5v
const byte DPIN_BOT_RED   = 9;  // ground
const byte DPIN_BOT_GREEN = 10; // ground
const byte DPIN_BOT_BLUE  = 11; // ground


/**
 * EEPROM Addresses
 */
const int ADDRESS_STATE = 0; // current LED state
const int ADDRESS_CYCLE = 1; // cycle LED enabled


/**
 * LED State
 *
 * The variable names are the color names Dell used in the bios configuration.
 * The values are integers represent the configuration index.
 *
 * I have kept them in the order Dell listed them in the bios (except NONE).
 */
const int LED_NONE     = 0;
const int LED_RUBY     = 1;
const int LED_EMERALD  = 2;
const int LED_AMBER    = 3;
const int LED_SAPPHIRE = 4;
const int LED_AMETHYST = 5;
const int LED_TOPAZ    = 6;
const int LED_DIAMOND  = 7;

const int LED_FIRST = LED_RUBY;
const int LED_LAST  = LED_DIAMOND;

const int LED_CYCLE_DELAY = 2000;


/**
 * LED Configurations
 *
 * Each configuration contains the state of each of the 3 configurable wires
 * necessary to generate a color on Dell's board.
 *
 * 0 - red color state
 * 1 - green color state
 * 2 - blue color state
 * 3 - state indicator
 */
const int LED_CONFIGURATIONS[8][4] = {
  { HIGH, HIGH, HIGH, LED_NONE     },
  { LOW,  HIGH, HIGH, LED_RUBY     },
  { HIGH, LOW,  HIGH, LED_EMERALD  },
  { LOW,  LOW,  HIGH, LED_AMBER    },
  { HIGH, HIGH, LOW,  LED_SAPPHIRE },
  { LOW,  HIGH, LOW,  LED_AMETHYST },
  { HIGH, LOW,  LOW,  LED_TOPAZ    },
  { LOW,  LOW,  LOW,  LED_DIAMOND  }
};


// -----------------------------------------------------------------------------
//
// LED State
//
// -----------------------------------------------------------------------------

/**
 * Set the state, which controls the LED color
 */
void setState(const int state, const bool shouldDisplayState) {
  // If we encounter an unrecognized state
  if (state < LED_NONE || state > LED_LAST) {
    setState(LED_NONE, shouldDisplayState);
    return 0;
  }

  // Configure the LED color
  digitalWrite(DPIN_TOP_RED,  LED_CONFIGURATIONS[state][0]);
  digitalWrite(DPIN_BOT_RED,  LED_CONFIGURATIONS[state][0]);
  digitalWrite(DPIN_TOP_GREEN, LED_CONFIGURATIONS[state][1]);
  digitalWrite(DPIN_BOT_GREEN, LED_CONFIGURATIONS[state][1]);
  digitalWrite(DPIN_TOP_BLUE, LED_CONFIGURATIONS[state][2]);
  digitalWrite(DPIN_BOT_BLUE, LED_CONFIGURATIONS[state][2]);

  // Save the current LED color state
   EEPROM.write(ADDRESS_STATE, LED_CONFIGURATIONS[state][3]);
}

/**
 * Read the current state from the pin that is broadcasting it
 */
int readState() {
  return EEPROM.read(ADDRESS_STATE);
}

void cycleState() {
  volatile int state = readState();

  // Ensure none is skipped when cycling
  if (state == LED_LAST) {
    state = LED_NONE;
  }

  // Increment the LED state to show the next color
  state++;

  setState(state, false);
  delay(LED_CYCLE_DELAY);
}

// -----------------------------------------------------------------------------
//
// Arduino
//
// -----------------------------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Front Top LEDs
  pinMode(DPIN_TOP_PWR, OUTPUT);
  pinMode(DPIN_TOP_RED, OUTPUT);
  pinMode(DPIN_TOP_GREEN, OUTPUT);
  pinMode(DPIN_TOP_BLUE, OUTPUT);

  // Front Bottom LEDs
  pinMode(DPIN_BOT_PWR, OUTPUT);
  pinMode(DPIN_BOT_RED, OUTPUT);
  pinMode(DPIN_BOT_GREEN, OUTPUT);
  pinMode(DPIN_BOT_BLUE, OUTPUT);

  digitalWrite(DPIN_TOP_PWR, HIGH);
  digitalWrite(DPIN_BOT_PWR, HIGH);
}

void loop() {
  cycleState();
}

