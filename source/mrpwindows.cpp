#include "mrpwindows.h"
#include <unordered_map>

// Oh, the horror
#ifndef WIN32
#include "WDL/WDL/swell/swell-dlggen.h"
#define IDD_EMPTYDIALOG 666
#ifndef SWELL_DLG_SCALE_AUTOGEN
#define SWELL_DLG_SCALE_AUTOGEN 1.7
#endif
#ifndef SWELL_DLG_FLAGS_AUTOGEN
#define SWELL_DLG_FLAGS_AUTOGEN SWELL_DLG_WS_FLIPPED|SWELL_DLG_WS_NOAUTOSIZE
#endif

#ifndef SET_IDD_EMPTYDIALOG_SCALE
#define SET_IDD_EMPTYDIALOG_SCALE SWELL_DLG_SCALE_AUTOGEN
#endif
#ifndef SET_IDD_EMPTYDIALOG_STYLE
#define SET_IDD_EMPTYDIALOG_STYLE SWELL_DLG_FLAGS_AUTOGEN|SWELL_DLG_WS_RESIZABLE|SWELL_DLG_WS_OPAQUE
#endif
SWELL_DEFINE_DIALOG_RESOURCE_BEGIN(IDD_EMPTYDIALOG,SET_IDD_EMPTYDIALOG_STYLE,"Dialog",309,179,SET_IDD_EMPTYDIALOG_SCALE)
BEGIN
END
SWELL_DEFINE_DIALOG_RESOURCE_END(IDD_EMPTYDIALOG)

#define IDD_EMPTYDIALOG2 667
#ifndef SWELL_DLG_SCALE_AUTOGEN
#define SWELL_DLG_SCALE_AUTOGEN 1.7
#endif
#ifndef SWELL_DLG_FLAGS_AUTOGEN
#define SWELL_DLG_FLAGS_AUTOGEN SWELL_DLG_WS_FLIPPED|SWELL_DLG_WS_NOAUTOSIZE
#endif

#ifndef SET_IDD_EMPTYDIALOG_SCALE
#define SET_IDD_EMPTYDIALOG_SCALE SWELL_DLG_SCALE_AUTOGEN
#endif
#ifndef SET_IDD_EMPTYDIALOG2_STYLE
#define SET_IDD_EMPTYDIALOG2_STYLE SWELL_DLG_FLAGS_AUTOGEN|SWELL_DLG_WS_OPAQUE
#endif
SWELL_DEFINE_DIALOG_RESOURCE_BEGIN(IDD_EMPTYDIALOG2,SET_IDD_EMPTYDIALOG2_STYLE,"Dialog",309,179,SET_IDD_EMPTYDIALOG_SCALE)
BEGIN
END
SWELL_DEFINE_DIALOG_RESOURCE_END(IDD_EMPTYDIALOG2)


#endif

extern HINSTANCE g_hInst;


#ifdef WIN32
struct MyDLGTEMPLATE : DLGTEMPLATE
{
	WORD ext[3];
	MyDLGTEMPLATE()
	{
		memset(this, 0, sizeof(*this));
	}
};
#endif

std::unordered_map<HWND, MRPWindow*> g_mrpwindowsmap;
extern HWND g_parent;

MRPWindow::MRPWindow(HWND parent, std::string title)
{
#ifdef WIN32
	MyDLGTEMPLATE t;
	t.style = DS_SETFONT | DS_FIXEDSYS | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
	t.cx = 200;
	t.cy = 100;
	t.dwExtendedStyle = WS_EX_TOOLWINDOW;
	m_hwnd = CreateDialogIndirectParam(g_hInst, &t, parent, (DLGPROC)dlgproc, (LPARAM)this);
#else
	m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
		parent, dlgproc, (LPARAM)this);
#endif
	//m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
	//	parent, dlgproc, (LPARAM)this);
	g_mrpwindowsmap[m_hwnd] = this;
	SetWindowText(m_hwnd, title.c_str());
	SetWindowPos(m_hwnd, NULL, 20, 60, 100, 100, SWP_NOACTIVATE | SWP_NOZORDER);
	ShowWindow(m_hwnd, SW_SHOW);
	m_is_closed = false;
}

MRPWindow::~MRPWindow()
{
	//readbg() << "MRPWindow dtor\n";
	m_is_closed = true; 
	m_controls.clear();
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
		g_mrpwindowsmap.erase(m_hwnd);
	}
}

void MRPWindow::add_control(std::shared_ptr<WinControl> c)
{
	auto it = std::find(m_controls.begin(), m_controls.end(), c);
	if (it == m_controls.end())
		m_controls.push_back(c);
}

void MRPWindow::setWindowTitle(std::string title)
{
	SetWindowText(m_hwnd, title.c_str());
}

