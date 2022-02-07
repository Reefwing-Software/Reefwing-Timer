# Nexgen Timer Library
 A non blocking Scheduler based on millis().

## Background
Both the Nexgen Rover and the Nexgen Drone use LiPo batteries to provide power. These are a good choice in terms of current delivery, size, weight and capacity. However, you don't want to over discharge them or you can impact battery life. 

To help prevent this we developed a library which can be used to regularly check the battery voltage, allowing us to take action if it gets too low.

We have rover shields which suit the Arduino UNO, Mega 2560, and the Nano 33 IoT. Our Magpie drone can use the Nano 33 BLE, Nano 33 IoT and the Portenta H7. Ideally we want a scheduling library which is suitable for all the Arduino boards.

## Battery Voltage Monitoring
The 3S LiPo battery we are using has a nominal voltage of 11.1V and when fully charged, will put out 12.6V. You don't want to discharge the LiPo below 15% capacity (around 11V for the 3S battery or somewhere between 3.4V to 3.6V per cell), as it then quickly falls off the capacity cliff and you can damage the battery.

To monitor the battery we will use a simple voltage divider and the ADC (Analog to Digital Converter) on the Arduino. We will look at the requirements for the UNO, but the principles are the same for the other boards, although you may need the voltage divider to reduce the measured voltage to 3V3 rather than 5V depending on the board.

## Arduino UNO Analog to Digital Converter
We need a resistive voltage divider (R1 and R2) because you can't apply more than 5V to an Analog Input on the UNO.

The ATMega328P has a 10-bit ADC. This means that it can return 2^10 (ie., 0 – 1023) values. The eight analog inputs on the UNO are connected to the same ADC, so you can only sample 1 input at a time. The ADC can also be used to measure the internal temperature sensor, GND and the 1.1V band gap reference voltage.

The ADC measures voltage by charging an internal 14pF capacitor and then measures that voltage with successive approximations. This implies that resistor R1 in our voltage divider (the input impedance) can't be too large or the capacitor wont charge quickly enough. The ADC Sample and Hold takes approximately 12μs, and the entire conversion process can take up to 260μs (depending on the prescaler selected). How large would be too large for R1? To work this out we have to calculate how long it takes to charge the sample and hold capacitor.

The time taken to charge the sample & hold capacitor will be affected by two things:
- The frequency of the input signal
- The total impedance of the input signal.

A capacitor is considered fully charged at 5τ. Where:
```
τ = RC
```
From the data sheet, Analog Input impedance is 1 to 100k (depending on frequency). The recommended R1 is 10k, thus total impedance = 110k.
```
charge time = 5RC = 110 x 103 x 14 x 10–12 = 7.7 μs
```
We require 12μs for sample and hold, so R1 = 10k works, and if we work backwards:
```
Maximum impedance = 12μs / 5C = 171 kΩTherefore, maximum R1 = 171k - 100k = 71 kΩ
```
Having established that R1 = 10 kΩ, Vout = 5V and Vin = 12.6V, we can use the voltage divider formula to calculate R2.
```
R2 = (R1 x Vout/Vin) / (1 - Vout/Vin) = (10k x 5/12.6) / (1–5/12.6) = 6579 Ω
```
The closest available resistor (5% tolerance) is 6.2 kΩ, this will give Vout = 4.8V for Vin = 12.6V. 

The code to read the battery voltage is quite straight forward.

```c++
const float dividerRatio = (float)(R1 + R2) / (float)R2; 

// ADC Hardware rounds down so add 0.5 
float adcValue = (float)analogRead(VBAT) + 0.5; 
batteryVoltage = (adcValue / 1024.0) * 5.0 * dividerRatio;
```
R1 and R2 are the values of the resistors used in the voltage divider. It is important to cast the resistor values to float or the Arduino will do integer division and you will get the wrong value for the dividerRatio. We add 0.5 to the value read from the ADC (Analog to Digital Converter) because the hardware rounds down when it is measuring the voltage.

