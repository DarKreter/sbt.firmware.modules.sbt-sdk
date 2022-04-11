//
// Created by hubert25632 on 24.02.2022.
//

#ifndef F1XX_PROJECT_TEMPLATE_MAINBOARDS_HPP
#define F1XX_PROJECT_TEMPLATE_MAINBOARDS_HPP

// Boards
#define SBT_MAINBOARD_ALLREMAP 1
#define SBT_MAINBOARD_OLD      2
#define SBT_MAINBOARD_MINI_V1  3
#define SBT_MAINBOARD_MINI_V2  4

#ifndef SBT_BSP_MAINBOARD
#define SBT_BSP_MAINBOARD SBT_MAINBOARD_OLD
#endif

#if SBT_BSP_MAINBOARD == SBT_MAINBOARD_ALLREMAP
#include "Mainboards/AllRemapped.hpp"
#elif SBT_BSP_MAINBOARD == SBT_MAINBOARD_OLD
#include "Mainboards/Old.hpp"
#elif SBT_BSP_MAINBOARD == SBT_MAINBOARD_MINI_V1
#include "Mainboards/MiniV1.hpp"
#elif SBT_BSP_MAINBOARD == SBT_MAINBOARD_MINI_V2
#include "Mainboards/MiniV2.hpp"
#else
#error "Invalid mainboard selected"
#endif

#endif // F1XX_PROJECT_TEMPLATE_MAINBOARDS_HPP
