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

HWND open_my_first_modeless_dialog(HWND parent);
HWND open_lice_dialog(HWND parent);

class ReaScriptWindow
{
public:
	struct control_t
	{
		std::string m_name;
		HWND m_hwnd = NULL;
		bool m_dirty = false;
	};
	ReaScriptWindow(std::string title);
	~ReaScriptWindow();
	void setWindowTitle(std::string title);
	bool isControlDirty(std::string name);
	void cleanControl(std::string name);
	control_t* controlFromName(std::string name);
	bool m_wants_close = false;
private:
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	HWND m_hwnd = NULL;
	
	std::vector<control_t> m_controls;
};

ReaScriptWindow* open_reascript_test_gui(std::string title);

void clean_up_gui();