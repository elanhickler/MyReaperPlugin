#include "mrpwincontrols.h"
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#endif

extern HINSTANCE g_hInst;

int g_control_counter = 0;

WinControl::WinControl(HWND parent)
{
	m_parent = parent;
	++g_control_counter;
	m_control_id = g_control_counter;
}

WinControl::~WinControl()
{
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
	}
}

int WinControl::getWidth() const
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	return r.right - r.left;
}

int WinControl::getHeight() const
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	return r.bottom - r.top;
}

void WinControl::setBounds(int x, int y, int w, int h)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void WinControl::setTopLeftPosition(int x, int y)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void WinControl::setSize(int w, int h)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

WinButton::WinButton(HWND parent, std::string text) :
	WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("BUTTON", "button", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent,
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeButton(0, text.c_str(), g_control_counter, 0, 0, 20, 20, WS_CHILD | WS_TABSTOP);
	SetParent(m_hwnd, parent);
#endif
	SetWindowText(m_hwnd, text.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
	GenericNotifyCallback = [this](GenericNotifications)
	{
		readbg() << "button " << getText() << " clicked\n";
	};
}

void WinButton::setText(std::string text)
{
	SetWindowText(m_hwnd, text.c_str());
}

std::string WinButton::getText()
{
	char buf[1024];
	GetWindowText(m_hwnd, buf, 1024);
	return std::string(buf);
}

bool WinButton::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_COMMAND && LOWORD(wparam) == m_control_id
		&& HIWORD(wparam) == BN_CLICKED && GenericNotifyCallback)
	{
		GenericNotifyCallback(GenericNotifications::Clicked);
		return true;
	}
	return false;
}

WinLabel::WinLabel(HWND parent, std::string text) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("STATIC", "label", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent,
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwd = SWELL_MakeLabel(-1, text.c_str(), g_control_counter, 0, 0, 20, 20, 0);
	SetParent(m_hwnd, parent);
#endif
	SetWindowText(m_hwnd, text.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
}
// Slightly annoying redundancy here with the WinButton methods...
void WinLabel::setText(std::string text)
{
	SetWindowText(m_hwnd, text.c_str());
}

std::string WinLabel::getText()
{
	char buf[1024];
	GetWindowText(m_hwnd, buf, 1024);
	return std::string(buf);
}

ReaSlider::ReaSlider(HWND parent, int initpos) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("REAPERhfader", "slider", WS_CHILD | WS_TABSTOP, 5, 5, 30, 10, parent,
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeControl("REAPERhfader", g_control_counter, "REAPERhfader",
		WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, 0);
	SetParent(m_hwnd, parent);
#endif
	SendMessage(m_hwnd, TBM_SETPOS, 0, (LPARAM)initpos);
	SendMessage(m_hwnd, TBM_SETTIC, 0, 500);
	ShowWindow(m_hwnd, SW_SHOW);
}

bool ReaSlider::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		if ((HWND)lparam == m_hwnd)
		{
			if (SliderValueCallback)
			{
				int pos = SendMessage((HWND)lparam, TBM_GETPOS, 0, 0);
				SliderValueCallback(pos);
				return true;
			}
		}
	}
	return false;
}

int ReaSlider::getPosition()
{
	if (m_hwnd != NULL)
	{
		return SendMessage(m_hwnd, TBM_GETPOS, 0, 0);
	}
	return 0;
}

void ReaSlider::setPosition(int pos)
{
	if (m_hwnd != NULL)
	{
		SendMessage(m_hwnd, TBM_SETPOS, 0, (LPARAM)pos);
	}
}

void ReaSlider::setTickMarkPosition(int pos)
{
	SendMessage(m_hwnd, TBM_SETTIC, 0, pos);
}