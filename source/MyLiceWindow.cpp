#include "MyLiceWindow.h"
#include <unordered_map>
#include "WDL/WDL/lice/lice.h"
#include "../library/reaper_plugin/reaper_plugin_functions.h"
#include "utilfuncs.h"
#include <string>

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

LiceControl::LiceControl(HWND parent)
{
	m_hwnd = SWELL_CreatePlainWindow(g_hInst, parent, wndproc, NULL);
	g_controlsmap[m_hwnd] = this;
	m_bitmap = std::make_unique<LICE_SysBitmap>(200, 200);
	setBounds(20,60,200, 200);
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

void LiceControl::setBounds(int x, int y, int w, int h)
{
	m_bitmap->resize(w, h);
	SetWindowPos(m_hwnd, 0, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}


void LiceControl::repaint()
{
	InvalidateRect(m_hwnd, NULL, TRUE);
}

int LiceControl::getWidth() const
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	return r.right - r.left;
}

int LiceControl::getHeight() const
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	return r.bottom-r.top;
}

bool map_mouse_message(LiceControl* c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//auto foo=std::mem_fn(&LiceControl::mousePressed);
	
	if (msg==WM_LBUTTONDOWN || msg==WM_RBUTTONDOWN || msg==WM_MBUTTONDOWN
		|| msg==WM_MOUSEMOVE || msg==WM_LBUTTONUP || msg==WM_RBUTTONUP || msg==WM_MBUTTONUP)
	{
		// LOWORD/HIWORD are not technically correct according to MSDN
		// However, SWELL does not seem to provide the alternative macros
		// to get the support for multiple monitors. Will need to investigate...
		int x = (short)LOWORD(lParam);
		int y = (short)HIWORD(lParam);
		if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
		{
			SetCapture(hwnd);
			MouseEvent::MouseButton but(MouseEvent::MBLeft);
			if (msg == WM_RBUTTONDOWN)
				but = MouseEvent::MBRight;
			else if (msg == WM_MBUTTONDOWN)
				but = MouseEvent::MBMiddle;
			c->mousePressed(MouseEvent(x,y,but));
		}
		if (msg==WM_MOUSEMOVE)
			c->mouseMoved(x, y);
		if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_MBUTTONUP)
		{
			ReleaseCapture();
			c->mouseReleased(x, y);
		}
		return true;
	}
	return false;
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
	if (map_mouse_message(c, hwnd, Message, wParam, lParam)==true)
	{
		return 0;
	}

	if (Message == WM_MOUSEWHEEL)
	{
		c->mouseWheel(0,0, (short)HIWORD(wParam));
		return 0;
	}
	if (Message == WM_DESTROY)
	{
		//ShowConsoleMsg("lice control window destroy\n");
		g_controlsmap.erase(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, Message, wParam, lParam);
}

void TestControl::paint(LICE_IBitmap * bm)
{
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	for (int i=0;i<m_points.size();++i)
	{
		LICE_pixel color=LICE_RGBA(255,255,255,255);
		if (m_hot_point==i)
			color=LICE_RGBA(255, 0, 0, 255);
		auto& e = m_points[i];
		LICE_FillCircle(bm, e.m_x, e.m_y, m_circlesize, color);
	}
}

int TestControl::find_hot_point(int x, int y)
{
	for (int i=0;i<m_points.size();++i)
	{
		const point& pt=m_points[i];
		if (is_point_in_rect(x,y,pt.m_x-m_circlesize,pt.m_y-m_circlesize,2*m_circlesize,2*m_circlesize)==true)
		{
			return i;
		}
	}
	return -1;
}

void TestControl::mousePressed(const MouseEvent& ev)
{
	if (ev.m_mb == MouseEvent::MBMiddle)
	{
		readbg() << "you pressed the middle button!\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBRight)
	{
		PopupMenu menu(getWindowHandle());
		menu.add_menu_item("First action", []() { readbg() << "first action chosen\n"; });
		if (m_hot_point>=0)
			menu.add_menu_item("Remove point", [this]() 
			{ 
				m_points.erase(m_points.begin() + m_hot_point);
				m_hot_point = -1;
				repaint();
			});
		for (int i = 0; i < 10; ++i)
		{
			menu.add_menu_item(std::to_string(i + 1), [i]()
			{
				readbg() << "You chose number " << i + 1 << "\n";
			});
		}
		menu.execute(ev.m_x, ev.m_y);
		return;
	}
	m_mousedown=true;
	if (m_hot_point==-1)
	{
		m_points.push_back({ ev.m_x,ev.m_y });
		m_hot_point=(int)m_points.size()-1;
		repaint();
	}
}

void TestControl::mouseMoved(int x, int y)
{
	if (m_mousedown==false)
	{
		int found=find_hot_point(x, y);
		if (found!=m_hot_point)
		{
			m_hot_point=found;
			repaint();
		}
	} else
	{
		if (m_hot_point>=0)
		{
			if (m_delete_point_when_dragged_outside == true)
			{
				if (is_point_in_rect(m_points[m_hot_point].m_x, m_points[m_hot_point].m_y, 
					-30, -30, getWidth()+60, getHeight()+60) == false)
				{
					m_points.erase(m_points.begin() + m_hot_point);
					m_hot_point = -1;
					repaint();
					return;
				}
			}
			m_points[m_hot_point].m_x = x; // bound_value(0, x, getWidth());
			m_points[m_hot_point].m_y = y; // bound_value(0, y, getHeight());
			if (PointMovedCallback)
			{
				double normx = bound_value(0.0, 1.0 / getWidth()*x, 1.0);
				double normy = bound_value(0.0, 1.0 / getHeight()*y, 1.0);
				PointMovedCallback(m_hot_point, normx, normy);
			}
			repaint();
		}
	}
}

void TestControl::mouseReleased(int x, int y)
{
	m_mousedown=false;
}

void TestControl::mouseWheel(int x, int y, int delta)
{
	float temp = 1.0f;
	if (delta<0)
		temp=-1.0f;
	m_circlesize = bound_value(1.0f, m_circlesize+temp, 100.0f);
	repaint();
}

void add_menu_item_(HMENU menu, std::string text, int id)
{
	MENUITEMINFO info = { 0 };
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STRING | MIIM_ID;
	info.wID = id;
	info.dwTypeData = (LPSTR)text.c_str();
	InsertMenuItem(menu, id-1, TRUE, &info);
}

PopupMenu::PopupMenu(HWND parent) : m_hwnd(parent)
{
	m_menu = CreatePopupMenu();
}

PopupMenu::~PopupMenu()
{
	if (m_menu != NULL)
		DestroyMenu(m_menu);
}

void PopupMenu::add_menu_item(std::string txt, std::function<void(void)> action)
{
	menu_entry_t entry;
	entry.m_text = txt;
	entry.m_f = action;
	m_entries.push_back(entry);
}

void PopupMenu::execute(int x, int y)
{
	if (m_entries.size() == 0)
		return;
	for (int i = 0; i < m_entries.size(); ++i)
		add_menu_item_(m_menu, m_entries[i].m_text, i + 1);
	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen(m_hwnd, &pt);
	BOOL result = TrackPopupMenu(m_menu, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL);
	if (result > 0)
	{
		m_entries[result - 1].m_f();
	}
}
