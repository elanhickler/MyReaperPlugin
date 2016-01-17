#include "mrpwincontrols.h"
#include "mrpwindows.h"
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#else
#include "WDL/WDL/swell/swell-dlggen.h"
#endif

extern HINSTANCE g_hInst;

HFONT g_defaultwincontrolfont = NULL;

// Incremented for each control added to get a dialog control identifier number.
// Obviously not thread safe but GUI stuff doesn't work with multiple threads anyway.
int g_control_counter = 0;

// To observe WinControl creation/destruction counts
int g_leak_counter = 0;

int get_wincontrol_leak_count()
{
	return g_leak_counter;
}

// std::string data can't be used to for LB_GETTEXT etc calls, so have this instead
// with some space already allocated
std::vector<char> g_messagetextsbuffer(1024);

void adjust_message_text_buffer(int size, bool fillzeros=true)
{
	if (g_messagetextsbuffer.size() < size)
		// 10% more than requested so we don't have to reallocate if the next request is just tiny bit larger
		g_messagetextsbuffer.resize(size*1.1); 
	if (fillzeros == true)
		std::fill(g_messagetextsbuffer.begin(), g_messagetextsbuffer.end(), 0);
}

WinControl::WinControl(MRPWindow* parent)
{
	m_parent = parent;
	++g_control_counter;
	m_control_id = g_control_counter;
	++g_leak_counter;
	if (g_defaultwincontrolfont == NULL)
	{
		g_defaultwincontrolfont = CreateFont(14, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
	}

}

WinControl::~WinControl()
{
	//readbg() << "WinControl dtor of " << this << "\n";
	if (m_hwnd != NULL)
	{
		// Since these should always be parented to some other window, 
		// probably don't need to destroy the handle outselves...?
		//DestroyWindow(m_hwnd);
	}
	--g_leak_counter;
}

bool WinControl::isVisible()
{
	if (m_hwnd!=NULL)
		return IsWindowVisible(m_hwnd);
	return false;
}

void WinControl::setVisible(bool b)
{
	if (m_hwnd != NULL)
	{
		if (b == true)
			ShowWindow(m_hwnd, SW_SHOW);
		else ShowWindow(m_hwnd, SW_HIDE);
	}
}

bool WinControl::isEnabled()
{
	if (m_hwnd == NULL)
		return false;
	
#ifdef WIN32
	return IsWindowEnabled(m_hwnd);
#else
	return true;
#endif
}

void WinControl::setEnabled(bool b)
{

	if (m_hwnd == NULL)
		return;
	if (b == true)
		EnableWindow(m_hwnd, TRUE);
	else EnableWindow(m_hwnd, FALSE);
}

int WinControl::getXPosition() const
{
	RECT r;
	GetWindowRect(m_hwnd, &r);
	POINT pt;
	pt.x=r.left;
	pt.y=r.top;
	ScreenToClient(m_hwnd, &pt);
	return pt.x;
}

int WinControl::getYPosition() const
{
	RECT r;
	GetWindowRect(m_hwnd, &r);
	POINT pt;
	pt.x=r.left;
	pt.y=r.top;
	ScreenToClient(m_hwnd, &pt);
	return pt.y;
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

MRP::Rectangle WinControl::getBounds() const
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	return MRP::Rectangle(r.left,r.top,r.right-r.left,r.bottom-r.top);
}

void WinControl::setBounds(MRP::Rectangle g)
{
	if (g.isValid() == false)
		return;
	if (getBounds() == g)
		return;
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, g.getX(), g.getY(), g.getWidth(), g.getHeight(), SWP_NOACTIVATE | SWP_NOZORDER);
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

void WinControl::setObjectName(std::string name)
{
	m_object_name = name;
}

WinButton::WinButton(MRPWindow* parent, std::string text) :
	WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("BUTTON", "button", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeButton(0, text.c_str(), g_control_counter, 0, 0, 20, 20, WS_CHILD | WS_TABSTOP);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_defaultwincontrolfont, TRUE);
	SetWindowText(m_hwnd, text.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
	GenericNotifyCallback = [this](GenericNotifications)
	{
		//readbg() << "button " << getText() << " clicked. No custom click callback set yet!\n";
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

void WinButton::setStringProperty(int which, std::string text)
{
	if (which == 0)
		setText(text);
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

WinLabel::WinLabel(MRPWindow* parent, std::string text, bool alignright) : WinControl(parent)
{
	DWORD styleflags = WS_CHILD | WS_TABSTOP;
	if (alignright)
		styleflags = WS_CHILD | WS_TABSTOP | SS_RIGHT;
#ifdef WIN32
	m_hwnd = CreateWindow("STATIC", "label", styleflags, 0, 0, 10, 10, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeLabel(-1, text.c_str(), g_control_counter, 0, 0, 20, 20, 0);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_defaultwincontrolfont, TRUE);
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

ReaSlider::ReaSlider(MRPWindow* parent, double initpos) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("REAPERhfader", "slider", WS_CHILD | WS_TABSTOP, 5, 5, 30, 10, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeControl("REAPERhfader", g_control_counter, "REAPERhfader",
		WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, 0);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	m_val_converter = std::make_shared<LinearValueConverter>(0.0,1.0);
	int slidpos = 1000*m_val_converter->toNormalizedFromValue(initpos);
	SendMessage(m_hwnd, TBM_SETPOS, 0, (LPARAM)slidpos);
	SendMessage(m_hwnd, TBM_SETTIC, 0, (LPARAM)slidpos);
	ShowWindow(m_hwnd, SW_SHOW);
}

bool ReaSlider::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		if ((HWND)lparam == m_hwnd)
		{
			double pos = SendMessage((HWND)lparam, TBM_GETPOS, 0, 0);
			if (LOWORD(wparam) == SB_THUMBTRACK && SliderValueCallback)
			{
				SliderValueCallback(GenericNotifications::DuringManipulation, m_val_converter->fromNormalizedToValue(pos/1000.0));
				return true;
			}
			if (LOWORD(wparam) == SB_ENDSCROLL && SliderValueCallback)
			{
				SliderValueCallback(GenericNotifications::AfterManipulation, m_val_converter->fromNormalizedToValue(pos / 1000.0));
				return true;
			}
		}
	}
	return false;
}

double ReaSlider::getValue()
{
	if (m_hwnd != NULL)
	{
		int slidpos = SendMessage(m_hwnd, TBM_GETPOS, 0, 0);
		return m_val_converter->fromNormalizedToValue((double)slidpos / 1000.0);
	}
	return 0.0;
}

void ReaSlider::setValue(double val)
{
	if (m_hwnd != NULL)
	{
		int slidpos = m_val_converter->toNormalizedFromValue(val)*1000.0;
		SendMessage(m_hwnd, TBM_SETPOS, 0, (LPARAM)slidpos);
	}
}

void ReaSlider::setTickMarkPositionFromValue(double val)
{
	int slidpos = m_val_converter->toNormalizedFromValue(val)*1000.0;
	SendMessage(m_hwnd, TBM_SETTIC, 0, slidpos);
}

void ReaSlider::setValueConverter(std::shared_ptr<IValueConverter> c)
{
	int curpos = SendMessage(m_hwnd, TBM_GETPOS, 0, 0);
	double oldnormalized = (double)curpos / 1000.0;
	m_val_converter = c;
	setValue(m_val_converter->fromNormalizedToValue(oldnormalized));
}

double ReaSlider::getFloatingPointProperty(int which)
{
	if (which == 0)
		return getValue();
	return 0.0;
}

void ReaSlider::setFloatingPointProperty(int which, double v)
{
	if (which == 0)
	{
		setValue(v);
	}
}

WinLineEdit::WinLineEdit(MRPWindow* parent, std::string text) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("EDIT", "edit", WS_CHILD | WS_TABSTOP, 5, 5, 30, 20, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeEditField(g_control_counter, 0, 0, 50, 20, WS_CHILD | WS_TABSTOP);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_defaultwincontrolfont, TRUE);
	setText(text);
	ShowWindow(m_hwnd, SW_SHOW);
}

std::string WinLineEdit::getText()
{
	if (m_hwnd == NULL)
		return std::string();
	char buf[1024];
	if (GetWindowText(m_hwnd, buf, 1024) == 0)
	{
		//readbg() << "GetWindowText error : " << (int)GetLastError() << "\n";
	}
	return std::string(buf);
}

void WinLineEdit::setText(std::string txt)
{
	if (m_hwnd == NULL)
		return;
	SetWindowText(m_hwnd, txt.c_str());
}

bool WinLineEdit::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_COMMAND && LOWORD(wparam) == m_control_id
		&& HIWORD(wparam) == EN_CHANGE && TextCallback)
	{
		TextCallback(getText());
		return true;
	}
	return false;
}

