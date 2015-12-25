#include "mrpwincontrols.h"
#include "mrpwindows.h"
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#else
#include "WDL/WDL/swell/swell-dlggen.h"
#endif

extern HINSTANCE g_hInst;

int g_control_counter = 0;

int g_leak_counter = 0;

int get_wincontrol_leak_count()
{
	return g_leak_counter;
}

WinControl::WinControl(MRPWindow* parent)
{
	m_parent = parent;
	++g_control_counter;
	m_control_id = g_control_counter;
	++g_leak_counter;
}

WinControl::~WinControl()
{
	readbg() << "WinControl dtor of " << this << "\n";
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
#ifdef WIN32
	if (m_hwnd == NULL)
		return;
	if (b == true)
		EnableWindow(m_hwnd, TRUE);
	else EnableWindow(m_hwnd, FALSE);
#endif
}

// get*Position are suspect. Copy pasted from Stackoverflow answer

int WinControl::getXPosition() const
{
	RECT r;
	RECT rparent;
	GetWindowRect(m_hwnd, &r);
	GetWindowRect(GetParent(m_hwnd), &rparent);
	return r.left-rparent.left;
}

int WinControl::getYPosition() const
{
	RECT r;
	RECT rparent;
	GetWindowRect(m_hwnd, &r);
	GetWindowRect(GetParent(m_hwnd), &rparent);
	return r.top-rparent.top;
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

WinLabel::WinLabel(MRPWindow* parent, std::string text) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("STATIC", "label", WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeLabel(-1, text.c_str(), g_control_counter, 0, 0, 20, 20, 0);
	SetParent(m_hwnd, parent->getWindowHandle());
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
			if (SliderValueCallback)
			{
				double pos = SendMessage((HWND)lparam, TBM_GETPOS, 0, 0);
				SliderValueCallback(m_val_converter->fromNormalizedToValue(pos/1000.0));
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

WinLineEdit::WinLineEdit(MRPWindow* parent, std::string text) : WinControl(parent)
{
#ifdef WIN32
	m_hwnd = CreateWindow("EDIT", "edit", WS_CHILD | WS_TABSTOP, 5, 5, 30, 20, parent->getWindowHandle(),
		(HMENU)g_control_counter, g_hInst, 0);
#else
	m_hwnd = SWELL_MakeEditField(g_control_counter, 0, 0, 50, 20, WS_CHILD | WS_TABSTOP);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
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
		//readbg() << "GetWindowText error : " << GetLastError();
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
#else
	m_hwnd = SWELL_MakeCombo(g_control_counter,0,0,20,20, WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST);
	SetParent(m_hwnd, parent->getWindowHandle());
#endif
	if (m_hwnd == NULL)
		readbg() << "yngh";
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
