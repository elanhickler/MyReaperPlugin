#pragma once

#ifdef _WIN32
#include <windows.h>
#include "WDL/WDL/win32_utf8.h"
#else
#include "WDL/WDL/swell/swell.h"
#endif

class MRPWindow;

#include "WDL/WDL/lice/lice.h"
#include "WDL/WDL/lice/lice_text.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <array>
#include "mrpwincontrols.h"

int acProc(MSG *msg, accelerator_register_t *ctx);

enum
{
	KEY_DOWN = 4096,
	KEY_UP,
	KEY_PPAGE,
	KEY_NPAGE,
	KEY_RIGHT,
	KEY_LEFT,
	KEY_HOME,
	KEY_END,
	KEY_IC,
	KEY_DC,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
};

#define KEY_BACKSPACE '\b'

#define KEY_F(x) (KEY_F1 + (x) - 1)


enum ModifierKey
{
	MKNone,
	MKShift,
	MKControl,
	MKAlt,
	MKAppleOrWindowsKey // not sure if this can be nicely supported, but added here anyway...
};

class ModifierKeys
{
public:
	ModifierKeys() 
	{ 
		// std::array doesn't explicitly default construct non-class elements!
		// so we have to reset it manually to zeros.
		reset(); 
	}
	ModifierKeys(bool shiftdown, bool controldown, bool altdown, bool winappledown = false)
	{
		reset();
		if (shiftdown == true)
			m_keys[0] = 1;
		if (controldown == true)
			m_keys[1] = 1;
		if (altdown == true)
			m_keys[2] = 1;
		if (winappledown == true)
			m_keys[3] = 1;
	}

	void reset()
	{
		for (int i = 0; i < m_keys.size(); ++i)
			m_keys[i] = 0;
	}
	bool noneDown() const
	{
		return std::all_of(m_keys.begin(), m_keys.end(), [](char x) { return x == 0; });
	}
	bool isModifierKeyDown(ModifierKey k) const
	{
		if (k == MKShift && m_keys[0] != 0)
			return true;
		if (k == MKControl && m_keys[1] != 0)
			return true;
		if (k == MKAlt && m_keys[2] != 0)
			return true;
		if (k == MKAppleOrWindowsKey && m_keys[3] != 0)
			return true;
		return false;
	}
	bool areModifiersDown(const std::initializer_list<ModifierKey>& ks) const
	{
		int cnt = 0;
		for (auto& e : ks)
			if (isModifierKeyDown(e) == true)
				++cnt;
		if (cnt == ks.size())
			return true;
		return false;
	}
	void setModifierDown(ModifierKey k, bool b)
	{
		if (k == MKShift)
			m_keys[0] = b;
		if (k == MKControl)
			m_keys[1] = b;
		if (k == MKAlt)
			m_keys[2] = b;
		if (k == MKAppleOrWindowsKey)
			m_keys[3] = b;
	}
private:
	// could maybe use std::bitset here but probably not worth it...
	std::array<char, 4> m_keys;
};

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
	ModifierKeys m_modkeys;
private:
};

class PaintEvent
{
public:
	PaintEvent() {}
	PaintEvent(LICE_IBitmap* thebm) : bm(thebm) {}
	LICE_IBitmap* bm = nullptr;
};

/*
Base class for implementing your own completely custom controls aka widgets. So, useful for things like 
XY-controls, breakpoint envelope editors, spectral displays etc. These create an underlying win32 
window and Lice system bitmap, so you probably should not go overboard instantiating these...
It could get expensive. Likely hundreds can be instantiated without too many problems, though.
 
The paint virtual function is pure, so you are always required to implement that in your subclasses. 
An empty control wouldn't be much fun to look at anyway.
*/
class LiceControl : public WinControl
{
public:
	LiceControl(MRPWindow* parent);
    ~LiceControl();

	// The bitmap to be used for drawing is passed into the method with the PaintEvent instead of
	// the method using the object's bitmap directly. This may be useful in the future,
	// although for now the LiceControl's internal bitmap is always passed.
	// Currently we don't do any fancy detection and passing of dirty subareas etc so the paint method
	// needs to just draw everything from scratch each time it is called. Code that ends up causing
	// the paint calls, like repaint() or control size changing should be careful to not do it too much.
	// Alternatively LiceControl subclasses could do their own internal caching of drawn things etc...
	virtual void paint(PaintEvent& ev) = 0;

	virtual void mousePressed(const MouseEvent& ev) {}
	virtual void mouseDoubleClicked(const MouseEvent& ev) {}
	virtual void mouseMoved(const MouseEvent& ev) {}
	virtual void mouseReleased(const MouseEvent& ev) {}
	virtual void mouseLeave() {}
	virtual void mouseWheel(int x, int y, int delta) {}

	void setSize(int w, int h);
	void setBounds(MRP::Rectangle g) override;
	
	void repaint();

	bool wantsFocus() const { return m_wants_focus; }
	void setWantsFocus(bool b) { m_wants_focus = b; }
	bool hasFocus() const;
	void setFocused();

	virtual bool keyPressed(const ModifierKeys& modkeys, int keycode) { return false; }

	bool isCursorOver();

	virtual std::string getType() const = 0;

private:
	static LRESULT WINAPI wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	std::unique_ptr<LICE_SysBitmap> m_bitmap;
	bool m_wants_focus = true;
	accelerator_register_t m_acreg;
	HCURSOR m_cursor = NULL;
};

class PopupMenu
{
public:
	enum CheckState { NotCheckable, Unchecked, Checked };
	struct menu_entry_t
	{
		std::string m_text;
		std::function<void(CheckState)> m_f;
		CheckState m_checkstate = NotCheckable;
		std::shared_ptr<PopupMenu> m_submenu;
		int m_id = 0;
	};
	HWND m_hwnd = NULL;
	HMENU m_menu = NULL;
	std::vector<menu_entry_t> m_entries;
	std::function<void(void)> m_none_chosen_f;
	
	PopupMenu(HWND parent);
	PopupMenu(const PopupMenu&) = delete;
	PopupMenu& operator=(const PopupMenu&) = delete;
	PopupMenu(PopupMenu&& other);
	~PopupMenu();
	void add_menu_item(std::string txt, std::function<void(CheckState)> action);
	void add_menu_item(std::string txt, CheckState cs, std::function<void(CheckState)> action);
	void add_submenu(std::string txt, PopupMenu& menu);
	void set_none_chosen_handler(std::function<void(void)> action);
	void execute(int x, int y, bool use_screen_coordinates = false);
	HMENU getMenuHandle() const { return m_menu; }
	void setMenuHandle(HMENU mh) { m_menu = mh; }
	bool m_is_submenu = false;
private:
	
	
};
// Wrapper for Windows timer. This isn't tied to a window and the
// WM_TIMER messages, so you can create these wherever a timer can work,
// namely when the win32 message loop is free to run
class Timer
{
public:
	Timer() {}
	~Timer();
	void start(int milliseconds);
	void stop();
	void set_callback(std::function<void(void)> f)
	{
		m_callback = f;
	}
private:
	UINT_PTR m_id = 0;
	std::function<void(void)> m_callback;
	static void CALLBACK thetimerproc(HWND hwnd, UINT uMsg,UINT_PTR idEvent, DWORD dwTime);
};


