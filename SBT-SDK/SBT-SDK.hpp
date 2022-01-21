//
// Created by jazon on 10/30/21.
//

#ifndef F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP
#define F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP

#include "Hardware.hpp"
#include "TaskManager.hpp"

#include <string>

namespace SBT {
namespace System {
enum class SystemState {
    Running,
    SoftFault
};

void Init();

void Start();

void SystickHandler();

[[maybe_unused]] void softfault(const std::string& fileName,
                                const int& lineNumber,
                                const std::string& comment);

} // namespace System
} // namespace SBT

#endif // F1XX_PROJECT_TEMPLATE_SBT_SDK_HPP
