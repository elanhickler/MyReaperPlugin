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
#include "mrpwindows.h"

class ReaScriptWindow : public MRPWindow
{
public:
	ReaScriptWindow(std::string title);
	~ReaScriptWindow();
	void addControlFromName(std::string cname, std::string objectname);
	void setControlBounds(std::string name, int x, int y, int w, int h);
private:
	std::vector<char> m_leak_test;
	WinControl* control_from_name(std::string name);
};

bool is_valid_reascriptwindow(ReaScriptWindow* w);

#ifdef REASCRIPTGUIWORKS
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
	void setControlText(std::string cname, std::string text);
	double getControlValueDouble(std::string controlname, int which);
	void setControlValueDouble(std::string controlname, int which, double val);

	void add_button(std::string name, std::string text);
	void add_slider(std::string name, int initialvalue);
	void add_line_edit(std::string name, std::string initialtext);
	void add_label(std::string name, std::string inittext);
	void add_custom_control(std::string name, std::string controlclassname);

	void setControlBounds(std::string name, int x, int y, int w, int h);
	int getBoundsValue(int which);

	bool m_was_closed = false;
	bool m_window_dirty = false;
	bool m_was_resized = false;
	std::unordered_set<std::string> m_last_used_controls;
private:
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	HWND m_hwnd = NULL;
	std::vector<control_t> m_controls;
	int m_control_id_count = 1;
};

ReaScriptWindow* open_reascript_test_gui(std::string title);
#endif
