#include "MyLiceWindow.h"
#include <map>
#include "WDL/WDL/lice/lice.h"
#include "../library/reaper_plugin/reaper_plugin_functions.h"

std::map<HWND, LiceControl*> g_controlsmap;

extern HINSTANCE g_hInst;

void TestControl::paint(LICE_IBitmap * bm)
{
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	//LICE_Line(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(255, 255, 255, 255));
	for (auto& e : m_points)
	{
		LICE_FillCircle(bm, e.m_x, e.m_y, 10.0f, LICE_RGBA(255, 255, 255, 255));
	}
}

void TestControl::mousePressed(int x, int y)
{
	m_points.push_back({ x,y });
	repaint();
}

LiceControl::LiceControl(HWND parent)
{
	HWND w = CreateWindow("STATIC", "", WS_CHILD|SS_NOTIFY|SS_BLACKFRAME, 0, 0, 100, 100, parent, NULL, g_hInst, NULL);
	if (w == NULL)
	{
		ShowConsoleMsg("Could not create window for Lice control\n");
	}
	else
	{
		m_hwnd = w;
		g_controlsmap[m_hwnd] = this;
		m_bitmap = std::make_unique<LICE_SysBitmap>(100, 100);
		m_origwndproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)wndproc);
		ShowWindow(m_hwnd, SW_SHOW);
	}
}

LiceControl::~LiceControl()
{
	ShowConsoleMsg("Lice Control dtor\n");
}

void LiceControl::setSize(int w, int h)
{
	m_bitmap->resize(w, h);
	SetWindowPos(m_hwnd, 0, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void LiceControl::repaint()
{
	InvalidateRect(m_hwnd, NULL, TRUE);
}

BOOL LiceControl::wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LiceControl* c = nullptr;
	if (g_controlsmap.count(hwnd)>0)
		c=g_controlsmap[hwnd];
	else return 0;
	if (Message == WM_PAINT)
	{
		RECT r;
		GetClientRect(hwnd, &r);
		PAINTSTRUCT ps = { 0 };
		HDC dc = BeginPaint(hwnd, &ps);
		c->paint(c->m_bitmap.get());
		BitBlt(dc, r.left,
			r.top,
			c->m_bitmap->getWidth(),
			c->m_bitmap->getHeight(),
			c->m_bitmap->getDC(), 0, 0, SRCCOPY);

		EndPaint(hwnd, &ps);
		return TRUE;
	}
	if (Message == WM_LBUTTONDOWN)
	{
		c->mousePressed(LOWORD(lParam), HIWORD(lParam));
		return TRUE;
	}
	if (Message == WM_DESTROY)
	{
		ShowConsoleMsg("lice control window destroy\n");
		g_controlsmap.erase(hwnd);
		return TRUE;
	}
	return CallWindowProc(c->m_origwndproc, hwnd, Message, wParam, lParam);
}
