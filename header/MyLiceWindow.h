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
	
	virtual void paint(LICE_IBitmap*) = 0;
	
	// TODO: pass mouse button and key modifiers states...
	virtual void mousePressed(int x, int y) {}
	virtual void mouseMoved(int x, int y) {}
	virtual void mouseReleased(int x, int y) {}
	virtual void mouseWheel(int x, int y, int delta) {}
	
	void setSize(int w, int h);
	void setBounds(int x, int y, int w, int h);
	int getWidth() const;
	int getHeight() const;
	void repaint();
	
	// for nefarious purposes. use responsibly.
	HWND getWindowHandle() const { return m_hwnd; }
private:
	HWND m_hwnd = NULL;
	static LRESULT WINAPI wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	std::unique_ptr<LICE_SysBitmap> m_bitmap;
};

// Development test control
class TestControl : public LiceControl
{
public:
	TestControl(HWND parent, bool delwhendraggedoutside=false) : 
		LiceControl(parent), m_delete_point_when_dragged_outside(delwhendraggedoutside) {}
	void paint(LICE_IBitmap* bm) override;
	void mousePressed(int x, int y) override;
	void mouseMoved(int x, int y) override;
	void mouseReleased(int x, int y) override;
	void mouseWheel(int x, int y, int delta) override;
private:
	struct point
	{
		point() {}
		point(int x, int y) : m_x(x), m_y(y) {}
		int m_x = 0;
		int m_y = 0;
	};
	std::vector<point> m_points;
	int find_hot_point(int x, int y);
	int m_hot_point = -1;
	float m_circlesize = 10.0f;
	bool m_mousedown = false;
	bool m_delete_point_when_dragged_outside = false;
};
