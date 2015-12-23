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
#include "mylicecontrols.h"

class MRPWindow
{
public:
	MRPWindow() {}
	MRPWindow(HWND parent, std::string title = "Untitled");
	virtual ~MRPWindow();
	void add_control(std::shared_ptr<WinControl> c)
	{
		m_controls.push_back(c);
	}
	virtual void resized() {};
	virtual void closeRequested();
	std::pair<int, int> getSize();
	void setPosition(int x, int y);
	void setSize(int w, int h);
	void setDestroyOnClose(bool b) { m_destroy_on_close = b; }
	HWND getWindowHandle() const { return m_hwnd; }
	virtual void init_modal_dialog(HWND hwnd);
	int runModally(HWND parent);
protected:
	HWND m_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	bool m_destroy_on_close = false;
	bool m_is_modal = false;
};

class TestMRPPWindow : public MRPWindow
{
public:
	TestMRPPWindow(HWND parent, std::string title = "Untitled");
	~TestMRPPWindow()
	{
		readbg() << "TestMRPWindow dtor\n";
	}
	void resized() override;
private:
	std::shared_ptr<WinComboBox> m_combo1;
	std::shared_ptr<WinComboBox> m_combo2;
};

HWND open_win_controls_window(HWND parent);
void show_modal_dialog(HWND parent);
