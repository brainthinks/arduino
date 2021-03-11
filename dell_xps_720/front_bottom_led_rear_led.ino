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

// Front Bottom LEDs
// const byte DPIN_FB_PWR = ???; // 5v
const byte DPIN_FB_RED   = 5; // ground
const byte DPIN_FB_GREEN = 6; // ground
const byte DPIN_FB_BLUE  = 7; // ground

// Rear LEDs
// const byte DPIN_RR_PWR = ???; // 5v
const byte DPIN_RR_RED   = 5; // ground
const byte DPIN_RR_GREEN = 6; // ground
const byte DPIN_RR_BLUE  = 7; // ground


