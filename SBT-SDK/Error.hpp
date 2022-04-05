//
// Created by hubert25632 on 12.02.2022.
//

#ifndef F1XX_PROJECT_TEMPLATE_ERROR_HPP
#define F1XX_PROJECT_TEMPLATE_ERROR_HPP

#include <string>

void softfault(const std::string& fileName, const int& lineNumber,
               const std::string& comment);

void softfault(const std::string& comment);

#endif // F1XX_PROJECT_TEMPLATE_ERROR_HPP
