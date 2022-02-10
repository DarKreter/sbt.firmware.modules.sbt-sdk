#include "TaskManager.hpp"

#include <FreeRTOS.h>
#include <queue.h>

namespace SBT::System {
std::vector<std::shared_ptr<Task>> TaskManager::_tasks;

void TaskManager::registerTask(const std::shared_ptr<Task>& task)
{
    _tasks.push_back(task);
}

void TaskManager::startTasks()
{
    // This method simply calls main task's method that executes that task
    static const auto taskEntryPoint = [](void* task) {
        reinterpret_cast<Task*>(task)->executeTask();
    };

    // Create all task by calling executeTask(). This is done by passing task
    // pointer to taskEntryPoint.
    for(const auto& task : _tasks) {
        xTaskCreate(taskEntryPoint, task->getName(), task->getStackDepth(),
                    task.get(), task->getPriority(), nullptr);
    }
}

// void TaskManager::startRtos() { vTaskStartScheduler(); }
} // namespace SBT::System
