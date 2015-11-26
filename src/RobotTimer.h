#ifndef ROBOTTIMER_H_
#define ROBOTTIMER_H_

enum TimeUnit {
  MILLISECONDS = 1,
  SECONDS = 1000,
  MINUTES = SECONDS * 60,
  HOURS = MINUTES * 60
};

class RobotTimer {
  private:
    bool started = false;
    bool complete = false;
    unsigned int lengthMs = 0;
    unsigned int startTimeMs = 0;

  public:
    RobotTimer();
    void process();
    void start();
    void stop();
    bool isComplete();
    void setDuration(unsigned int length, TimeUnit unit);
};

#endif
