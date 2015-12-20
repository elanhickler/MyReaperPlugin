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

class LiceControl;

class ReaScriptWindow
{
public:
	enum control_type_t
	{
		Unknown,
		Button,
		LineEdit,
		Label,
		Slider,
		Custom
	};
	struct control_t
	{
		std::string m_name;
		HWND m_hwnd = NULL;
		int m_control_id = 0;
		LiceControl* m_licecontrol = nullptr;
		control_type_t m_type = Unknown;
		bool m_dirty = false;
		double m_val = 0.0;
	};
	ReaScriptWindow(std::string title);
	~ReaScriptWindow();
	void setWindowTitle(std::string title);
	bool isControlDirty(std::string name);
	void cleanControl(std::string name);
	
	control_t* controlFromName(std::string name);
	const char* getControlText(std::string controlname);
	double getControlValueDouble(std::string controlname);
	
	void add_button(std::string name, std::string text);
	void add_slider(std::string name, int initialvalue);
	void add_line_edit(std::string name, std::string initialtext);

	void setControlBounds(std::string name, int x, int y, int w, int h);
	int getBoundsValue(int which);

	bool m_was_closed = false;
	bool m_window_dirty = false;
	bool m_was_resized = false;
	std::string m_last_used_control;
private:
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	HWND m_hwnd = NULL;
	std::vector<control_t> m_controls;
	int m_control_id_count = 1;
};

ReaScriptWindow* open_reascript_test_gui(std::string title);

void clean_up_gui();