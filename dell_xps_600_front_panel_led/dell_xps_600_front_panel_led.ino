/**
 * Controller for Dell XPS 600 Front Panel LED
 *
 * Author: Brian Andress
 * Site: https://github.com/brainthinks
 *
 * Cycle through the different colors of Dell's front panel LED for the XPS 600, and
 * remember the most recently set color, even between power cycles.
 *
 * My father used to have a Dell Dimension XPS, which is a giant blue computer with a
 * proprietary motherboard layout and other components.  For his birthday in 2018, I
 * wanted to give him a modern computer in that old case because it was so unique.  I
 * quickly learned that that would be a difficult task.  I had to modify the case to
 * accept a standard ATX motherboard, and I had to do some custom wiring to get the
 * front panel LED and the power button board to work.  The front panel LED was more
 * difficult than the power button because the board for the front panel LED accepts
 * multiple power sources, and the state of those power sources controls what color
 * the LED displays.  Additionally, the mechanism to control this board was contained
 * in the motherboard, which is not something that I could leave in the case.  In order
 * to control the color of Dell's front panel LED for this particular case, I wrote
 * this arduino script.
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
const byte DPIN_WIRE_RED   = 5; // 5V, always on
const byte DPIN_WIRE_WHITE = 6; // not used
const byte DPIN_WIRE_BLACK = 7; // ground

const byte DPIN_WIRE_BLUE  = 8; // 3.3V, configurable
const byte DPIN_WIRE_GREEN = 9; // 3.3V, configurable
const byte DPIN_WIRE_BROWN = 10; // 3.3V, configurable

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
 * 0 - blue wire state
 * 1 - green wire state
 * 2 - brown wire state
 * 3 - state indicator
 */
const int LED_CONFIGURATIONS[8][4] = {
  { LOW,  LOW,  LOW,  LED_NONE     },
  { LOW,  LOW,  HIGH, LED_RUBY     },
  { LOW,  HIGH, LOW,  LED_EMERALD  },
  { LOW,  HIGH, HIGH, LED_AMBER    },
  { HIGH, LOW,  LOW,  LED_SAPPHIRE },
  { HIGH, LOW,  HIGH, LED_AMETHYST },
  { HIGH, HIGH, LOW,  LED_TOPAZ    },
  { HIGH, HIGH, HIGH, LED_DIAMOND  }
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
  digitalWrite(DPIN_WIRE_BLUE,  LED_CONFIGURATIONS[state][0]);
  digitalWrite(DPIN_WIRE_GREEN, LED_CONFIGURATIONS[state][1]);
  digitalWrite(DPIN_WIRE_BROWN, LED_CONFIGURATIONS[state][2]);

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

  pinMode(DPIN_WIRE_RED, OUTPUT);
  pinMode(DPIN_WIRE_BLUE, OUTPUT);
  pinMode(DPIN_WIRE_GREEN, OUTPUT);
  pinMode(DPIN_WIRE_BROWN, OUTPUT);
  pinMode(DPIN_WIRE_BLACK, OUTPUT);

  initializeButtonPins(DPIN_BUTTON_INTERRUPT, DPIN_BUTTON_GROUND);

  // Board power or something...
  digitalWrite(DPIN_WIRE_RED, HIGH);
  digitalWrite(DPIN_WIRE_BLACK, LOW);

  // Display the LED color that was set last time
  setState(readState(), true);
}

void loop() {
  if (isCycling()) {
    cycleState();
  }

  buttonPushedReset();
}
