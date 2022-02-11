/******************************************************************
  @file       userTimeout.ino
  @brief      Stops waiting for user input after 2 seconds
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

  Credit - based on the example at: 
  https://www.pjrc.com/teensy/td_timing_elaspedMillis.html

******************************************************************/

#include <NexgenTimer.h>

void setup() {
  //  Start Serial
  Serial.begin(115200);
}

void loop() {                          // each time the loop() runs,
  Timeout timeout = Timeout(2000);     // timeout is reset to 2 secs.
  
  while (timeout.notExpired()) {
    if (Serial.available()) {
      char c = Serial.read();
      
      Serial.print("got char = ");  // do something with c
      if (c == '\r')
        Serial.println("CR");
       else if (c == '\n')
        Serial.println("NL");
       else
        Serial.println(c);
    }
  }
  
  Serial.println("waited 2 seconds, no data arrived");  
}