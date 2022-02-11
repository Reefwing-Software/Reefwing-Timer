/******************************************************************
  @file       NexgenTimer.cpp
  @brief      A non blocking Scheduler based on millis().
  @author     David Such
  @copyright  Please see the accompanying LICENSE.txt file.

  Code:        David Such
  Version:     1.0
  Date:        06/02/22

  1.0 Original Release          06/02/22

******************************************************************/

#include "Arduino.h"
#include "NexgenTimer.h"

// Constructor
NexgenTimer::NexgenTimer(uint32_t interval, timerEventHandler handler) {
  m_interval = interval;
  m_state = STOPPED;                   // Stopped
  m_repeat = m_repeatCount = 0;        // Repeat indefinitely
  m_targetTime = millis() + interval;
  m_remainingTime = 0;
  cb_onExpired = handler;
}

bool NexgenTimer::isRunning() const {
  return (m_state == RUNNING);
}

void NexgenTimer::run() {
  expired();
}

bool NexgenTimer::expired() {
  // Only if we're running
  if (m_state == RUNNING) {
    // If we have passed the target time...
    if (millis() >= m_targetTime) {
      // Calculate repeat. If repeat = 0, then we
      // repeat forever until stopped.
      // Otherwise, when we've hit the last repeat (1),
      // then we stop.
      if (m_repeatCount != 1) {
        if (m_repeatCount > 0) {
          m_repeatCount--;
        }
        // Set the new target (based on our last target time
        // for accuracy)
        m_targetTime += m_interval;
      }
      else {
        m_state = EXPIRED;
      }

      // Fire the call back.
      if (cb_onExpired) {
        cb_onExpired(*this);
      }

      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

void NexgenTimer::stop() {
  m_state = STOPPED;
  
  // If we have stopped before the target time,
  // let's save the remaining time so we can resume later.
  if (millis() < m_targetTime) {
    m_remainingTime = m_targetTime - millis();
  }
}

// Start the timer.
void NexgenTimer::start() {
  startFrom(millis());
}

// Start from a specific time provided.
void NexgenTimer::startFrom(uint32_t startTime) {
  m_state = RUNNING;
  // If we have some remaining time, then let's use that.
  if (m_remainingTime > 0) {
    m_targetTime = startTime + m_remainingTime;
    m_remainingTime = 0;
  }
  else {
    // otherwise, we start normally
    m_targetTime = startTime + m_interval;
  }
}

// Arbitrarily set the target time.
void NexgenTimer::setTargetTime(uint32_t targetTime) {
  m_targetTime = targetTime;
}

// Reset the timer. Stop, and reset repeat count.
void NexgenTimer::reset() {
  m_state = STOPPED;
  m_remainingTime = 0;
  m_repeatCount = m_repeat;
}

void NexgenTimer::setInterval(uint32_t interval) {
  m_interval = interval;
}

void NexgenTimer::setRepeats(uint32_t repeatCount) {
  m_repeat = m_repeatCount = repeatCount;
}

void NexgenTimer::expiredHandler(timerEventHandler handler) {
  cb_onExpired = handler;
}

uint32_t NexgenTimer::getTargetTime() const {
  return m_targetTime;
}

uint32_t NexgenTimer::getRemainingTime() const {
  if (m_state == RUNNING) {
    return m_targetTime - millis();
  }
  else {
    return m_remainingTime;
  }
}

uint32_t NexgenTimer::getRemainingRepeats() const {
  if (m_state == EXPIRED && m_repeatCount == 1)
    return 0;
  else
    return m_repeatCount;
}