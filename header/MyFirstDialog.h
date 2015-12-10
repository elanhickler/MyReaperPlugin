#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

HWND open_my_first_modeless_dialog(HWND parent);
