
int DPIN_BUTTON_SENSOR = 8; // one switch wire
int DPIN_BUTTON_GROUND = 9; // the other switch wire

int BUTTON_IS_PRESSED = LOW;

void setup() {
  // The built in LED is going to show the button state
  pinMode(LED_BUILTIN, OUTPUT);

  // To avoid noise(?), use INPUT_PULLUP instead of INPUT
  pinMode(DPIN_BUTTON_SENSOR, INPUT_PULLUP);

  // Configure the other switch wire to be a ground
  pinMode(DPIN_BUTTON_GROUND, OUTPUT);
  digitalWrite(DPIN_BUTTON_GROUND, LOW);
}

void loop() {
  if (digitalRead(DPIN_BUTTON_SENSOR) == BUTTON_IS_PRESSED){
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