MRP::Size MRPWindow::getSize()
{
	if (m_hwnd == NULL)
		return{ 0,0 };
	RECT r;
	GetClientRect(m_hwnd, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	return{ w,h };
}

void MRPWindow::setPosition(int x, int y)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}
}

void MRPWindow::setSize(int w, int h)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, 0, 0, w, h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
}

MRP::Rectangle MRPWindow::getBounds() const
{
	if (m_hwnd == NULL)
		return MRP::Rectangle();
	RECT r;
	GetClientRect(m_hwnd, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	return{ r.left, r.top, w,h };
}

MRPModalDialog::MRPModalDialog(HWND parent, std::string title)
{
	m_parent_hwnd = parent;
	m_modal_title = title;
}

MRPWindow::ModalResult MRPModalDialog::runModally()
{
	m_is_modal = true;
#ifdef WIN32
	MyDLGTEMPLATE t;
	t.style = DS_SETFONT | DS_FIXEDSYS | WS_CAPTION | WS_SYSMENU;
	t.cx = 200;
	t.cy = 100;
	DialogBoxIndirectParam(g_hInst, &t, m_parent_hwnd, dlgproc, (LPARAM)this);
#else
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), m_parent_hwnd, dlgproc, (LPARAM)this);
#endif
	return m_modal_result;
}

void MRPWindow::finishModal(ModalResult result)
{
	if (m_is_modal == true && m_hwnd != NULL)
	{
		m_modal_result = result;
		g_mrpwindowsmap.erase(m_hwnd);
		m_hwnd = NULL;
		onModalClose();
	}
}

void MRPWindow::closeRequested()
{
	//readbg() << "close requested...\n";
	m_is_closed = true;
	if (m_hwnd != NULL && m_destroy_on_close == false)
	{
		//readbg() << "only hiding this window...\n";
		ShowWindow(m_hwnd, SW_HIDE);
		
		return;
	}

	if (m_destroy_on_close == true)
	{
		delete this;
		//readbg() << "window map has " << g_mrpwindowsmap.size() << " entries\n";
	}
}

bool MRPWindow::isVisible() const
{
	return IsWindowVisible(m_hwnd) == TRUE;
}

void MRPWindow::setVisible(bool b)
{
	if (b == true)
		ShowWindow(m_hwnd, SW_SHOW);
	else ShowWindow(m_hwnd, SW_HIDE);
}


INT_PTR MRPWindow::dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_INITDIALOG)
	{
		MRPWindow* mrpw = (MRPWindow*)lp;
		if (mrpw != nullptr)
		{
			mrpw->m_hwnd = hwnd;
			g_mrpwindowsmap[hwnd] = mrpw;
			if (mrpw->m_is_modal == true)
			{
				mrpw->init_modal_dialog();
				SetWindowText(hwnd, mrpw->m_modal_title.c_str());
			}
			mrpw->m_helper_timer = SetTimer(hwnd, 25000, 1000, nullptr);
		}
		return TRUE;
	}
	MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
	if (msg == WM_COMMAND || msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		
		if (mptr != nullptr)
		{
			for (auto& e : mptr->m_controls)
				if (e!=nullptr && e->handleMessage(hwnd, msg, wp, lp) == true)
				{
					if (mptr->m_is_modal == true && mptr->m_modal_result != MRPWindow::Undefined)
					{
						EndDialog(hwnd, 0);
					}
					return TRUE;
				}
			
		}
	}
	if (msg == WM_SIZE)
	{
		if (mptr != nullptr)
		{
			mptr->resized();
			InvalidateRect(hwnd, NULL, TRUE);
			return TRUE;
		}
	}
	if (msg == WM_CLOSE)
	{
		
		if (mptr != nullptr)
		{
			if (mptr->m_is_modal == false)
			{
				mptr->closeRequested();
			}
			else
			{
				mptr->m_modal_result = MRPWindow::Rejected;
				EndDialog(hwnd, 2);
			}
			return TRUE;
		}
	}
	if (msg == WM_TIMER && wp==25000)
	{
		if (mptr != nullptr)
			mptr->onTimer();
		return TRUE;
	}
	if (msg == WM_DESTROY)
	{
		if (mptr != nullptr)
			KillTimer(hwnd, mptr->m_helper_timer);
	}
	return FALSE;
}

void MRPWindow::onTimer()
{
	//readbg() << "MRPWindow onTimer\n";
	for (auto& e : m_controls)
		e->onRefreshTimer();
	onRefreshTimer();
}

bool is_valid_mrp_window(MRPWindow* w)
{
	for (auto& e : g_mrpwindowsmap)
		if (e.second==w)
			return true;
	return false;
}

void shutdown_windows()
{
	// Should do some GUI stuff clean up here, but not sure yet how...
}
