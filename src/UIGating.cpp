/**
 * @file UIGating.cpp
 * @brief UI state gating and lock management implementation
 */

#include "UIGating.h"

UIGating uiGate;
bool uiForceSwitchOn = false;

void initUIGating() {
    uiGate = UIGating();
    uiForceSwitchOn = false;
}
