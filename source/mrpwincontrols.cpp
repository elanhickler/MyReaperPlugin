#include "mrpwincontrols.h"
#include "utilfuncs.h"

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

}

void WinControl::setBounds(int x, int y, int w, int h)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

WinButton::WinButton(HWND parent, std::string text) :
	WinControl(parent)
{
	m_hwnd = CreateWindow("BUTTON", "button", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent,
		(HMENU)g_control_counter, g_hInst, 0);
	SetWindowText(m_hwnd, text.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
	GenericNotification = [this](GenericNotifications) 
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
		&& HIWORD(wparam) == BN_CLICKED && GenericNotification)
	{
		GenericNotification(GenericNotifications::Clicked);
		return true;
	}
	return false;
}

WinLabel::WinLabel(HWND parent, std::string text) : WinControl(parent)
{
	m_hwnd = CreateWindow("STATIC", "label", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent,
		(HMENU)g_control_counter, g_hInst, 0);
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
