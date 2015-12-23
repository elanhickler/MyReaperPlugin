#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif
#include "utilfuncs.h"
#include <memory>
#include <functional>
#include <vector>
#include <unordered_set>
#include "mrpwincontrols.h"

class ReaperDialog : public NoCopyNoMove
{
public:
	ReaperDialog(HWND parent, int dialogresource, std::function<bool(HWND, UINT, WPARAM, LPARAM)> proc);
	virtual ~ReaperDialog();
	std::function<bool(HWND, UINT, WPARAM, LPARAM)> DialogProc;
	HWND getWindowHandle() const { return m_hwnd; }
	bool isVisible() const;
	void setVisible(bool b);
	void add_command_handler(WORD control, WORD id, std::function<void(void)> f);
	void add_text_changed_handler(WORD control, WORD id, std::function<void(std::string)> f);
private:
	HWND m_hwnd = NULL;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	struct callback_entry_t
	{
		WORD m_control_id = 0;
		WORD m_notification_id = 0;
		std::function<void(void)> m_func;
		std::function<void(std::string)> m_text_changed_func;
	};
	std::vector<callback_entry_t> m_simple_command_handlers;
};

class MRPWindow
{
public:
	MRPWindow(HWND parent,std::string title="Untitled");
	virtual ~MRPWindow();
	virtual void resized() {};
	virtual void closeRequested() {}
	std::pair<int, int> getSize();
	void setSize(int w, int h);
	HWND getWindowHandle() const { return m_hwnd; }
protected:
	HWND m_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	
};

class TestMRPPWindow : public MRPWindow
{
public:
	TestMRPPWindow(HWND parent,std::string title="Untitled") : MRPWindow(parent,title)
	{
		for (int i = 0; i < 8; ++i)
		{
			auto but = std::make_shared<WinButton>(m_hwnd, std::to_string(i));
			but->GenericNotifyCallback = [i](GenericNotifications)
			{
				readbg() << "you pressed " << i << "\n";
			};
			m_controls.push_back(but);
		}
		m_controls.push_back(std::make_shared<WinLabel>(m_hwnd, "This is a label"));
	}
	~TestMRPPWindow()
	{
		readbg() << "TestMRPWindow dtor\n";
	}
	void resized() override
	{
		if (m_controls.size() == 0)
			return;
		auto sz = getSize();
		int ch = (double)sz.second / m_controls.size();
		for (int i = 0; i < m_controls.size(); ++i)
		{
			m_controls[i]->setBounds(5, 5 + ch*i, sz.first - 10, ch - 3);
		}
	}
	void closeRequested() override;
	
};

HWND open_my_first_modeless_dialog(HWND parent);
HWND open_pitch_bender(HWND parent);
HWND open_env_point_generator(HWND parent);
HWND open_xy_control(HWND parent);
HWND open_wave_controls(HWND parent);
HWND open_gui_designer(HWND parent);
HWND open_win_controls_window(HWND parent);
void clean_up_gui();