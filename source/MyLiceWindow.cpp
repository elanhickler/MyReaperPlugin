#include "MyLiceWindow.h"
#include <unordered_map>
#include "WDL/WDL/lice/lice.h"
#include "../library/reaper_plugin/reaper_plugin_functions.h"

std::unordered_map<HWND, LiceControl*> g_controlsmap;

extern HINSTANCE g_hInst;

// creates a plain child window (control).
// wndProc will receive a WM_CREATE, but it will have lParam set to lParamContext rather than LPCREATESTRUCT
// (lParam is passed directly on WM_CREATE in SWELL, this duplicates that behavior)
HWND SWELL_CreatePlainWindow(HINSTANCE hInstance, HWND parent, WNDPROC wndProc, LPARAM lParamContext)
{
#ifdef _WIN32
#define SWELL_GENERIC_CONTROL_CLASS_NAME "SWELLPlainChildWindow"
	static bool reg;
	struct parms {
		WNDPROC newWndProc;
		LPARAM context;

		static LRESULT WINAPI tmpProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (uMsg != WM_CREATE) return DefWindowProc(hwnd, uMsg, wParam, lParam);

			const parms *_this = (const parms *)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LPARAM)_this->newWndProc);
			return _this->newWndProc(hwnd, uMsg, wParam, _this->context);
		}
	} p = { wndProc, lParamContext };

	if (!reg)
	{
		WNDCLASS wc = { CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW,parms::tmpProc, 0, 0, hInstance, NULL, NULL, (HBRUSH)0, NULL, SWELL_GENERIC_CONTROL_CLASS_NAME };
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		RegisterClass(&wc);
		reg = true;
	}
	return CreateWindowEx(0, SWELL_GENERIC_CONTROL_CLASS_NAME, "", WS_CHILD, 0, 0, 1, 1, parent, NULL, hInstance, &p);

#else
	return SWELL_CreateDialog(NULL, NULL, parent, (DLGPROC)wndProc, lParamContext);
#endif
}

void TestControl::paint(LICE_IBitmap * bm)
{
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	for (auto& e : m_points)
	{
		LICE_FillCircle(bm, e.m_x, e.m_y, m_circlesize, LICE_RGBA(255, 255, 255, 255));
	}
}

void TestControl::mousePressed(int x, int y)
{
	m_points.push_back({ x,y });
	repaint();
}

void TestControl::mouseMoved(int x, int y)
{
	//char buf[100];
	//sprintf(buf, "(%d %d)", x, y);
	//ShowConsoleMsg(buf);
}

void TestControl::mouseReleased(int x, int y)
{
}

void TestControl::mouseWheel(int x, int y, int delta)
{
	char buf[100];
	sprintf(buf,"mousewheel %d\n",delta);
	//ShowConsoleMsg(buf);
	float temp = 1.0f;
	if (delta>32768)
		temp=-1.0f;
	m_circlesize += temp;
	if (m_circlesize<1.0f)
		m_circlesize = 1.0f;
	if (m_circlesize>100.0f)
		m_circlesize=100.0f;
	repaint();
}

LiceControl::LiceControl(HWND parent)
{
	m_hwnd = SWELL_CreatePlainWindow(g_hInst, parent, wndproc, NULL);
	g_controlsmap[m_hwnd] = this;
	m_bitmap = std::make_unique<LICE_SysBitmap>(100, 100);
	setSize(100, 100);
	ShowWindow(m_hwnd, SW_SHOW);
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

LRESULT LiceControl::wndproc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LiceControl* c = nullptr;
	if (g_controlsmap.count(hwnd)>0)
		c=g_controlsmap[hwnd];
	else return DefWindowProc(hwnd, Message, wParam, lParam);;
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
		return 0;
	}
	if (Message == WM_LBUTTONDOWN)
	{
		// LOWORD/HIWORD not really technically correct for this...but good enough for now
		c->mousePressed(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	if (Message == WM_LBUTTONUP)
	{
		c->mouseReleased(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	if (Message == WM_MOUSEMOVE)
	{
		c->mouseMoved(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	if (Message == WM_MOUSEWHEEL)
	{
		c->mouseWheel(0,0, HIWORD(wParam));
		return 0;
	}
	if (Message == WM_DESTROY)
	{
		ShowConsoleMsg("lice control window destroy\n");
		g_controlsmap.erase(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, Message, wParam, lParam);
	//return CallWindowProc(c->m_origwndproc, hwnd, Message, wParam, lParam);
}
