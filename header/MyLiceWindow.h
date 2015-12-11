#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#include <memory>

class LiceControl
{
public:
	LiceControl(HWND parent);
	virtual ~LiceControl();
	HWND getWindowHandle() const { return m_hwnd; }
	virtual void paint(LICE_IBitmap*) = 0;
	void setSize(int w, int h);
private:
	HWND m_hwnd = NULL;
	static BOOL WINAPI wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	WNDPROC m_origwndproc = nullptr;
	std::unique_ptr<LICE_SysBitmap> m_bitmap;
};

// Development test control
class TestControl : public LiceControl
{
public:
	TestControl(HWND parent) : LiceControl(parent) {}
	void paint(LICE_IBitmap* bm) override;
};
