#include "Timer.h"

#include <unistd.h>
#include <stdio.h>

Timer::Timer()
{
  m_state = TIMER_STOPPED;

  stop();
  m_set = false;
  m_done = false;
}

Timer::Timer(time_t timeMs)
{
  m_state = TIMER_STOPPED;

  stop();
  set(timeMs);
}

Timer::~Timer()
{

}

bool Timer::tick()
{
  if(!isSet())
    return false;

  if(isStopped())
    return m_done;

  // must be started
  struct timeval now, delta;
  time_t deltaMs = 0;

  gettimeofday(&now, NULL);
  timersub(&now, &m_lastTick, &delta);

  deltaMs = delta.tv_sec*1000 + delta.tv_usec/1000;

  if(deltaMs >= m_timeLeft)
  {
    m_done = true;
    m_timeLeft = 0;

    stop();

    return true;
  }
  else
  {
    m_done = false;

    if(deltaMs)
    {
      m_timeLeft -= deltaMs;
      m_lastTick = now;
    }

    return false;
  }
}

void Timer::set(time_t timeMs)
{
  stop(); // stop any running timer

  m_set = true; // note that we are set
  m_timerLen = timeMs;
  m_timeLeft = timeMs;
}

void Timer::start()
{
  if(!isSet())
    return;

  // stop -> start, reset the start time
  if(m_state == TIMER_STOPPED) 
  {
    m_state = TIMER_STARTED;

    gettimeofday(&m_startTime, NULL);
    m_lastTick = m_startTime;
    m_timeLeft = m_timerLen;
    m_done = false;
  }
}

void Timer::stop()
{
  if(!isSet())
    return;

  // start -> stop, note the time left
  if(m_state == TIMER_STARTED)
  {
    m_state = TIMER_STOPPED;
  }
}

void Timer::restart()
{
  if(!isSet())
    return;

  stop();
  start();
}

bool Timer::isDone()
{
  return m_done;
}

bool Timer::isSet()
{
  return m_set;
}

bool Timer::isStarted()
{
  return m_state == TIMER_STARTED;
}

bool Timer::isStopped()
{
  return m_state == TIMER_STOPPED;
}

time_t Timer::timeLeft()
{
  return m_timeLeft;
}
