#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#include <memory>
#include <vector>

class LiceControl
{
public:
	LiceControl(HWND parent);
	virtual ~LiceControl();
	HWND getWindowHandle() const { return m_hwnd; }
	virtual void paint(LICE_IBitmap*) = 0;
	virtual void mousePressed(int x, int y) {}
	void setSize(int w, int h);
	void repaint();
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
	void mousePressed(int x, int y) override;
private:
	struct point
	{
		point() {}
		point(int x, int y) : m_x(x), m_y(y) {}
		int m_x = 0;
		int m_y = 0;
	};
	std::vector<point> m_points;
};
