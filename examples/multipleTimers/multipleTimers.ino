/******************************************************************
  @file       multipleTimers.ino
  @brief      Approximates multitasking using multiple timers
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

******************************************************************/

#include <NexgenTimer.h>

void timer1Handler(NexgenTimer &nt) {
  Serial.print("Timer 1 Task - remaining calls: ");
  Serial.println(nt.getRemainingRepeats());
}

void timer2Handler(NexgenTimer &nt) {
  Serial.println("Timer 2 Task");
}

void timer3Handler(NexgenTimer &nt) {
  Serial.println("Timer 3 Task");
}

void timer4Handler(NexgenTimer &nt) {
  Serial.println("Timer 4 Task");
}

void timer5Handler(NexgenTimer &nt) {
  Serial.print("Timer 5 Task - remaining calls: ");
  Serial.println(nt.getRemainingRepeats());
}

NexgenTimer timer1(1000, timer1Handler), timer2(2000, timer2Handler), timer3(2500, timer3Handler), 
            timer4(6200, timer4Handler), timer5(8700, timer5Handler);

void setup() {
  //  Start Serial
  Serial.begin(115200);
  Serial.println("Nexgen Multiple Timer Example");

  //  Timer Configuration
  timer1.setRepeats(10);
  timer5.setRepeats(5);
  
  timer1.start();
  timer2.start();
  timer3.start();
  timer4.start();
  timer5.start();
}

void loop() {
  //  Put your main code here, to run repeatedly:
  timer1.run();
  timer2.run();
  timer3.run();
  timer4.run();
  timer5.run();
  delay(10);
}