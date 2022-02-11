/******************************************************************
  @file       nonBlockingBlink.ino
  @brief      A non blocking LED blink example using NexgenTimer
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

******************************************************************/

#include <NexgenTimer.h>

NexgenTimer nexgenTimer = NexgenTimer();

void timerHandler(NexgenTimer &nt) {
  //  Called when the timer expires - toggle LED state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
  //  Set pin connected to LED as an output
  pinMode(LED_BUILTIN, OUTPUT);

  //  Configure Timer - it defaults to infinite repeat
  //  Interval (in ms) and Handler could alternatively be set in the constructor,
  //  For example: NexgenTimer nexgenTimer = NexgenTimer(1000, timerHandler);
  nexgenTimer.setInterval(1000);
  nexgenTimer.expiredHandler(timerHandler);
  nexgenTimer.start();
}

void loop() {
  //  Put your main code here, to run repeatedly:
  nexgenTimer.run();
  delay(10);
}