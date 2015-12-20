#include "MyFirstDialog.h"

#include "../Visual Studio/resource.h"
#ifndef _WIN32 // MAC resources
#include "WDL/WDL/swell/swell-dlggen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_dlg"
#undef BEGIN
#undef END
#include "WDL/WDL/swell/swell-menugen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_menu"
#endif

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include "MyLiceWindow.h"
#include <unordered_map>
#include <memory>
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#endif
extern HINSTANCE g_hInst;

std::unordered_map<HWND, ReaperDialog*> g_dialogmap;

ReaperDialog::ReaperDialog(HWND parent, int dialogresource, std::function<bool(HWND, UINT, WPARAM, LPARAM)> proc)
	: DialogProc(proc)
{
	m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(dialogresource), parent, dlgproc, (LPARAM)this);
	if (m_hwnd == NULL)
		readbg() << "Failed to create native dialog for " << this << "\n";
}

ReaperDialog::~ReaperDialog()
{
	if (m_hwnd != NULL)
	{
		g_dialogmap.erase(m_hwnd);
		DestroyWindow(m_hwnd);
	}
}

bool ReaperDialog::isVisible() const
{
	return IsWindowVisible(m_hwnd) == TRUE;
}

void ReaperDialog::setVisible(bool b)
{
	if (b == true)
		ShowWindow(m_hwnd, SW_SHOW);
	else ShowWindow(m_hwnd, SW_HIDE);
}

void ReaperDialog::add_command_handler(WORD control, WORD id, std::function<void(void)> f)
{
	callback_entry_t entry;
	entry.m_control_id = control;
	entry.m_notification_id = id;
	entry.m_func = f;
	m_simple_command_handlers.push_back(entry);
}

void ReaperDialog::add_text_changed_handler(WORD control, WORD id, std::function<void(std::string)> f)
{
	callback_entry_t entry;
	entry.m_control_id = control;
	entry.m_notification_id = id;
	entry.m_text_changed_func = f;
	m_simple_command_handlers.push_back(entry);
}

INT_PTR CALLBACK ReaperDialog::dlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		ReaperDialog* d = (ReaperDialog*)lParam;
		if (d != nullptr)
		{
			g_dialogmap[hwnd] = d;
			if (d->DialogProc)
				d->DialogProc(hwnd, msg, wParam, lParam);
		}
		return TRUE;
	}
	ReaperDialog* d = nullptr;
	if (g_dialogmap.count(hwnd)>0)
		d = g_dialogmap[hwnd];
	if (d == nullptr)
		return FALSE;
	if (msg == WM_COMMAND)
	{
		bool did_handle = false;
		for (auto& entry : d->m_simple_command_handlers)
		{
			if (entry.m_control_id == LOWORD(wParam) && entry.m_notification_id == HIWORD(wParam))
			{
				did_handle = true;
				if (entry.m_func)
					entry.m_func();
				if (entry.m_text_changed_func && entry.m_notification_id == EN_CHANGE)
				{
					char buf[4096];
					GetWindowText((HWND)lParam, buf, 4096);
					entry.m_text_changed_func(buf);
				}
			}
		}
		if (did_handle == true)
			return TRUE;
		else return FALSE;
	}
	if (msg == WM_DESTROY)
	{
		g_dialogmap.erase(hwnd);
		d->m_hwnd = NULL;
		return TRUE;
	}
	
	if (d->DialogProc)
	{
		if (d->DialogProc(hwnd, msg, wParam, lParam) == true)
			return TRUE;
		else return FALSE;
	}
	
	return FALSE;
}

void set_selected_take_name_to_line_edit(HWND hwndDlg)
{
	if (CountSelectedMediaItems(nullptr) > 0)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr, 0);
		MediaItem_Take* take = GetActiveTake(item);
		if (take != nullptr)
		{
			char buf[1024];
			if (GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false)==true)
				SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), buf);
		}
		
	}
	else
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), "No item selected!");
}

