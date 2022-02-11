/******************************************************************
  @file       NexgenTimer.h
  @brief      A non blocking Scheduler based on millis().
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

  Credits - based on an amalgamation of the MillisTimer code by 
            Brett Hagman <bhagman@wiring.org.co> and elapsedMillis 
            (http://www.pjrc.com/teensy/).

******************************************************************/

#ifndef NexgenTimer_h
#define NexgenTimer_h

#include "Arduino.h"

class NexgenTimer;
typedef void (*timerEventHandler)(NexgenTimer&);

class NexgenTimer
{
  public:
    NexgenTimer(uint32_t interval = 0, timerEventHandler handler = NULL);

    void expiredHandler(timerEventHandler handler);
    void setInterval(uint32_t interval);
    void setRepeats(uint32_t repeat);      // number of times to repeat, (0) for indefinitely (default)
    void setTargetTime(uint32_t targetTime);
    uint32_t getTargetTime() const;
    uint32_t getRemainingTime() const;
    uint32_t getRemainingRepeats() const;  // Number of repeats remaining.
    bool isRunning() const;
    void stop();
    void start();
    void startFrom(uint32_t startTime);
    void reset();
    bool expired();
    void run();

    uint8_t ID;

  private:
    enum { RUNNING, STOPPED, EXPIRED } m_state;
    
    uint32_t m_targetTime;
    uint32_t m_remainingTime;
    uint32_t m_interval;
    uint32_t m_repeat;
    uint32_t m_repeatCount;
    timerEventHandler cb_onExpired;
};

class Timeout
{
  private:
    unsigned long ms;
    unsigned long timeout;

  public:
    Timeout(void) { ms = millis(); }
    Timeout(unsigned long val) { timeout = millis() + val; }

    bool expired() { return (millis() >= timeout); }
    bool notExpired() { return (millis() < timeout); }
};

class ElapsedMillis
{
private:
  unsigned long ms;

public:
  ElapsedMillis(void) { ms = millis(); }
  ElapsedMillis(unsigned long val) { ms = millis() - val; }
  ElapsedMillis(const ElapsedMillis &orig) { ms = orig.ms; }
  operator unsigned long () const { return millis() - ms; }
  ElapsedMillis & operator = (const ElapsedMillis &rhs) { ms = rhs.ms; return *this; }
  ElapsedMillis & operator = (unsigned long val) { ms = millis() - val; return *this; }
  ElapsedMillis & operator -= (unsigned long val)      { ms += val ; return *this; }
  ElapsedMillis & operator += (unsigned long val)      { ms -= val ; return *this; }
  ElapsedMillis operator - (int val) const           { ElapsedMillis r(*this); r.ms += val; return r; }
  ElapsedMillis operator - (unsigned int val) const  { ElapsedMillis r(*this); r.ms += val; return r; }
  ElapsedMillis operator - (long val) const          { ElapsedMillis r(*this); r.ms += val; return r; }
  ElapsedMillis operator - (unsigned long val) const { ElapsedMillis r(*this); r.ms += val; return r; }
  ElapsedMillis operator + (int val) const           { ElapsedMillis r(*this); r.ms -= val; return r; }
  ElapsedMillis operator + (unsigned int val) const  { ElapsedMillis r(*this); r.ms -= val; return r; }
  ElapsedMillis operator + (long val) const          { ElapsedMillis r(*this); r.ms -= val; return r; }
  ElapsedMillis operator + (unsigned long val) const { ElapsedMillis r(*this); r.ms -= val; return r; }
};

#endif