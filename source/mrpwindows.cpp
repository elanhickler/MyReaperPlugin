#include "mrpwindows.h"
#include <unordered_map>

// Oh, the horror
#ifndef WIN32
#define IDD_EMPTYDIALOG 666
#include "WDL/WDL/swell/swell-dlggen.h"
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

HWND open_win_controls_window(HWND parent)
{
	static int counter = 1;
	// Test window deletes itself when it is closed, so we can keep this
	// raw pointer just here
	TestMRPPWindow* w = new TestMRPPWindow(parent, std::string("Test window ") + std::to_string(counter));
	w->setDestroyOnClose(true);
	w->setPosition(20 + counter * 20, 60 + counter * 20);
	w->setSize(500, 300);
	++counter;
	return w->getWindowHandle();
}

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
}

MRPWindow::~MRPWindow()
{
	readbg() << "MRPWindow dtor\n";
	m_controls.clear();
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
		g_mrpwindowsmap.erase(m_hwnd);
	}
}

std::pair<int, int> MRPWindow::getSize()
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

void MRPWindow::closeRequested()
{
	readbg() << "close requested...\n";
	if (m_hwnd != NULL && m_destroy_on_close == false)
	{
		readbg() << "only hiding this window...\n";
		ShowWindow(m_hwnd, SW_HIDE);
		
		return;
	}

	if (m_destroy_on_close == true)
	{
		delete this;
		readbg() << "window map has " << g_mrpwindowsmap.size() << " entries\n";
	}
}


INT_PTR MRPWindow::dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_INITDIALOG)
	{
		return TRUE;
	}
	if (msg == WM_COMMAND || msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			for (auto& e : mptr->m_controls)
				if (e->handleMessage(hwnd, msg, wp, lp) == true)
					return TRUE;
		}
	}
	/*
	if (msg == WM_SHOWWINDOW)
	{
	MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
	if (mptr != nullptr)
	mptr->resized();
	return TRUE;
	}
	*/
	if (msg == WM_SIZE)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			mptr->resized();
			InvalidateRect(hwnd, NULL, TRUE);
			return TRUE;
		}
	}
	if (msg == WM_CLOSE)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			mptr->closeRequested();
			return TRUE;
		}
	}
	return FALSE;
}