void set_selected_take_name_from_line_edit(HWND hwndDlg)
{
	if (CountSelectedMediaItems(nullptr) > 0)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr, 0);
		MediaItem_Take* take = GetActiveTake(item);
		if (take != nullptr)
		{
			char buf[1024];
			GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), buf, 1024);
			GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, true);
			UpdateArrange();
		}
	}
}


std::unique_ptr<ReaperDialog> g_my_dialog;
Timer g_test_timer;
HWND open_my_first_modeless_dialog(HWND parent)
{
	auto dlg_proc = [](HWND hwnd, UINT msg, WPARAM, LPARAM)
	{
		if (msg == WM_INITDIALOG)
		{
			SetWindowText(hwnd, "My First Dialog");
			ShowWindow(hwnd, SW_SHOW);
			return true;
		}
		if (msg == WM_CLOSE)
		{
			ShowWindow(hwnd, SW_HIDE);
			return true;
		}
		return false;
	};
	if (g_my_dialog == nullptr)
	{
		g_my_dialog = std::make_unique<ReaperDialog>(parent, IDD_DIALOG1, dlg_proc);
		g_my_dialog->add_text_changed_handler(IDC_EDIT2, EN_CHANGE, [](std::string text)
		{
			readbg() << "edit2 text changed to : " << text << "\n";
		});
		g_my_dialog->add_command_handler(IDC_DO1, BN_CLICKED, []()
		{
			set_selected_take_name_to_line_edit(g_my_dialog->getWindowHandle());
		});
		g_my_dialog->add_command_handler(IDC_DO2, BN_CLICKED, []()
		{
			set_selected_take_name_from_line_edit(g_my_dialog->getWindowHandle());
		});
		int counter = 0;
		g_test_timer.set_callback([counter]() mutable
		{ 
			std::string text = "My First Dialog " + std::to_string(counter);
			SetWindowText(g_my_dialog->getWindowHandle(), text.c_str());
			++counter;
		});
		g_test_timer.start(1000);
	}
	if (g_my_dialog->isVisible() == false)
		g_my_dialog->setVisible(true);
	return g_my_dialog->getWindowHandle();
}

std::vector<std::unique_ptr<TestControl>> g_testcontrols;

INT_PTR CALLBACK mylicedialogproc(
	HWND   hwndDlg,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Lice Test");
		
		g_testcontrols.clear();
		int num_controls = 4;
		for (int i=0;i<num_controls;++i)
		{
			bool foo = i == 0;
			auto temp = std::make_unique<TestControl>(hwndDlg,foo);
			if (i == 1)
			{
				TestControl* ptr = temp.get();
				temp->PointMovedCallback = [ptr](int ptindex, double x, double y)
				{
					fx_param_t* x_target = ptr->getFXParamTarget(ptindex, 0);
					fx_param_t* y_target = ptr->getFXParamTarget(ptindex, 1);
					if (x_target != nullptr && y_target != nullptr)
					{
						if (x_target->tracknum >= 1)
							TrackFX_SetParamNormalized(GetTrack(nullptr, x_target->tracknum - 1),
								x_target->fxnum, x_target->paramnum, x);
						if (y_target->tracknum >= 1)
							TrackFX_SetParamNormalized(GetTrack(nullptr, y_target->tracknum - 1),
								y_target->fxnum, y_target->paramnum, y);
					}
				};
			}
			g_testcontrols.push_back(std::move(temp));
		}
		ShowWindow(hwndDlg, SW_SHOW);

		return TRUE;
	}
	if (uMsg == WM_CLOSE)
	{
		ShowWindow(hwndDlg, SW_HIDE);
		return TRUE;
	}
	if (uMsg == WM_SIZE)
	{
		if (g_testcontrols.size()>0)
		{
			RECT r;
			GetClientRect(hwndDlg, &r);
			int w = r.right-r.left;
			int h = r.bottom-r.top;
			g_testcontrols[0]->setBounds(0, 0, w/2-5, h/2-5);
			g_testcontrols[1]->setBounds(w/2+5, 0, w/2-5, h/2-5);
			g_testcontrols[2]->setBounds(0, h/2+5, w/2-5, h/2-5);
			g_testcontrols[3]->setBounds(w/2+5, h/2+5, w/2-5, h/2-5);
			InvalidateRect(hwndDlg, NULL, TRUE);
		}
		return TRUE;
	}
	//if (uMsg == WM_KEYDOWN || uMsg == WM_CHAR || uMsg == WM_KEYUP)
	//	return FALSE;
	return FALSE;
}

HWND g_licetestwindow = NULL;


HWND open_lice_dialog(HWND parent)
{
	if (g_licetestwindow==NULL)
	{
		g_licetestwindow = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), parent, mylicedialogproc, NULL);
		SetWindowPos(g_licetestwindow, NULL, 20, 60, 300, 300, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	ShowWindow(g_licetestwindow, SW_SHOW);
	return g_licetestwindow;
}

void clean_up_gui()
{
	g_test_timer.stop();
}

std::unordered_map<HWND, ReaScriptWindow*> g_reascriptwindowsmap;

ReaScriptWindow::ReaScriptWindow(std::string title)
{
	HWND parent = GetMainHwnd();
	m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_REASCRIPTDIALOG1), parent, dlgproc, (LPARAM)this);
	if (m_hwnd == NULL)
	{
		readbg() << "failed to create reascript dialog\n";
		return;
	}
	control_t c;
	/*
	c.m_name = "Line edit 1";
	c.m_hwnd = CreateWindow("EDIT", c.m_name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd, 0, g_hInst, 0);
	ShowWindow(c.m_hwnd, SW_SHOW);
	m_controls.push_back(c);
	*/
	/*
	for (int i = 0; i < 4; ++i)
	{
		std::string cname = "Slider " + std::to_string(i+1);
		int ycor = 40 + i * 25;
		c.m_hwnd = CreateWindow("REAPERhfader", cname.c_str(), WS_CHILD | WS_TABSTOP, 5, ycor, 290, 20, m_hwnd, 0, g_hInst, 0);
		ShowWindow(c.m_hwnd, SW_SHOW);
		c.m_name = cname;
		m_controls.push_back(c);
	}
	*/
	
	SetWindowText(m_hwnd, title.c_str());
	ShowWindow(m_hwnd, SW_SHOW);
	SetWindowPos(m_hwnd, NULL, 20, 60, 300, 160, SWP_NOACTIVATE | SWP_NOZORDER);
}

ReaScriptWindow::~ReaScriptWindow()
{
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
		g_reascriptwindowsmap.erase(m_hwnd);
	}
}

