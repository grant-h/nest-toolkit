#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <time.h>

class Timer
{
public:
  Timer();
  Timer(time_t timeMs);
  ~Timer();

  // returns true on timer completion
  bool tick();
  void set(time_t timeMs);
  void start();
  void stop();
  void restart();

  bool isSet();
  bool isDone();
  bool isStarted();
  bool isStopped();
  time_t timeLeft();
private:
  enum timer_state_t
  {
    TIMER_STOPPED,
    TIMER_STARTED
  } m_state;

  struct timeval m_startTime, m_lastTick;
  time_t m_timerLen; // in milliseconds
  time_t m_timeLeft; // in milliseconds
  bool m_set;
  bool m_done;
};

#endif
