/******************************************************************
  @file       serialTimeout.ino
  @brief      Stops waiting for a serial connection after 5 seconds
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     2.0
  Date:        13/12/22

  1.0 Original Release          06/02/22
  2.0 Rebranding                13/12/22

******************************************************************/

#include <ReefwingTimer.h>

void setup() {
  Timeout timeout = Timeout(5000);
  
  //  Start Serial and wait 5 seconds for connection
  Serial.begin(115200);
  while (!Serial || timeout.notExpired());
}

void loop() {
  //  Put your main code here, to run repeatedly:
  if (Serial) {
    Serial.println("Serial Connection made after 5 seconds.");
  }
  delay(1000);
}