WinComboBox::WinComboBox(MRPWindow* parent) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("COMBOBOX", "combo", CBS_DROPDOWNLIST	| WS_CHILD | WS_TABSTOP, 5, 5, 30, 20, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
	WDL_UTF8_HookComboBox(m_hwnd);
#else
	m_hwnd = SWELL_MakeCombo(g_control_counter,0,0,20,20, WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	if (m_hwnd == NULL)
		readbg() << "ComboBox could not be created\n";
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_defaultwincontrolfont, TRUE);
	ShowWindow(m_hwnd, SW_SHOW);
}

void WinComboBox::addItem(std::string text, int user_id)
{
	if (m_hwnd == NULL)
		return;
	auto index = SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text.c_str());
	if (index != CB_ERR)
	{
		SendMessage(m_hwnd, CB_SETITEMDATA, index, (LPARAM)user_id);
	}
}

int WinComboBox::numItems()
{
	return SendMessage(m_hwnd, CB_GETCOUNT, 0, 0);
}

int WinComboBox::getSelectedIndex()
{
	return SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);;
}

int WinComboBox::getSelectedUserID()
{
	auto index = getSelectedIndex();
	return userIDfromIndex(index);
}

void WinComboBox::setSelectedIndex(int index)
{
	SendMessage(m_hwnd, CB_SETCURSEL, index, 0);
}

void WinComboBox::setSelectedUserID(int id)
{
	int count = numItems();
	for (int i = 0; i < count; ++i)
	{
		auto result = SendMessage(m_hwnd, CB_GETITEMDATA, i, 0);
		if (result == id)
		{
			setSelectedIndex(i);
			return;
		}
	}
}

int WinComboBox::userIDfromIndex(int index)
{
	auto result = SendMessage(m_hwnd, CB_GETITEMDATA, index, 0);
	if (result!=CB_ERR)
		return result;
	return -1;
}

bool WinComboBox::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_COMMAND && (HWND)lparam == m_hwnd && HIWORD(wparam) == CBN_SELCHANGE &&
		SelectedChangedCallback)
	{
		auto index = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
		SelectedChangedCallback(index);
		return true;
	}
	return false;
}

WinListBox::WinListBox(MRPWindow* parent) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("LISTBOX", "list", LBS_NOTIFY | WS_VSCROLL | WS_CHILD | WS_TABSTOP, 5, 5, 30, 20, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
	WDL_UTF8_HookListBox(m_hwnd);
#else
	m_hwnd = SWELL_MakeListBox(g_control_counter, 0, 0, 20, 20, LVS_SINGLESEL | WS_CHILD | WS_TABSTOP);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	if (m_hwnd == NULL)
		readbg() << "ListBox could not be created\n";
	
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_defaultwincontrolfont, TRUE);
	ShowWindow(m_hwnd, SW_SHOW);
}

bool WinListBox::handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_COMMAND && (HWND)lparam == m_hwnd && HIWORD(wparam) == LBN_SELCHANGE &&
		SelectedChangedCallback)
	{
		auto index = SendMessage((HWND)lparam, LB_GETCURSEL, 0, 0);
		SelectedChangedCallback(index);
		return true;
	}
	return false;
}

void WinListBox::addItem(std::string text, int user_id)
{
	int pos = (int)SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)text.c_str());
	SendMessage(m_hwnd, LB_SETITEMDATA, pos, (LPARAM)user_id);
}

int WinListBox::userIDfromIndex(int index)
{
	auto result = SendMessage(m_hwnd, LB_GETITEMDATA, index, 0);
	if (result != LB_ERR)
		return result;
	return -1;
}

int WinListBox::numItems()
{
	return SendMessage(m_hwnd, LB_GETCOUNT, 0, 0);
}

std::string WinListBox::getItemText(int index)
{
	if (index<0)
		return std::string();
	int textLen = SendMessage(m_hwnd, LB_GETTEXTLEN, index, 0);
	if (textLen > 0)
	{
		adjust_message_text_buffer(textLen + 1);
		SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)g_messagetextsbuffer.data());
		return std::string(g_messagetextsbuffer.data());
	}
	return std::string();
}

int WinListBox::getSelectedIndex()
{
	return SendMessage(m_hwnd, LB_GETCURSEL, 0, 0);
}

void WinListBox::setSelectedIndex(int index)
{
	SendMessage(m_hwnd, LB_SETCURSEL, index, 0);
}



void WinListBox::clearItems()
{
	SendMessage(m_hwnd, LB_RESETCONTENT, 0, 0);
}

void WinListBox::removeItem(int index)
{
	if (index >= 0 && index < numItems())
	{
		SendMessage(m_hwnd, LB_DELETESTRING, index, 0);
	}
	
}
