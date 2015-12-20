#include "reascriptgui.h"
#include "../Visual Studio/resource.h"
#ifndef _WIN32 // MAC resources
#include "WDL/WDL/swell/swell-dlggen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_dlg"
#undef BEGIN
#undef END
#include "WDL/WDL/swell/swell-menugen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_menu"
#endif

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include "MyLiceWindow.h"
#include <unordered_map>
#include <memory>
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#endif

extern HINSTANCE g_hInst;

std::unordered_map<HWND, ReaScriptWindow*> g_reascriptwindowsmap;

ReaScriptWindow::ReaScriptWindow(std::string title)
{
	HWND parent = GetMainHwnd();
	m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_REASCRIPTDIALOG1), parent, dlgproc, (LPARAM)this);
	if (m_hwnd == NULL)
	{
		readbg() << "failed to create reascript dialog\n";
		return;
	}
	control_t c;
	/*
	c.m_name = "Line edit 1";
	c.m_hwnd = CreateWindow("EDIT", c.m_name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd, 0, g_hInst, 0);
	ShowWindow(c.m_hwnd, SW_SHOW);
	m_controls.push_back(c);
	*/
	/*
	for (int i = 0; i < 4; ++i)
	{
	std::string cname = "Slider " + std::to_string(i+1);
	int ycor = 40 + i * 25;
	c.m_hwnd = CreateWindow("REAPERhfader", cname.c_str(), WS_CHILD | WS_TABSTOP, 5, ycor, 290, 20, m_hwnd, 0, g_hInst, 0);
	ShowWindow(c.m_hwnd, SW_SHOW);
	c.m_name = cname;
	m_controls.push_back(c);
	}
	*/

	SetWindowText(m_hwnd, title.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
	SetWindowPos(m_hwnd, NULL, 20, 60, 300, 160, SWP_NOACTIVATE | SWP_NOZORDER);
}

ReaScriptWindow::~ReaScriptWindow()
{
	// These should of course be deleted, but there might be some subtleties involved,
	// postpone figuring out all that...
	//for (auto& e : m_controls)
	//	delete e.m_licecontrol;
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
		g_reascriptwindowsmap.erase(m_hwnd);
	}
}

void ReaScriptWindow::add_slider(std::string name, int initialvalue)
{
	int ycor = 5 + m_controls.size() * 25;
	control_t c;
#ifdef WIN32
	c.m_hwnd = CreateWindow("REAPERhfader", name.c_str(), WS_CHILD | WS_TABSTOP, 5, ycor, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
#else
	c.m_hwnd = SWELL_MakeControl("REAPERhfader", m_control_id_count, "REAPERhfader", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, 0);
	if (c.m_hwnd == NULL)
		readbg() << "could not create reaper fader";

	SetParent(c.m_hwnd, m_hwnd);
#endif
	SendMessage(c.m_hwnd, TBM_SETPOS, 0, (LPARAM)initialvalue);
	SendMessage(c.m_hwnd, TBM_SETTIC, 0, 500);
	ShowWindow(c.m_hwnd, SW_SHOW);
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_button(std::string name, std::string text)
{
	control_t c;
#ifdef WIN32
	c.m_hwnd = CreateWindow("BUTTON", name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
#else
	c.m_hwnd = SWELL_MakeButton(0, text.c_str(), m_control_id_count, 0, 0, 20, 20, WS_CHILD | WS_TABSTOP);
	SetParent(c.m_hwnd, m_hwnd);
#endif
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	SetWindowText(c.m_hwnd, text.c_str());
	ShowWindow(c.m_hwnd, SW_SHOW);
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_line_edit(std::string name, std::string text)
{
	control_t c;
#ifdef WIN32
	c.m_hwnd = CreateWindow("EDIT", name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
#else
	c.m_hwnd = SWELL_MakeEditField(m_control_id_count, 0, 0, 50, 20, WS_CHILD | WS_TABSTOP);
	SetParent(c.m_hwnd, m_hwnd);
#endif
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	SetWindowText(c.m_hwnd, text.c_str());
	ShowWindow(c.m_hwnd, SW_SHOW);
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_label(std::string name, std::string inittext)
{
	control_t c;
#ifdef WIN32
	c.m_hwnd = CreateWindow("STATIC", name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
#else
	c.m_hwnd = SWELL_MakeLabel(-1, inittext.c_str(), m_control_id_count, 0, 0, 20, 20, 0);
	SetParent(c.m_hwnd, m_hwnd);
#endif
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	SetWindowText(c.m_hwnd, inittext.c_str());
	ShowWindow(c.m_hwnd, SW_SHOW);
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_custom_control(std::string name, std::string controltype)
{
	control_t c;
	// This is a stupid way to do this. Will really need a proper factory pattern
	// for the LiceControls
	if (controltype == "MultiXYControl")
	{
		c.m_licecontrol = new TestControl(m_hwnd);
	}
	if (controltype == "WaveformControl")
	{
		c.m_licecontrol = new WaveformControl(m_hwnd);
		c.m_licecontrol->ChangeNotifyCallback = [this, name](std::string reason)
		{
			//readbg() << name << " : " << reason << "\n";
			m_last_used_controls.insert(name);
		};
	}
	if (c.m_licecontrol != nullptr)
	{
		c.m_hwnd = c.m_licecontrol->getWindowHandle();
		ShowWindow(c.m_hwnd, SW_SHOW);
	}
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::setWindowTitle(std::string title)
{
	if (m_hwnd != NULL)
	{
		SetWindowText(m_hwnd, title.c_str());
	}
}

const char* ReaScriptWindow::getControlText(std::string cname)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		static char buf[1024];
		GetWindowText(c->m_hwnd, buf, 1024);
		return buf;
	}
	return "";
}

void ReaScriptWindow::setControlText(std::string cname, std::string txt)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		SetWindowText(c->m_hwnd, txt.c_str());
	}

}

double ReaScriptWindow::getControlValueDouble(std::string cname, int which)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		if (c->m_licecontrol==nullptr)
			return SendMessage(c->m_hwnd, TBM_GETPOS, 0, 0);
		else 
			return c->m_licecontrol->getFloatingPointProperty(which);
	}
	return 0.0;
}

void ReaScriptWindow::setControlValueDouble(std::string cname, int which, double v)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		if (c->m_licecontrol != nullptr)
			c->m_licecontrol->setFloatingPointProperty(which, v);
	}
}

bool ReaScriptWindow::isControlDirty(std::string name)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
		return c->m_dirty;
	return false;
}

