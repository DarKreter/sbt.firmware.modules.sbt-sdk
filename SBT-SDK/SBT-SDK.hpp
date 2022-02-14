//
// Created by jazon on 10/30/21.
//

#ifndef F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP
#define F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP

#include "TaskManager.hpp"

#include <string>

namespace SBT::System {
enum class SystemState {
    Running,
    SoftFault
};

void Init();

void Start(unsigned watchdogTimeout_ms = 1000);

void SystickHandler();
} // namespace SBT::System

#endif // F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP
