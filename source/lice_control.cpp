#include "lice_control.h"
#include <unordered_map>
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
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
	if (m_hwnd == NULL)
	{
		readbg() << "Failed to create window for LiceControl " << this << "\n";
		return;
	}
	g_controlsmap[m_hwnd] = this;
	m_bitmap = std::make_unique<LICE_SysBitmap>(200, 200);
	setBounds(20, 60, 200, 200);
	ShowWindow(m_hwnd, SW_SHOW);
}

LiceControl::~LiceControl()
{
	//readbg() << "Lice Control dtor\n";
	if (m_hwnd != NULL)
	{
		g_controlsmap.erase(m_hwnd);
		DestroyWindow(m_hwnd);
	}
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
	return r.bottom - r.top;
}

bool LiceControl::hasFocus() const
{
	return GetFocus() == m_hwnd;
}

void update_modifiers_state(ModifierKeys& keys, WPARAM wParam)
{
#ifdef WIN32
	if (wParam & MK_SHIFT)
		keys.setModifierDown(MKShift, true);
	if (wParam & MK_CONTROL)
		keys.setModifierDown(MKControl, true);
	if (HIBYTE(GetKeyState(VK_MENU)) & 0x80)
		keys.setModifierDown(MKAlt, true);
	if (HIBYTE(GetKeyState(VK_LWIN)) & 0x80)
		keys.setModifierDown(MKAppleOrWindowsKey, true);
#else
	if (HIBYTE(GetAsyncKeyState(VK_SHIFT)) & 0x80)
		keys.setModifierDown(MKShift, true);
	if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) & 0x80)
		keys.setModifierDown(MKControl, true);
	if (HIBYTE(GetAsyncKeyState(VK_MENU)) & 0x80)
		keys.setModifierDown(MKAlt, true);
	if (HIBYTE(GetAsyncKeyState(VK_LWIN)) & 0x80)
		keys.setModifierDown(MKAppleOrWindowsKey, true);
#endif
}

bool map_mouse_message(LiceControl* c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN
		|| msg == WM_MOUSEMOVE || msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_MBUTTONUP)
	{
		// LOWORD/HIWORD are not technically correct according to MSDN
		// However, SWELL does not seem to provide the alternative macros
		// to get the support for multiple monitors. Will need to investigate...
		int x = (short)LOWORD(lParam);
		int y = (short)HIWORD(lParam);
		if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
		{
			SetCapture(hwnd);
			if (c->wantsFocus()==true)
				SetFocus(hwnd);
			MouseEvent::MouseButton but(MouseEvent::MBLeft);
			if (msg == WM_RBUTTONDOWN)
				but = MouseEvent::MBRight;
			else if (msg == WM_MBUTTONDOWN)
				but = MouseEvent::MBMiddle;
			MouseEvent me(x, y, but);
			update_modifiers_state(me.m_modkeys, wParam);
			c->mousePressed(me);
		}
		if (msg == WM_MOUSEMOVE)
		{
			MouseEvent::MouseButton but(MouseEvent::MBLeft);
			if (wParam & MK_RBUTTON)
				but = MouseEvent::MBRight;
			if (wParam & MK_MBUTTON)
				but = MouseEvent::MBMiddle;
			MouseEvent me(x, y, but);
			update_modifiers_state(me.m_modkeys, wParam);
			c->mouseMoved(me);
		}
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
		c = g_controlsmap[hwnd];
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
	if (map_mouse_message(c, hwnd, Message, wParam, lParam) == true)
	{
		return 0;
	}

	if (Message == WM_MOUSEWHEEL)
	{
		c->mouseWheel(0, 0, (short)HIWORD(wParam));
		return 0;
	}
	if (Message == WM_KEYDOWN)
	{
		readbg() << c << " " << wParam << "\n";
	}
	if (Message == WM_DESTROY)
	{
		//ShowConsoleMsg("lice control window destroy\n");
		g_controlsmap.erase(hwnd);
		c->m_hwnd = NULL;
		return 0;
	}
	return DefWindowProc(hwnd, Message, wParam, lParam);
}

void add_menu_item_(HMENU menu, std::string text, int id)
{
	MENUITEMINFO info = { 0 };
	info.cbSize = sizeof(MENUITEMINFO);
#ifdef WIN32
	info.fMask = MIIM_STRING | MIIM_ID;
#else
	info.fMask = MIIM_DATA | MIIM_ID;
#endif
	info.wID = id;
	info.dwTypeData = (LPSTR)text.c_str();
	InsertMenuItem(menu, id - 1, TRUE, &info);
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

void PopupMenu::execute(int x, int y, bool use_screen_coordinates)
{
	if (m_entries.size() == 0)
		return;
	for (int i = 0; i < m_entries.size(); ++i)
		add_menu_item_(m_menu, m_entries[i].m_text, i + 1);
	POINT pt;
	pt.x = x;
	pt.y = y;
	if (use_screen_coordinates == false)
		ClientToScreen(m_hwnd, &pt);
	BOOL result = TrackPopupMenu(m_menu, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL);
	if (result > 0)
	{
		m_entries[result - 1].m_f();
	}
	else
	{
		if (m_none_chosen_f)
			m_none_chosen_f();
	}
}

void PopupMenu::set_none_chosen_handler(std::function<void(void)> f)
{
	m_none_chosen_f = f;
}
