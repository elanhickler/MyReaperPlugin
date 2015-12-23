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

class MRPWindow
{
public:
	MRPWindow(HWND parent, std::string title = "Untitled");
	virtual ~MRPWindow();
	virtual void resized() {};
	virtual void closeRequested();
	std::pair<int, int> getSize();
	void setPosition(int x, int y);
	void setSize(int w, int h);
	void setDestroyOnClose(bool b) { m_destroy_on_close = b; }
	HWND getWindowHandle() const { return m_hwnd; }
protected:
	HWND m_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	bool m_destroy_on_close = false;
};

class TestMRPPWindow : public MRPWindow
{
public:
	TestMRPPWindow(HWND parent, std::string title = "Untitled") : MRPWindow(parent, title)
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
	//void closeRequested() override;

};

HWND open_win_controls_window(HWND parent);