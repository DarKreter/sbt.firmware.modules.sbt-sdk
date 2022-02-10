#ifndef TASKMANAGER_HPP
#define TASKMANAGER_HPP

#include <array>
#include <memory>
#include <vector>

#include "Task.hpp"

// Abstract class that wraps user's tasks.
// If user wants to create new task:
// Create object derived from Task
// Configure Task using Task's constructor
// Implement initialize() and run()
// Add this new task to TaskManager's tasks vector in registerTasks method

namespace SBT::System {
class TaskManager {
public:
    static void registerTask(const std::shared_ptr<Task>& task);

    // Register all tasks in FreeRTOS - allocate local stack etc.
    static void startTasks();
    //
    //  // Start scheduler - this function theoretically should not return
    //  static void startRtos();

private:
    static std::vector<std::shared_ptr<Task>> _tasks;
};
} // namespace SBT::System

#endif // TASKMANAGER_HPP
