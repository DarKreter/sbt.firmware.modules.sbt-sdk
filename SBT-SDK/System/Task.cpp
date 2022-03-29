//
// Created by jazon on 10/30/21.
//

#include "Task.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace SBT::System {

Task::Task(const std::string& name, size_t priority, size_t stackDepth)
    : _name(name), _priority(priority), _stackDepth(stackDepth)
{
}

void Task ::executeTask()
{
    while(true) {
        run();
    }
}

const char* Task::getName() const { return _name.c_str(); }

size_t Task::getPriority() const { return _priority; }

size_t Task::getStackDepth() const { return _stackDepth; }

PeriodicTask::PeriodicTask(const std::string& name, size_t priority,
                           size_t periodicity, size_t stackDepth)
    : Task(name, priority, stackDepth), _periodicity(periodicity)
{
}

void PeriodicTask::executeTask()
{
    while(true) {
        run();
        vTaskDelay(_periodicity);
    }
}

} // namespace SBT::System