void ReaScriptWindow::cleanControl(std::string name)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
		c->m_dirty = false;
}

ReaScriptWindow::control_t* ReaScriptWindow::controlFromName(std::string name)
{
	for (auto& e : m_controls)
		if (e.m_name == name)
			return &e;
	return nullptr;
}

void ReaScriptWindow::setControlBounds(std::string name, int x, int y, int w, int h)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
	{
		if (c->m_licecontrol == nullptr)
			SetWindowPos(c->m_hwnd, NULL, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
		else c->m_licecontrol->setBounds(x, y, w, h);
	}
}

int ReaScriptWindow::getBoundsValue(int which)
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	if (which == 0)
		return r.left;
	if (which == 1)
		return r.top;
	if (which == 2)
		return r.right - r.left;
	if (which == 3)
		return r.bottom - r.top;
	return 0;
}

INT_PTR CALLBACK ReaScriptWindow::dlgproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_INITDIALOG)
	{
		ReaScriptWindow* wptr = (ReaScriptWindow*)lparam;
		g_reascriptwindowsmap[hwnd] = wptr;
		return TRUE;
	}
	ReaScriptWindow* wptr = get_from_map(g_reascriptwindowsmap, hwnd);
	if (wptr == nullptr)
		return FALSE;
	if (msg == WM_COMMAND)
	{
		if (HIWORD(wparam) == BN_CLICKED || HIWORD(wparam) == EN_CHANGE)
		{
			for (auto& e : wptr->m_controls)
				if (e.m_control_id == LOWORD(wparam))
				{
					wptr->m_last_used_controls.insert(e.m_name);
					return TRUE;
				}
		}
		wptr->m_window_dirty = true;
		return TRUE;
	}
	if (msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		for (auto& e : wptr->m_controls)
			if (e.m_hwnd == (HWND)lparam)
			{
				wptr->m_last_used_controls.insert(e.m_name);
				return TRUE;
			}
		wptr->m_window_dirty = true;
		return TRUE;
	}
	if (msg == WM_SIZE)
	{
		wptr->m_was_resized = true;
		return TRUE;
	}
	if (msg == WM_CLOSE)
	{
		g_reascriptwindowsmap.erase(hwnd);
		wptr->m_hwnd = NULL;
		DestroyWindow(hwnd);
		wptr->m_was_closed = true;
		return TRUE;
	}
	return FALSE;
}

HWND g_reascript_gui = NULL;

ReaScriptWindow* open_reascript_test_gui(std::string title)
{
	ReaScriptWindow* result = new ReaScriptWindow(title);
	return result;
}