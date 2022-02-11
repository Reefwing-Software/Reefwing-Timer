# Nexgen Timer Library
 A non blocking Scheduler based on millis().

## Background
Both the Nexgen Rover and the Nexgen Drone use LiPo batteries to provide power. These are a good choice in terms of current delivery, size, weight and capacity. However, you don't want to over discharge them or you can impact battery life. 

To help prevent this we developed a library which can be used to regularly check the battery voltage, allowing us to take action if it gets too low.

We have rover shields which suit the Arduino UNO, Mega 2560, and the Nano 33 IoT. Our Magpie drone can use the Nano 33 BLE, Nano 33 IoT and the Portenta H7. Ideally we want a scheduling library which is suitable for all the Arduino boards.

Although this library was originally for scheduling battery readings, we found that it has much wider application. The examples folder of the library contains the following sketches:
- ```nonBlockingBlink```: An implementation of the classic blink sketch using ```NexgenTimer```.
- ```batteryCheck```: An example of a scheduled task, in this case checking the battery voltage every second.
- ```serialTimeout```: Stops waiting for a serial connection after 5 seconds.
- ```userTimeout```: Stops waiting for user input after a set period.
- ```debounceTimer```: How to use ```ElapsedMillis``` as a switch debounce timer.
- ```multipleTimers```: A simple multitasking example.

