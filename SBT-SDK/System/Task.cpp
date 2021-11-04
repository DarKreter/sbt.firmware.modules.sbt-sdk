//
// Created by jazon on 10/30/21.
//

#include "Task.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace SBT::System {

Task::Task(std::string name, size_t priority)
    : _name(name), _priority(priority) {}

void Task ::executeTask() {
  initialize();

  while (true) {
    run();
  }
}

const char* Task::getName() const { return _name.c_str(); }

size_t Task::getPriority() const { return _priority; }

PeriodicTask::PeriodicTask(std::string name, size_t priority,
                           size_t periodicity)
    : Task(name, priority), _periodicity(periodicity) {}

void PeriodicTask::executeTask() {
  initialize();

  while (true) {
    run();
    vTaskDelay(_periodicity);
  }
}

} // namespace SBT::System

#include "Task.hpp"