void ReaScriptWindow::add_slider(std::string name, int initialvalue)
{
	int ycor = 5 + m_controls.size() * 25;
	control_t c;
	c.m_hwnd = CreateWindow("REAPERhfader", name.c_str(), WS_CHILD | WS_TABSTOP, 5, ycor, 290, 20, m_hwnd, 
		(HMENU)m_control_id_count, g_hInst, 0);
	SendMessage(c.m_hwnd, TBM_SETPOS, 0, (LPARAM)initialvalue);
	SendMessage(c.m_hwnd, TBM_SETTIC, 0, 500);
	ShowWindow(c.m_hwnd, SW_SHOW);
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_button(std::string name, std::string text)
{
	control_t c;
	c.m_hwnd = CreateWindow("BUTTON", name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	SetWindowText(c.m_hwnd, text.c_str());
	ShowWindow(c.m_hwnd, SW_SHOW);
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::add_line_edit(std::string name, std::string text)
{
	control_t c;
	c.m_hwnd = CreateWindow("EDIT", name.c_str(), WS_CHILD | WS_TABSTOP, 5, 5, 290, 20, m_hwnd,
		(HMENU)m_control_id_count, g_hInst, 0);
	c.m_name = name;
	c.m_control_id = m_control_id_count;
	SetWindowText(c.m_hwnd, text.c_str());
	ShowWindow(c.m_hwnd, SW_SHOW);
	++m_control_id_count;
	m_controls.push_back(c);
}

void ReaScriptWindow::setWindowTitle(std::string title)
{
	if (m_hwnd != NULL)
	{
		SetWindowText(m_hwnd, title.c_str());
	}
}

const char* ReaScriptWindow::getControlText(std::string cname)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		char buf[1024];
		GetWindowText(c->m_hwnd, buf, 1024);
		return buf;
	}
	return "";
}

double ReaScriptWindow::getControlValueDouble(std::string cname)
{
	control_t* c = controlFromName(cname);
	if (c != nullptr)
	{
		return SendMessage(c->m_hwnd, TBM_GETPOS, 0, 0);
	}
	return 0.0;
}

bool ReaScriptWindow::isControlDirty(std::string name)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
		return c->m_dirty;
	return false;
}

void ReaScriptWindow::cleanControl(std::string name)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
		c->m_dirty = false;
}

ReaScriptWindow::control_t* ReaScriptWindow::controlFromName(std::string name)
{
	for (auto& e : m_controls)
		if (e.m_name == name)
			return &e;
	return nullptr;
}

void ReaScriptWindow::setControlBounds(std::string name, int x, int y, int w, int h)
{
	control_t* c = controlFromName(name);
	if (c != nullptr)
	{
		if (c->m_licecontrol==nullptr)
			SetWindowPos(c->m_hwnd, NULL, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
		else c->m_licecontrol->setBounds(x, y, w, h);
	}
}

int ReaScriptWindow::getBoundsValue(int which)
{
	RECT r;
	GetClientRect(m_hwnd, &r);
	if (which == 0)
		return r.left;
	if (which == 1)
		return r.top;
	if (which == 2)
		return r.right - r.left;
	if (which == 3)
		return r.bottom - r.top;
	return 0;
}

INT_PTR CALLBACK ReaScriptWindow::dlgproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_INITDIALOG)
	{
		ReaScriptWindow* wptr = (ReaScriptWindow*)lparam;
		g_reascriptwindowsmap[hwnd] = wptr;
		return TRUE;
	}
	ReaScriptWindow* wptr = get_from_map(g_reascriptwindowsmap, hwnd);
	if (wptr == nullptr)
		return FALSE;
	if (msg == WM_COMMAND)
	{
		if (HIWORD(wparam) == BN_CLICKED || HIWORD(wparam) == EN_CHANGE)
		{
			for (auto& e : wptr->m_controls)
				if (e.m_control_id == LOWORD(wparam))
				{
					wptr->m_last_used_control = e.m_name;
					return TRUE;
				}
		}
		wptr->m_window_dirty = true;
		return TRUE;
	}
	if (msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		for (auto& e : wptr->m_controls)
			if (e.m_hwnd == (HWND)lparam)
			{
				wptr->m_last_used_control = e.m_name;
				return TRUE;
			}
		wptr->m_window_dirty = true;
		return TRUE;
	}
	if (msg == WM_SIZE)
	{
		wptr->m_was_resized = true;
		return TRUE;
	}
	if (msg == WM_CLOSE)
	{
		g_reascriptwindowsmap.erase(hwnd);
		wptr->m_hwnd = NULL;
		DestroyWindow(hwnd);
		wptr->m_was_closed = true;
		return TRUE;
	}
	return FALSE;
}

HWND g_reascript_gui = NULL;

ReaScriptWindow* open_reascript_test_gui(std::string title)
{
	ReaScriptWindow* result = new ReaScriptWindow(title);
	return result;
}