The ```NexgenTimer``` is a wrapper library which uses the ```millis()``` function to instantiate a non blocking scheduler. Our library is an amalgamation of the [MillisTimer](https://github.com/bhagman/MillisTimer) and [ElapsedTimer](http://www.pjrc.com/teensy/) libraries with some application specific examples and an extra ```Timeout``` class. It only uses functions from the Arduino core API and hence should work on all Arduino boards.

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

## Scheduling a Reading
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

>The millis() function is part of the Arduino core and is driven by a millisecond timer overflow interrupt (TIMER0 on the UNO) that increments an unsigned long every time it activates and returns the value of that variable. For the ATMega328P processor on the UNO running at 16 MHz, the ISR (Interrupt Service Routine) actually gets called every 1.024 ms, so the function needs to account for this small error. It does this by accumulating the 0.024 ms error each time it executes (every overflow), until the total error approaches 1 ms. At this point, millis() jumps by 2 ms and corrects itself. For this reason, don't ever use == as a conditional comparison when using millis() as it can increment by more than one. Always use >=, <=, >, or <.

The [Arduino core](https://medium.com/r/?url=https%3A%2F%2Fwww.arduino.cc%2Freference%2Fen%2F) is a software API for a specific group of MCU's. It provides an abstraction layer between your sketch and how these functions are coded for the physical hardware. Sketches which only use Arduino core functions should run on all Arduino boards.

For this reason we have created a wrapper library which uses the millis() function to instantiate a non blocking scheduler. Our library is an amalgamation of the [MillisTimer](https://github.com/bhagman/MillisTimer) and [ElapsedTimer](http://www.pjrc.com/teensy/) libraries with some application specific examples and an extra ```Timeout``` class.

## The NexgenTimer Library
To use the library you need to include it in your sketch.
```c++
#include <NexgenTimer.h>
```
There are two approaches to instantiating a NexgenTimer object:
- Use the constructor which includes the interval in milliseconds and the name of the expired timer handler function [e.g., ```NexgenTimer nexgenTimer = NexgenTimer(1000, timerHandler);```], or
- Use an empty constructor and define those variables in setup{} [e.g., ```NexgenTimer nexgenTimer = NexgenTimer();```].

If you use the first approach, the timerHandler function will need to be defined above the NexgenTimer. By default, the timer will repeat indefinitely, but you can use the setRepeats method to configure it for a certain number of repeats [e.g., ```nexgenTimer.setRepeats(10);```].
The last thing that you should do in setup{} is to start the timer.
```c++
nexgenTimer.start();
```
Then within the loop{}, you have to let the timer update by calling:
```c++
nexgenTimer.run();
```

## Examples
The best way to see how to use the NexgenTimer library is with some examples. These are all contained within the library and can be accessed via the Arduino IDE once you have added the library with the library manager.

The examples folder of the library contains the following sketches:
- nonBlockingBlink: An implementation of the classic blink sketch using NexgenTimer.
- batteryCheck: An example of a scheduled task, in this case checking the battery voltage every second.
- serialTimeout: Stops waiting for a serial connection after 5 seconds.
- userTimeout: Stops waiting for user input after a set period.
- debounceTimer: How to use ElapsedMillis as a switch debounce timer.
- multipleTimers: A simple multitasking example

## 1. Non Blocking Blink
The first Arduino sketch you run is normally the one that blinks the built in LED using ```delay()```. The problem with ```delay()``` is that you can't do anything else while the delay is in progress. This can make your sketch appear unresponsive and is a problem if you are trying to do any sort of multi-tasking.

There is already a non blocking blink example in the Arduino IDE under ```Digital -> BlinkWithoutDelay```. This sketch uses ```millis()```. Our example does exactly the same thing but by using the library makes the code largely self documenting.

Our sketch sets up a timer which fires every second for ever. When the timer expires, it calls ```timerHandler()```, which toggles the digital output pin which is connected to the built in LED that most Arduino boards have.

## 2. Battery Check
The second example is called ```batteryCheck()```. This works the same as the non blocking blink sketch. We create a timer which expires every second and then in the expired timer handle function we read the battery voltage. As with any interrupt handler you don't want to do too much work in the handler. In our flight controller, where we use this timer, we set a flag if the battery is too low and the main loop then takes appropriate action (i.e., sends a warning to the remote control).

## 3. Serial and User Timeouts
Another common application is where your sketch is waiting for something to happen but doesn't want to wait forever. An example of this is waiting for a serial connection. On boards with a native USB connection like the Nano 33 IoT, Nano 33 BLE and Portenta H7, it is possible to miss seeing serial messages on the terminal if you don't wait for a connection. To that end you will often see the following code in ```setup{}```:
```c++
while (!Serial);
```
For older Arduino boards that use a USB to UART serial converter, like the ATMega8U2 or FT232, ```if (Serial)``` will always return true. When you connect to the serial port of a board like the UNO or Mega 2560 the whole board usually resets, so opening the serial port allows you to see the first bits of serial data. On the Leonardo, Nano 33, etc., it doesn't reset when you connect to serial, so any serial output during the ```setup{}``` function could be missed. Adding this line makes the board pause until you open the serial port, so you get to see that initial bit of data.

The problem is if you don't make a serial connection then your sketch will hang. Often the serial connection is only required for debugging so this is not what you want. To overcome this, we can use something like the serial timeout sketch. 

The Timeout class accepts one variable during initialisation. This sets the timeout period in milliseconds (i.e., 5000 ms = 5 seconds). The class includes two methods which both return a boolean:
- ```timeout.expired()```: which is true when the timeout period is passed and false otherwise; and
- ```timeout.notExpired()```: which is the inverse of the previous function.
```c++
void setup() {
  Timeout timeout = Timeout(5000);
  
  //  Start Serial and wait 5 seconds for connection
  Serial.begin(115200);
  while (!Serial || timeout.notExpired());
}
```
You can use a similar approach to wait for user input. Full versions of both sketches are provided in the examples folder of the NexgenTimer library.
```c++
#include <NexgenTimer.h>

void setup() {
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
```

## 4. Debounce Timer
A problem with using momentary switches as inputs is that the input is not a clean square wave as you might expect. Instead, you normally get a voltage fluctuation as you press and release the button. This is referred to as bouncing. The Arduino can interpret this bounce as a series of rapid high and low signals which is not what we want.

One software method of debouncing switches is to detect the first ```LOW``` to ```HIGH``` transition and then start a timer to allow the switch to settle. For active ```LOW``` logic (i.e., 0V is ```HIGH``` and 5V is ```LOW```) or when the switch is released, you can do the same for the ```HIGH``` to ```LOW``` transition. We need to start a timer when a state change is detected and the ```ElapsedMillis``` class is ideal for this. To start the timer we set the ```ElapsedMillis``` variable to zero. A debounce delay between 50 and 100 ms is usual, this is the constant called ```DEBOUNCE_DELAY``` in our example sketch.

As this is a common requirement, there is a debounce example already available in the Arduino IDE under ```Examples -> 02. Digital -> Debounce```. It also uses ```millis()```. We think our approach is a bit more straight forward and self documenting. To avoid having to use an external 10k pull down resistor, as required by the IDE debounce sketch, we will use active ```LOW``` logic and utilise the internal pull up resistor. A pull up or pull down resistor prevents the input pin's state from floating. Assuming that we are connecting your switch to digital input 2, to turn on the internal pull up resistor you use:
```c++
pinMode(2, INPUT_PULLUP);
```
The other leg of the switch will be connected to ground. Normally pin 2 will then be held at logic ```HIGH``` by the pull up resistor and pushing the switch will bring it ```LOW```. The Arduino digitalRead(pin) function returns an int which represents ```HIGH``` or ```LOW```.

Our sketch contains two integer variables, ```previousState``` and ```currentState``` which we use to keep track of the switches state and whether it has changed. We initialise these to ```HIGH``` as that is the unpressed switch state.

The objective of the sketch is to toggle the built in LED, every time the momentary switch is pressed. The ```debounceTimer``` sketch is shown below.
```c++
#include <NexgenTimer.h>

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
```
## 5. Multiple Timers
You can't get true multitasking without using multiple processors or threads. However, we can get the next best thing with multiple timers. Say there is five tasks that we need to perform periodically, we can assign each of these tasks to its own NexgenTimer. The task can then be performed in the expired timer handler.
```c++
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
```
The timestamp on the serial monitor shows that the timers are not millisecond accurate but they are good enough for most tasks.

The ```NexgenTimer Library``` can be downloaded using the Arduino Library Manager.