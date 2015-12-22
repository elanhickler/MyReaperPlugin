#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include <string>

class WinControl
{
public:
	WinControl(HWND parent);
	virtual ~WinControl();
protected:
	HWND m_hwnd = NULL;
};

class WinButton : public WinControl
{
public:
	WinButton(HWND parent, std::string text);
	void setText(std::string text);
};