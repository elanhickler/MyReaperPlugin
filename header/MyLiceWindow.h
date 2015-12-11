#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"

class LiceControl
{
public:
	LiceControl(HWND parent);
	virtual ~LiceControl() {}
	HWND getWindowHandle() const { return m_hwnd; }
	virtual void paint(LICE_IBitmap*) = 0;
private:
	HWND m_hwnd = NULL;
};

// Development test control
class TestControl : public LiceControl
{
public:
	void paint(LICE_IBitmap* bm) override;
};
