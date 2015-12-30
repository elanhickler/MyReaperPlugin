#pragma once



#ifdef _WIN32
#include <windows.h>
#include "WDL/WDL/win32_utf8.h"
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
	bool addControlFromName(std::string cname, std::string objectname);
	void setControlBounds(std::string name, int x, int y, int w, int h);
	bool isControlDirty(std::string name);
	void clearDirtyControls();
	double getControlValueDouble(std::string obname, int which);
	void setControlValueDouble(std::string obname, int which, double v);
	int getControlValueInt(std::string obname, int which);
	void setControlValueString(std::string obname, int which, std::string text);
	void setControlValueInt(std::string obname, int which, int v);
	void sendCommandString(const std::string& obname, const std::string& cmd);
	void resized() override { m_was_resized = true; }
	bool m_was_resized = false;
private:
	std::vector<char> m_leak_test;
	WinControl* control_from_name(std::string name);
	std::unordered_set<std::string> m_dirty_controls;
};

bool is_valid_reascriptwindow(ReaScriptWindow* w);