## Scheduling a Reading
The loop{} frequency in an Arduino sketch will depend on the processor speed and code content. Typically though it will be executing thousands or even millions of times per second. With a flight controller, execution cycles are precious and you don't want to waste them on non-critical tasks. How often do we need to sample our battery voltage? Well it depends on the discharge rate (C). Here is the specification of a typical LiPo battery.
```
Minimum Capacity: 2200mAh
Configuration: 3S1P / 11.1v / 3Cell
Constant Discharge: 25C
Peak Discharge (10sec): 35C
```
There are two discharge ratings, one for safe continuous use and the other for sub 10 second bursts (e.g., when a drone is taking off or punching out). To convert the discharge rating to current, we multiply it by the capacity. From the specifications above the battery capacity is 2200 mAh = 2.2 Ah.
```
Discharge Current = C × Capacity
```
As we want the worst case discharge rate we will work with the peak number, which gives us:
```
Peak Discharge Rate = 35 × 2.2 = 77 A
```
We don't want to discharge below 15% of capacity (i.e., 85% × 2.2 Ah = 1.87 Ah) which at the peak discharge rate will take:
```
Discharge Time (s) = (1.87 / 77) × 3600 = 87.4 seconds
```
Of course you would destroy the battery if you tried to do this. The maximum safe discharge rate for the battery is:
```
Safe Discharge Time (s) = (1.87 × 3600) / (25 × 2.2) = 6732 / 55 = 122.4 s
```
Most drones/rovers/etc. are not going to be designed to run at 100% of the battery discharge capacity and so the discharge time is likely to be significantly longer than even the maximum safe discharge time.

The discharge curve is fairly linear within the safe operating range of the battery. Even ignoring the effects of temperature our monitored voltage will fluctuate with use. For example, when you use full throttle, the motors will draw a lot of current and the battery voltage will decrease or "sag" temporarily, this could trigger a false low voltage alarm. When you decrease throttle, the voltage level will recover. Thus our battery monitoring needs to include some sort of hysteresis.

Taking all this into account, even checking the battery voltage every second is excessive, but given we want to take a moving average to counter voltage sag under high loads, sampling every second will be workable.
```
Note that charging is very different to discharging. A typical safe charge rate for LiPo's is 1C.
```
## Scheduling Options
The initial thinking was to use a hardware timer interrupt and then measure the battery in the Interrupt Service Routine (ISR). This would work but interupt setting and handling is very hardware dependent. It would be difficult to design something that would work on all Arduino boards.

An alternative approach is to use the Arduino **millis()** function. This returns the number of milliseconds passed since the Arduino board began running the current program (powered up or reset). This number will overflow and be set to zero, after approximately 50 days (49.71 days to be precise).

>The millis() function is part of the Arduino core and is driven by a millisecond timer overflow interrupt (TIMER0 on the UNO) that increments an unsigned long every time it activates and returns the value of that variable. For the ATMega328P processor on the UNO running at 16 MHz, the ISR (Interrupt Service Routine) actually gets called every 1.024 ms, so the function needs to account for this small error. It does this by accumulating the 0.024 ms error each time it executes (every overflow), until the total error approaches 1 ms. At this point, millis() jumps by 2 ms and corrects itself.

The [Arduino core](https://medium.com/r/?url=https%3A%2F%2Fwww.arduino.cc%2Freference%2Fen%2F) is a software API for a specific group of MCU's. It provides an abstraction layer between your sketch and how these functions are coded for the physical hardware. Sketches which only use Arduino core functions should run on all Arduino boards.

For this reason we have created a wrapper library which uses the millis() function to instantiate a non blocking scheduler. Our library is an amalgamation of the [MillisTimer](https://github.com/bhagman/MillisTimer) and [ElapsedTimer](http://www.pjrc.com/teensy/) libraries with some application specific examples.

## Examples
The examples folder of the library contains the following sketches:
- nonBlockingBlink.ino: An implementation of the classic blink sketch using NexgenTimer.
- batteryCheck.ino: An example of a scheduled task, in this case checking the battery voltage every second.