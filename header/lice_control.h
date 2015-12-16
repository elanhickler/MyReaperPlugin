#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <array>

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

class LiceControl
{
public:
	LiceControl(HWND parent);
	virtual ~LiceControl();

	// The bitmap to be used for drawing is passed into the method instead of
	// the method using the object's bitmap directly. This may be useful in the future,
	// although for now the LiceControl's internal bitmap is always passed.
	virtual void paint(LICE_IBitmap*) = 0;

	virtual void mousePressed(const MouseEvent& ev) {}
	virtual void mouseMoved(const MouseEvent& ev) {}
	virtual void mouseReleased(int x, int y) {}
	virtual void mouseWheel(int x, int y, int delta) {}

	void setSize(int w, int h);
	void setBounds(int x, int y, int w, int h);
	int getWidth() const;
	int getHeight() const;
	void repaint();

	bool wantsFocus() const { return m_wants_focus; }
	void setWantsFocus(bool b) { m_wants_focus = b; }
	bool hasFocus() const;
	void setFocused();

	virtual bool keyPressed(int keycode) { return false; }

	// Use this responsibly.
	HWND getWindowHandle() const { return m_hwnd; }
private:
	HWND m_hwnd = NULL;
	HWND m_parenthwnd = NULL;
	static LRESULT WINAPI wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	std::unique_ptr<LICE_SysBitmap> m_bitmap;
	bool m_wants_focus = true;
	accelerator_register_t m_acreg;
};

class PopupMenu
{
public:
	PopupMenu(HWND parent);
	~PopupMenu();
	void add_menu_item(std::string txt, std::function<void(void)> action);
	void set_none_chosen_handler(std::function<void(void)> action);
	void execute(int x, int y, bool use_screen_coordinates = false);
private:
	struct menu_entry_t
	{
		std::string m_text;
		std::function<void(void)> m_f;
	};
	HWND m_hwnd = NULL;
	HMENU m_menu = NULL;
	std::vector<menu_entry_t> m_entries;
	std::function<void(void)> m_none_chosen_f;
};

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
	static VOID CALLBACK thetimerproc(HWND hwnd, UINT uMsg,UINT_PTR idEvent, DWORD dwTime);
};
