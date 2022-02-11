/******************************************************************
  @file       batteryCheck.ino
  @brief      Checks voltage on pin A0 every second
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

******************************************************************/

#include <NexgenTimer.h>

/******************************************************************
 *  Battery Voltage ADC DEFINES
 * 
 *  This sketch assumes that a voltage source (i.e., battery) is
 *  connected to analogue input pin A0 (i.e., pin 14), via a voltage
 *  divider which consists of R1 and R2.
 * 
 *  The values shown are correct for an UNO. You will need to change
 *  R1, R2 and VLOGIC if you are using an Arduino which uses 3V3 logic
 *  (e.g., the Nano 33 IoT). Also if your voltage source is connected 
 *  to a different analogue input, you will need to change VBAT as well.
 * 
 ******************************************************************/

#define VBAT      14                //  Analogue Pin A0

const float VLOGIC = 5.0;           //  UNO has 5V logic
const float R1 = 10000.0;           //  10K Resistor
const float R2 = 6800.0;            //  6K8 Resistor
const float RATIO = (R1 + R2) / R2; //  Voltage Divider ratio
const float SCALE = VLOGIC * RATIO; //  Voltage conversion factor

float readADCValue(int pin) {
  //  ADC hardware rounds down so add 0.5
  return (float)analogRead(pin) + 0.5;
}

float calculateVoltage(int pin) {
  float adcValue = readADCValue(pin); 

  return (adcValue / 1024.0) * SCALE;
}

void voltageCheck(NexgenTimer &nt) {
  //  Called when the timer expires - toggle LED state at each read
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  //  Read the Battery Voltage and send to Serial
  float batteryVoltage = calculateVoltage(VBAT);

  Serial.print("Voltage: ");
  Serial.println(batteryVoltage);
}

NexgenTimer nexgenTimer = NexgenTimer(1000, voltageCheck);

void setup() {
  //  Pin Configuration
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VBAT, INPUT);

  //  Start Serial
  Serial.begin(115200);

  //  Timer Configuration
  nexgenTimer.start();
}

void loop() {
  //  Put your main code here, to run repeatedly:
  nexgenTimer.run();
  delay(10);
}