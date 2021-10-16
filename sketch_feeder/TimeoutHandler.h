#ifndef TIMEOUT_HANDLER_H
#define TIMEOUT_HANDLER_H
#include <TaskManager.h>
#include <TaskScheduler.h>

typedef std::function<void()> TimeoutHandlerListener;

class TimeoutHandler : public Task {
  private:
    TimeoutHandlerListener timeoutHandlerListener = NULL;

  public:
    TimeoutHandler(Scheduler* scheduler) : Task(TASK_IMMEDIATE, TASK_ONCE, scheduler) {

    }
    ~TimeoutHandler() {};

    void setOnTimeoutListener(const TimeoutHandlerListener& listener) {
      this->timeoutHandlerListener = listener;
    }

    bool Callback() {
      Serial.println((String)"TimeoutHandler Callback");
      if (timeoutHandlerListener != NULL) {
        timeoutHandlerListener();
      }
      return true;
    }


    bool OnEnable() {
      Serial.println("TimeoutHandler: OnEnable");
      return true;
    }

    void restart() {
      restartDelayed();
    }

};
#endif
