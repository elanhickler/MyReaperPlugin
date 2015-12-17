#include "lice_control.h"
#include <unordered_map>
#include "utilfuncs.h"
#include <string>

std::unordered_map<HWND, LiceControl*> g_controlsmap;

extern HINSTANCE g_hInst;
bool g_kbdhookinstalled = false;
extern reaper_plugin_info_t* g_plugin_info;

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

HWND g_myfocuswindow;

int acProc(MSG *msg, accelerator_register_t *ctx)
{
	HWND myChildWindow = g_myfocuswindow; //*((HWND*)ctx->user);
	int x;
	if (myChildWindow && msg->hwnd && (msg->hwnd == myChildWindow || IsChild(myChildWindow, msg->hwnd)))
	{
#ifdef __APPLE__
		SendMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		return 1;
#else
		return -1;
#endif
	}
	return 0;
}

static accelerator_register_t g_acRec =
{
	acProc,
	true, nullptr
};

bool g_acrecinstalled=false;

LiceControl::LiceControl(HWND parent)
{
	m_hwnd = SWELL_CreatePlainWindow(g_hInst, parent, wndproc, NULL);
	if (m_hwnd == NULL)
	{
		readbg() << "Failed to create window for LiceControl " << this << "\n";
		return;
	}
	m_parenthwnd = parent;

	m_acreg.isLocal = true;
	m_acreg.translateAccel = acProc;
	m_acreg.user = (void*)&m_hwnd;
	
	if (g_plugin_info != nullptr) // && g_acrecinstalled == false)
	{
		//g_acRec.user = (void*)&m_hwnd;
		
		g_acrecinstalled = true;
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
		//g_plugin_info->Register("-accelerator", (void*)&m_acreg);
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

void LiceControl::setFocused()
{
	if (m_wants_focus == true)
	{
		
		SetFocus(m_hwnd);
		// Might be nice to have some customization point for what happens when the focus is got...
		repaint();
	}
}

void update_modifiers_state(ModifierKeys& keys, WPARAM wParam)
{
#ifdef WIN32
	/*
	if (wParam & MK_SHIFT)
		keys.setModifierDown(MKShift, true);
	if (wParam & MK_CONTROL)
		keys.setModifierDown(MKControl, true);
	*/
	if (HIBYTE(GetKeyState(VK_SHIFT)) & 0x80)
		keys.setModifierDown(MKShift, true);
	if (HIBYTE(GetKeyState(VK_CONTROL)) & 0x80)
		keys.setModifierDown(MKControl, true);
	if (HIBYTE(GetAsyncKeyState(VK_MENU)) & 0x80)
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
			c->setFocused();
			g_myfocuswindow = hwnd;
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
// Taken from WDL curses_win32.cpp
#define ERR -1
static LRESULT xlateKey(int msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYDOWN)
	{
#ifndef _WIN32
		if (lParam & FVIRTKEY)
#endif
			switch (wParam)
			{
			case VK_HOME: return KEY_HOME;
			case VK_UP: return KEY_UP;
			case VK_PRIOR: return KEY_PPAGE;
			case VK_LEFT: return KEY_LEFT;
			case VK_RIGHT: return KEY_RIGHT;
			case VK_END: return KEY_END;
			case VK_DOWN: return KEY_DOWN;
			case VK_NEXT: return KEY_NPAGE;
			case VK_INSERT: return KEY_IC;
			case VK_DELETE: return KEY_DC;
			case VK_F1: return KEY_F1;
			case VK_F2: return KEY_F2;
			case VK_F3: return KEY_F3;
			case VK_F4: return KEY_F4;
			case VK_F5: return KEY_F5;
			case VK_F6: return KEY_F6;
			case VK_F7: return KEY_F7;
			case VK_F8: return KEY_F8;
			case VK_F9: return KEY_F9;
			case VK_F10: return KEY_F10;
			case VK_F11: return KEY_F11;
			case VK_F12: return KEY_F12;
#ifndef _WIN32
			case VK_SUBTRACT: return '-'; // numpad -
			case VK_ADD: return '+';
			case VK_MULTIPLY: return '*';
			case VK_DIVIDE: return '/';
			case VK_DECIMAL: return '.';
			case VK_NUMPAD0: return '0';
			case VK_NUMPAD1: return '1';
			case VK_NUMPAD2: return '2';
			case VK_NUMPAD3: return '3';
			case VK_NUMPAD4: return '4';
			case VK_NUMPAD5: return '5';
			case VK_NUMPAD6: return '6';
			case VK_NUMPAD7: return '7';
			case VK_NUMPAD8: return '8';
			case VK_NUMPAD9: return '9';
			case (32768 | VK_RETURN) : return VK_RETURN;
#endif
			}

		switch (wParam)
		{
		case VK_RETURN: case VK_BACK: case VK_TAB: case VK_ESCAPE: return wParam;
		case VK_CONTROL: break;

		default:
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				if (wParam >= 'a' && wParam <= 'z')
				{
					wParam += 1 - 'a';
					return wParam;
				}
				if (wParam >= 'A' && wParam <= 'Z')
				{
					wParam += 1 - 'A';
					return wParam;
				}
				if ((wParam&~0x80) == '[') return 27;
				if ((wParam&~0x80) == ']') return 29;
			}
		}
	}

#ifdef _WIN32 // todo : fix for nonwin32
	if (msg == WM_CHAR)
	{
		if (wParam >= 32) return wParam;
	}
#else
	//osx/linux
	if (wParam >= 32)
	{
		if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
		{
			if (wParam >= 'A' && wParam <= 'Z')
			{
				if ((GetAsyncKeyState(VK_LWIN) & 0x8000)) wParam -= 'A' - 1;
				else
					wParam += 'a' - 'A';
			}
		}
		return wParam;
	}

#endif
	return ERR;
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

	if (Message == WM_GETDLGCODE)
	{
		return DLGC_WANTALLKEYS;
	}
	if (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN)
	{
		const char* validkeys = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-,.'";
		ModifierKeys modkeys;
		update_modifiers_state(modkeys, wParam);
		int k = 0;
		if (wParam == VK_LEFT) k = KEY_LEFT;
		if (wParam == VK_RIGHT) k = KEY_RIGHT;
		if (wParam == VK_UP) k = KEY_UP;
		if (wParam == VK_DOWN) k = KEY_DOWN;
		if (wParam == VK_NEXT) k = KEY_NPAGE;
		if (wParam == VK_PRIOR) k = KEY_PPAGE;
		if (wParam == VK_BACK) k = KEY_BACKSPACE;
		if (wParam == '+') k = wParam;
		if (wParam == '-') k = wParam;
		if (wParam == ',') k = wParam;
		if (wParam == '.') k = wParam;
		if (wParam == '\'') k = wParam;
		if (wParam >= VK_F1 && wParam <= VK_F12)
			k = wParam - VK_F1 + KEY_F1;
		if (wParam >= 'A' && wParam <= 'Z')
			k = wParam;
		if (wParam >= '0' && wParam <= '9')
			k = wParam;
		if (k != 0)
		{
			c->keyPressed(modkeys,k);
			return 0;
		}
	}
	if (Message == WM_KEYUP)
	{
		
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

// Yet another punishment from win32. The timer callback function isn't provided
// a context pointer, but rather the timer id, so we need to map ourselves
// from that id to the Timer pointers...
std::unordered_map<UINT_PTR, Timer*> g_timer_map;

Timer::~Timer()
{
	stop();
}

void Timer::start(int milliseconds)
{
	if (m_id != 0)
	{
		KillTimer(NULL, m_id);
		g_timer_map.erase(m_id);
	}
	m_id = SetTimer(NULL, 0, milliseconds, thetimerproc);
	g_timer_map[m_id] = this;
}

void Timer::stop()
{
	if (m_id != 0)
	{
		KillTimer(NULL, m_id);
		g_timer_map.erase(m_id);
		m_id = 0;
	}
}

void CALLBACK Timer::thetimerproc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	Timer* timer = get_from_map(g_timer_map, idEvent);
	if (timer != nullptr && timer->m_callback)
	{
		timer->m_callback();
	}
}

