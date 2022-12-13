/******************************************************************
  @file       debounceTimer.ino
  @brief      How to use ElapsedMillis as a debounce timer.
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     2.0
  Date:        12/12/22

  1.0 Original Release          06/02/22
  2.0 Rebranding                13/12/22

  Notes:

  1. To avoid having to use an external 10k pull down resistor, 
  as required by the IDE debounce sketch, we will use active LOW 
  logic and utilise the internal pull up resistor.
  2. Momentary switch is connected to digital input 2. The other 
  leg of the switch is connected to ground.
  3. The digitalRead() function returns an int which represents HIGH
  or LOW.
******************************************************************/

#include <ReefwingTimer.h>

const int SWITCH_PIN = 2;
const unsigned long DEBOUNCE_DELAY = 50;

int previousState = HIGH, currentState = HIGH;

ElapsedMillis debounceTimer;

void setup() {
  // Pin Configuration
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop() {
  //  Read the current switch state
  currentState = digitalRead(SWITCH_PIN);

  //  Detect state change
  if (currentState != previousState && debounceTimer > DEBOUNCE_DELAY) {
    if (currentState == LOW) {
      //  Switch pressed - toggle LED state
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    debounceTimer = 0;
  }

  previousState = currentState;

}