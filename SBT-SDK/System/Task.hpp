//
// Created by jazon on 10/30/21.
//

#ifndef F1XX_PROJECT_TEMPLATE_TASK_HPP
#define F1XX_PROJECT_TEMPLATE_TASK_HPP

#include <string>

namespace SBT::System {

class Task {
public:
    Task(const std::string& name, size_t priority, size_t stackDepth = 128);

    [[noreturn]] virtual void executeTask();

    [[nodiscard]] const char* getName() const;
    [[nodiscard]] size_t getPriority() const;
    [[nodiscard]] size_t getStackDepth() const;

protected:
    // Setup method that is run single time at the start of the task
    virtual void initialize() = 0;

    // Method that is called once per '_periodicity' milliseconds
    virtual void run() = 0;

protected:
    const std::string _name;
    const size_t _priority;
    const size_t _stackDepth;
};

class PeriodicTask : public Task {
public:
    PeriodicTask(const std::string& name, size_t priority, size_t periodicity,
                 size_t stackDepth = 128);

    [[noreturn]] void executeTask() override;

protected:
    // Setup method that is run single time at the start of the task
    //  virtual void initialize() = 0;

    // Method that is called once per '_periodicity' milliseconds
    //  virtual void run() = 0;

protected:
    const size_t _periodicity;
};

} // namespace SBT::System

#endif // F1XX_PROJECT_TEMPLATE_TASK_HPP
