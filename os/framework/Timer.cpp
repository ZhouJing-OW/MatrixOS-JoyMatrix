#include "Timer.h"

Timer::Timer() {
  Timer::RecordCurrent();
}

bool Timer::Tick(uint32_t ms, bool continuous_mode) {

  if(ms == UINT32_MAX)
    return false;
  if ((MatrixOS::SYS::Millis()) < previous)
    previous = 0;
  if (Timer::IsLonger(ms))
  {
    if (continuous_mode)
    { previous += ms; }
    else
    { Timer::RecordCurrent(); }
    return true;
  }
  return false;
}

bool Timer::IsLonger(uint32_t ms) {
  return (previous + ms) <= MatrixOS::SYS::Millis();
}

uint32_t Timer::SinceLastTick() {
  return MatrixOS::SYS::Millis() - previous;
}

void Timer::RecordCurrent() {
  previous = MatrixOS::SYS::Millis();
}

void Timer::Reset() {
  previous = 0;
}


// namespace MatrixOS::SYS {uint64_t Micros();}

MicroTimer::MicroTimer(bool start)
{
  gptimer_config_t timer_config = 
  {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
  };
  gptimer_new_timer(&timer_config, &gptimer);
  gptimer_enable(gptimer);
  if (start) 
  {
    gptimer_start(gptimer);
    MicroTimer::RecordCurrent();
  }
}

uint64_t MicroTimer::Micros()
{
  gptimer_get_raw_count(gptimer, &count);
  return count;
}

bool MicroTimer::Tick(uint64_t us) 
{
  gptimer_get_raw_count(gptimer, &count);
  if (count < previous)
    previous = 0;

  if (MicroTimer::IsLonger(us))
  {
    MicroTimer::RecordCurrent();
    return true;
  }
  return false;
}

bool MicroTimer::IsLonger(uint64_t us)
{
  gptimer_get_raw_count(gptimer, &count);
  return (previous + us) <= count;
}

uint64_t MicroTimer::SinceLastTick()
{
  gptimer_get_raw_count(gptimer, &count);
  return count - previous;
}

void MicroTimer::RecordCurrent()
{
  gptimer_get_raw_count(gptimer, &previous);
}

void MicroTimer::Start()
{
  gptimer_start(gptimer);
  MicroTimer::RecordCurrent();
}
void MicroTimer::Stop()
{
  gptimer_stop(gptimer);
}
