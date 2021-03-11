/**
 * Controller for Dell XPS 720 LEDs
 *
 * Author: Brian Andress
 * Site: https://github.com/brainthinks
 */


#include <EEPROM.h>

const bool DEBUG = false;

// -----------------------------------------------------------------------------
//
// DATA
//
// -----------------------------------------------------------------------------

/**
 * Dell Wires --> Arduino Pins
 */

// Controller board LEDs
const byte DPIN_CB_DLED1   = 5; // ground
const byte DPIN_CB_DLED2   = 6; // ground
const byte DPIN_CB_DLED3   = 7; // ground
const byte DPIN_CB_DLED4   = 8; // ground
const byte DPIN_CB_LAN_LED = 9; // ground
// const byte DPIN_CB_LED_PWR = ??? // 3.3v

// Front Top LEDs
// const byte DPIN_FT_PWR = ???; // 5v
const byte DPIN_FT_RED   = 10; // ground
const byte DPIN_FT_GREEN = 11; // ground
const byte DPIN_FT_BLUE  = 12; // ground


/**
 * Button
 *
 * The physical switch that will allow you to cycle through the different LED states
 */
const byte DPIN_BUTTON_INTERRUPT = 2; // one switch wire
const byte DPIN_BUTTON_GROUND = 3;    // the other switch wire

volatile bool acceptPushedChanges = false;
volatile bool pushed = false;

const int ACCEPT_PUSHED_CHANGE_DELAY = 1000;
const int DEBOUNCE_DELAY = 100;
const int PUSHED_RESET_DELAY = 500;


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

const int LED_CYCLE_DELAY = 1000;



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
// Utilities
//
// -----------------------------------------------------------------------------

void blink(const int number) {
  for (int i = 0; i < number; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}


// -----------------------------------------------------------------------------
//
// LED State
//
// -----------------------------------------------------------------------------

/**
 * Read the current state from the pin that is broadcasting it
 */
int readState() {
  return EEPROM.read(ADDRESS_STATE);
}

/**
 * Blink the built in LED the number of times that corresponds
 * with the currently selected state.
 */
void displayState() {
  blink(readState());
  digitalWrite(LED_BUILTIN, LOW);
}

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
  digitalWrite(DPIN_FT_RED,  LED_CONFIGURATIONS[state][0]);
  digitalWrite(DPIN_FT_GREEN, LED_CONFIGURATIONS[state][1]);
  digitalWrite(DPIN_FT_BLUE, LED_CONFIGURATIONS[state][2]);

  // Save the current LED color state
  EEPROM.write(ADDRESS_STATE, LED_CONFIGURATIONS[state][3]);

  if (DEBUG == true && shouldDisplayState) {
    displayState();
  }
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

bool isCycling() {
  return EEPROM.read(ADDRESS_CYCLE) == true;
}

void enableCycling() {
  EEPROM.write(ADDRESS_CYCLE, true);
}

void disableCycling() {
  EEPROM.write(ADDRESS_CYCLE, false);
}

void incrementState() {
  // If the cycle is enabled, disable it
  if (isCycling()) {
    disableCycling();
    setState(LED_NONE, true);
  }

  // If the LED state is on the last one, enable the cycle
  else if (readState() == LED_LAST) {
    enableCycling();
  }

  // Otherwise, increment the LED state
  else {
    setState(readState() + 1, true);
  }
}


// -----------------------------------------------------------------------------
//
// Button State
//
// -----------------------------------------------------------------------------

void initializeButtonPins(const byte interruptPin, const byte groundPin) {
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPushed, FALLING);
  pinMode(groundPin, OUTPUT);
  digitalWrite(groundPin, LOW);
}

void buttonPushed() {
  if (pushed == false) {
    pushed = true;

    if (acceptPushedChanges || millis() > ACCEPT_PUSHED_CHANGE_DELAY) {
      acceptPushedChanges = true;
      incrementState();
    }
  }
}

void buttonPushedReset() {
  delay(DEBOUNCE_DELAY);
  if (pushed == true) {
    delay(PUSHED_RESET_DELAY);
    pushed = false;
  }
}


// -----------------------------------------------------------------------------
//
// Arduino
//
// -----------------------------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Controller Board
  pinMode(DPIN_CB_DLED1, OUTPUT);
  pinMode(DPIN_CB_DLED2, OUTPUT);
  pinMode(DPIN_CB_DLED3, OUTPUT);
  pinMode(DPIN_CB_DLED4, OUTPUT);
  pinMode(DPIN_CB_LAN_LED, OUTPUT);

  digitalWrite(DPIN_CB_DLED1, LOW);
  digitalWrite(DPIN_CB_DLED2, LOW);
  digitalWrite(DPIN_CB_DLED3, LOW);
  digitalWrite(DPIN_CB_DLED4, LOW);
  digitalWrite(DPIN_CB_LAN_LED, LOW);

  // Front Top LEDs
  pinMode(DPIN_FT_RED, OUTPUT);
  pinMode(DPIN_FT_GREEN, OUTPUT);
  pinMode(DPIN_FT_BLUE, OUTPUT);

  initializeButtonPins(DPIN_BUTTON_INTERRUPT, DPIN_BUTTON_GROUND);

  // Display the LED color that was set last time
  setState(readState(), true);

//  digitalWrite(DPIN_FT_RED, LOW);
//  digitalWrite(DPIN_FT_GREEN, LOW);
//  digitalWrite(DPIN_FT_BLUE, LOW);
}

void loop() {
  if (isCycling()) {
    cycleState();
  }

  buttonPushedReset();
}
