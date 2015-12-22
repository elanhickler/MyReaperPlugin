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
#include "mylicecontrols.h"
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

std::unique_ptr<TestControl> g_xycontrol;
std::unique_ptr<PitchBenderEnvelopeControl> g_pitch_bender_control;
std::unique_ptr<EnvelopeGeneratorEnvelopeControl> g_env_lfo;

INT_PTR CALLBACK xycontroldlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Multi XY control");
		g_xycontrol = std::make_unique<TestControl>(hwndDlg);
		g_xycontrol->PointMovedCallback = [ptr=g_xycontrol.get()](int ptindex, double x, double y)
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
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;
		g_xycontrol->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK pitchbenderdialogproc(
	HWND   hwndDlg,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Lice Test");
		//g_env_lfo = std::make_unique<EnvelopeGeneratorEnvelopeControl>(hwndDlg);
		g_pitch_bender_control = std::make_unique<PitchBenderEnvelopeControl>(hwndDlg);
		auto pchenv = std::make_shared<breakpoint_envelope>("Bend amount (semitones)",
			LICE_RGBA(255,255,255,255));
		pchenv->add_point({ 0.0, 0.0 } , false);
		pchenv->add_point({ 0.1, 0.9 }, false);
		pchenv->add_point({ 0.8, 0.9 }, false);
		pchenv->add_point({ 0.9, 1.0 }, false);
		pchenv->add_point({ 1.0, 0.0 }, false);
		pchenv->sort_points();
		g_pitch_bender_control->add_envelope(pchenv);
		auto volenv = std::make_shared<breakpoint_envelope>("Volume",
			LICE_RGBA(255, 0, 255, 255));
		volenv->add_point({ 0.0, 0.0 }, false);
		volenv->add_point({ 0.05, 0.95 }, false);
		volenv->add_point({ 0.95, 0.95 }, false);
		volenv->add_point({ 1.00, 0.00 }, false);
		volenv->sort_points();
		g_pitch_bender_control->add_envelope(volenv);
		g_pitch_bender_control->fitEnvelopeTimeRangesIntoView();
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
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right-r.left;
		int h = r.bottom-r.top;
		g_pitch_bender_control->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK envgendialogproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Envelope point generator");
		g_env_lfo = std::make_unique<EnvelopeGeneratorEnvelopeControl>(hwndDlg);
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
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;
		g_env_lfo->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}

class WaveformsContainer
{
public:
	WaveformsContainer(HWND parent)
	{
		m_parent_wnd = parent;
		int numcontrols = 4;
		for (int i = 0; i < numcontrols; ++i)
		{
			auto wc = std::make_shared<WaveformControl>(parent);
			m_controls.push_back(wc);
		}
		m_import_button = CreateWindow("BUTTON", "importbut", WS_CHILD | WS_TABSTOP, 
			5, 5, 5, 5, m_parent_wnd,
			(HMENU)1, g_hInst, 0);
		SetWindowText(m_import_button, "Import");
		ShowWindow(m_import_button, SW_SHOW);
		m_render_button = CreateWindow("BUTTON", "renderbut", WS_CHILD | WS_TABSTOP,
			5, 5, 5, 5, m_parent_wnd,
			(HMENU)2, g_hInst, 0);
		SetWindowText(m_render_button, "Render");
		ShowWindow(m_render_button, SW_SHOW);
	}
	void setBounds(int x, int y, int w, int h)
	{
		int wavesavailh = h - 30;
		int wavecontrolh = (double)wavesavailh/m_controls.size();
		for (int i = 0; i < m_controls.size(); ++i)
		{
			int ycor = i*wavecontrolh;
			m_controls[i]->setBounds(0, ycor, w, wavecontrolh-5);
		}
		SetWindowPos(m_import_button, NULL, 5, h - 25, 95, 20, SWP_NOACTIVATE | SWP_NOZORDER);
		SetWindowPos(m_render_button, NULL, 105, h - 25, 95, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	void on_command_msg(WPARAM wparam, LPARAM lparam)
	{
		if (GetDlgItem(m_parent_wnd, LOWORD(wparam)) == m_render_button)
		{
			readbg() << "render button clicked\n";
		}
		if (GetDlgItem(m_parent_wnd, LOWORD(wparam)) == m_import_button)
		{
			readbg() << "import button clicked\n";
			int num_to_import = std::min((int)m_controls.size(), CountSelectedMediaItems(nullptr));
			int cnt = 0;
			for (int i = 0; i < num_to_import; ++i)
			{
				MediaItem* item = GetSelectedMediaItem(nullptr, i);
				MediaItem_Take* take = GetActiveTake(item);
				if (take != nullptr)
				{
					PCM_source* src = GetMediaItemTake_Source(take);
					std::string err = is_source_audio(src);
					if (err.empty() == true)
					{
						m_controls[cnt]->setSource(src);
						++cnt;
					}
				}
			}
		}
	}
private:
	std::vector<std::shared_ptr<WaveformControl>> m_controls;
	HWND m_parent_wnd = NULL;
	HWND m_import_button = NULL;
	HWND m_render_button = NULL;
};

std::unique_ptr<WaveformsContainer> g_waveformscont;

INT_PTR CALLBACK wavecontrolsdialogproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Waveforms");
		g_waveformscont = std::make_unique<WaveformsContainer>(hwndDlg);
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
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;
		g_waveformscont->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	if (uMsg == WM_COMMAND)
	{
		g_waveformscont->on_command_msg(wParam, lParam);
		return TRUE;
	}
	return FALSE;
}

HWND g_pitchbenderwindow = NULL;

HWND open_pitch_bender(HWND parent)
{
	if (g_pitchbenderwindow ==NULL)
	{
		g_pitchbenderwindow = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), 
			parent, pitchbenderdialogproc, NULL);
		SetWindowPos(g_pitchbenderwindow, NULL, 20, 60, 600, 350, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	ShowWindow(g_pitchbenderwindow, SW_SHOW);
	return g_pitchbenderwindow;
}

HWND g_envpt_gen_window = NULL;

HWND open_env_point_generator(HWND parent)
{
	if (g_envpt_gen_window == NULL)
	{
		g_envpt_gen_window = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), 
			parent, envgendialogproc, NULL);
		SetWindowPos(g_envpt_gen_window, NULL, 20, 60, 600, 350, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	ShowWindow(g_envpt_gen_window, SW_SHOW);
	return g_envpt_gen_window;
}

HWND g_xy_control_window = NULL;

HWND open_xy_control(HWND parent)
{
	if (g_xy_control_window == NULL)
	{
		g_xy_control_window = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
			parent, xycontroldlgproc, NULL);
		SetWindowPos(g_xy_control_window, NULL, 20, 60, 600, 350, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	ShowWindow(g_xy_control_window, SW_SHOW);
	return g_xy_control_window;
}

HWND g_wave_controls_window = NULL;

HWND open_wave_controls(HWND parent)
{
	if (g_wave_controls_window == NULL)
	{
		g_wave_controls_window = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
			parent, wavecontrolsdialogproc, NULL);
		SetWindowPos(g_wave_controls_window, NULL, 20, 60, 700, 400, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	ShowWindow(g_wave_controls_window, SW_SHOW);
	return g_wave_controls_window;
}

void clean_up_gui()
{
	g_test_timer.stop();
}

