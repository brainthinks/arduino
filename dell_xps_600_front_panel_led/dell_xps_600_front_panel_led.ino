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

/**
 * Hardware Constants
 */
// The Dell board wire colors
int DPIN_WIRE_RED   = 2; // 5V, always on
int DPIN_WIRE_BLUE  = 3; // 3.3V, configurable
int DPIN_WIRE_WHITE = -1; // not used
int DPIN_WIRE_GREEN = 4; // 3.3V, configurable
int DPIN_WIRE_BROWN = 5; // 3.3V, configurable
int DPIN_WIRE_BLACK = 6; // ground

// The physical switch that will allow you to cycle through the different LED states
int DPIN_BUTTON_SENSOR = 8; // one switch wire
int DPIN_BUTTON_GROUND = 9; // the other switch wire

int BUTTON_IS_PRESSED = LOW;

// The EEPROM address that will contain the current LED state
int ADDRESS_STATE = 0;

/**
 * LED State Constants
 * 
 * The variable names are the color names Dell used in the bios configuration.
 * The values are integers represent the configuration index.
 * 
 * I have kept them in the order Dell listed them in the bios (except NONE).
 */
int LED_NONE     = 0;
int LED_RUBY     = 1;
int LED_EMERALD  = 2;
int LED_AMBER    = 3;
int LED_SAPPHIRE = 4;
int LED_AMETHYST = 5;
int LED_TOPAZ    = 6;
int LED_DIAMOND  = 7;

int LED_FIRST = LED_NONE;
int LED_LAST = LED_DIAMOND;

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
int LED_CONFIGURATIONS[8][4] = {
  { LOW,  LOW,  LOW,  LED_NONE     },
  { LOW,  LOW,  HIGH, LED_RUBY     },
  { LOW,  HIGH, LOW,  LED_EMERALD  },
  { LOW,  HIGH, HIGH, LED_AMBER    },
  { HIGH, LOW,  LOW,  LED_SAPPHIRE },
  { HIGH, LOW,  HIGH, LED_AMETHYST },
  { HIGH, HIGH, LOW,  LED_TOPAZ    },
  { HIGH, HIGH, HIGH, LED_DIAMOND  }
};

/**
 * Read the current state from the pin that is broadcasting it
 */
int readState() {
  return EEPROM.read(0);
}

/**
 * Set the state, which controls the LED color
 */
void setState(int state) {
  // If we encounter an unrecognized state
  if (state < LED_FIRST || state > LED_LAST) {
    setState(LED_FIRST);
    return 0;
  }

  // Configure the LED color
  digitalWrite(DPIN_WIRE_BLUE,  LED_CONFIGURATIONS[state][0]);
  digitalWrite(DPIN_WIRE_GREEN, LED_CONFIGURATIONS[state][1]);
  digitalWrite(DPIN_WIRE_BROWN, LED_CONFIGURATIONS[state][2]);

  // Save the current LED color state
  EEPROM.write(ADDRESS_STATE, LED_CONFIGURATIONS[state][3]);

  displayState();
}

/**
 * Blink the built in LED the number of times that corresponds
 * with the currently selected state.
 */
void displayState() {
  int state = readState();
  
  for (int i = 0; i < state; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(DPIN_WIRE_RED, OUTPUT);
  pinMode(DPIN_WIRE_BLUE, OUTPUT);
  pinMode(DPIN_WIRE_GREEN, OUTPUT);
  pinMode(DPIN_WIRE_BROWN, OUTPUT);
  pinMode(DPIN_WIRE_BLACK, OUTPUT);

  // Push button
  pinMode(DPIN_BUTTON_SENSOR, INPUT_PULLUP);
  pinMode(DPIN_BUTTON_GROUND, OUTPUT);
  digitalWrite(DPIN_BUTTON_GROUND, LOW);
  
  // Board power or something...
  digitalWrite(DPIN_WIRE_RED, HIGH);
  digitalWrite(DPIN_WIRE_BLACK, LOW);

  // Display the LED color that was set last time
  setState(readState());
}

void loop() {
  if (digitalRead(DPIN_BUTTON_SENSOR) == BUTTON_IS_PRESSED) {
    setState(readState() + 1);
    delay(1000);
  }
}
