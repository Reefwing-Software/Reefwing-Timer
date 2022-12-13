/******************************************************************
  @file       nonBlockingBlink.ino
  @brief      A non blocking LED blink example using ReefwingTimer
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     2.0
  Date:        13/12/22

  1.0 Original Release          06/02/22
  2.0 Rebranding                13/12/22

******************************************************************/

#include <ReefwingTimer.h>

ReefwingTimer ReefwingTimer = rTimer();

void timerHandler(ReefwingTimer &nt) {
  //  Called when the timer expires - toggle LED state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
  //  Set pin connected to LED as an output
  pinMode(LED_BUILTIN, OUTPUT);

  //  Configure Timer - it defaults to infinite repeat
  //  Interval (in ms) and Handler could alternatively be set in the constructor,
  //  For example: ReefwingTimer ReefwingTimer = rTimer(1000, timerHandler);
  rTimer.setInterval(1000);
  rTimer.expiredHandler(timerHandler);
  rTimer.start();
}

void loop() {
  //  Put your main code here, to run repeatedly:
  rTimer.run();
  delay(10);
}