#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#include <memory>
#include <vector>
#include <functional>

class MouseEvent
{
public:
	enum MouseButton { MBLeft, MBRight, MBMiddle };
	MouseEvent() {}
	MouseEvent(int x, int y, MouseButton mb, int wheel = 0) :
		m_x(x), m_y(y), m_mb(mb), m_wheel(wheel) {}
	int m_x = 0;
	int m_y = 0;
	int m_wheel = 0;
	MouseButton m_mb = MBLeft;
};

class LiceControl
{
public:
	LiceControl(HWND parent);
	virtual ~LiceControl();
	
	virtual void paint(LICE_IBitmap*) = 0;
	
	// TODO: pass mouse button and key modifiers states...
	virtual void mousePressed(const MouseEvent& ev) {}
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
	void mousePressed(const MouseEvent& ev) override;
	void mouseMoved(int x, int y) override;
	void mouseReleased(int x, int y) override;
	void mouseWheel(int x, int y, int delta) override;
	std::function<void(int, double, double)> PointMovedCallback;
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

class PopupMenu
{
public:
	PopupMenu(HWND parent);
	~PopupMenu();
	void add_menu_item(std::string txt, std::function<void(void)> action);
	void execute(int x, int y);
private:
	struct menu_entry_t
	{
		std::string m_text;
		std::function<void(void)> m_f;
	};
	HWND m_hwnd = NULL;
	HMENU m_menu = NULL;
	std::vector<menu_entry_t> m_entries;
};
