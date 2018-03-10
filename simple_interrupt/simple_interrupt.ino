/**
 * Used with a simple computer case switch.
 *
 * I could not find a good simple tutorial on how to use a switch to trigger an
 * interrupt, so I wrote my own.  This is based on:
 *
 * https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 *
 * but no longer resembles it.
 *
 * Note that you can trigger two state changes with one button press if you hold
 * the button down :(
 *
 * @todo - implement the debounce logic from the debounce tutorial
 */


// -----------------------------------------------------------------------------
//
// Button Stuff
//
// -----------------------------------------------------------------------------

const byte DPIN_BUTTON_INTERRUPT = 2;
const byte DPIN_BUTTON_GROUND = 3;

volatile bool acceptPushedChanges = false;
volatile bool pushed = false;

// The number of ms to wait before accepting button pushed state changes
const int ACCEPT_PUSHED_CHANGE_DELAY = 1000;
// The number of ms to wait before checking the pushed state.
// @see - https://www.arduino.cc/en/Tutorial/Debounce
const int DEBOUNCE_DELAY = 100;
// The number of ms to wait before restting the pushed state.
// In my experience, a seemingly "natural" amount of time to wait for the button
// push event is half a second.  You can set this to whatever you like.
const int PUSHED_RESET_DELAY = 500;

/**
 * Call this in the setup() function.  This will configure the two pins needed
 * to detect the button press.
 */
void initializeButtonPins(const byte interruptPin, const byte groundPin) {
  // The "brains" of the interrupt switch rely on INPUT_PULLUP, which will put out 5V, which arduino
  // considers HIGH. To get the switch to make our pin read LOW, we must attach our pin to a ground
  // through our switch, which will create the LOW state on our pin.
  pinMode(interruptPin, INPUT_PULLUP);
  // I found FALLING and RISING to be the most reliable here.
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPushed, FALLING);
  pinMode(groundPin, OUTPUT);
  digitalWrite(groundPin, LOW);
}

/**
 * Logic that will be executed when a button press is detected.  Note that this
 * is more complicated than simply executing the logic you want to have happen
 * on button press.  We have to account for debouncing, both from user input and
 * from the aduino actually booting.
 *
 * You do not need to call this explicitly - it is registered as the function
 * that gets called when the button is pushed by the initializeButtonPins()
 * function above.
 */
void buttonPushed() {
  if (pushed == false) {
    // When the button is pushed, track the fact that it's been pushed once here.  This boolean must
    // be set back to false somewhere else.
    pushed = true;

    // This state will flip sometimes when the board first starts up or is reset.  This bootstrap
    // delay will ensure that the state cannot change until after a certain amount of time has
    // elapsed.
    if (acceptPushedChanges || millis() > ACCEPT_PUSHED_CHANGE_DELAY) {
      // Using this flag, and setting it to true here, will prevent the need to call the millis function.
      acceptPushedChanges = true;

      // This is the part you should change.  You can keep this function call if
      // you would like, but you should change the contents of the
      // "onButtonPushed" function to do what you need it to do.
      onButtonPushed();
    }
  }
}

/**
 * This should be called at the end of loop, or at least that's where I needed
 * it.  I don't know how to explain what it does and why it's needed other than
 * to say that it waits long enough to account for mechanical imprecision before
 * resetting the "pushed" state to allow the user to press the button again.
 */
void buttonPushedReset() {
  delay(DEBOUNCE_DELAY);

  if (pushed == true) {
    delay(PUSHED_RESET_DELAY);
    pushed = false;
  }
}


// -----------------------------------------------------------------------------
//
// State Stuff
//
// This is where you can put the logic that will control your state when the
// button is pushed.
//
// -----------------------------------------------------------------------------

// This is just a state indicator for this simple example.  The state you need
// to change when the button is pushed will likely be different.
volatile byte state = LOW;

void onButtonPushed() {
  // For this simple example, we're just flipping a boolean.  For a
  // not-so-simple application, this is where the logic should go for when the
  // button gets pushed.
  state = !state;
}


// -----------------------------------------------------------------------------
//
// Arduino Stuff
//
// -----------------------------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  initializeButtonPins(DPIN_BUTTON_INTERRUPT, DPIN_BUTTON_GROUND);
}

void loop() {
  digitalWrite(LED_BUILTIN, state);

  // For this simple example, we will reset the button push at the end of the loop.  There is a
  // built-in delay, so the loop won't restart right away.
  buttonPushedReset();
}
