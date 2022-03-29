#include "TaskManager.hpp"
#include "Error.hpp"

#include <FreeRTOS.h>
#include <queue.h>

namespace SBT::System {
std::vector<std::shared_ptr<Task>> TaskManager::_tasks;

void TaskManager::registerTask(const std::shared_ptr<Task>& task)
{
    if(task->getPriority() > 7)
        softfault(
            __FILE__, __LINE__,
            "Only system tasks are allowed to have priority greater than 7");
    _tasks.push_back(task);
}

void TaskManager::registerSystemTask(const std::shared_ptr<Task>& task)
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

void TaskManager::TasksInit()
{
    for(const auto& task : _tasks)
        task->initialize();
}

// void TaskManager::startRtos() { vTaskStartScheduler(); }
} // namespace SBT::System
