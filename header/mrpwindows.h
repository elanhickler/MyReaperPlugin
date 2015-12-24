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
	bool isVisible() const;
	void setVisible(bool b);
	MRP::Size getSize();
	void setPosition(int x, int y);
	void setSize(int w, int h);
	void setDestroyOnClose(bool b) { m_destroy_on_close = b; }
	HWND getWindowHandle() const { return m_hwnd; }
	
	virtual void init_modal_dialog() {}
	enum ModalResult
	{
		Undefined,
		Accepted,
		Rejected
	};
	ModalResult runModally(HWND parent);
	
	void finishModal(ModalResult result)
	{
		if (m_is_modal == true && m_hwnd!=NULL)
		{
			m_modal_result = result;
			m_modal_should_end = true;
			return;
			if (result == Accepted)
				EndDialog(m_hwnd, 1);
			if (result == Rejected)
				EndDialog(m_hwnd, 2);
		}
	}
protected:
	HWND m_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	bool m_destroy_on_close = false;
	bool m_is_modal = false;
	bool m_modal_should_end = false;
	ModalResult m_modal_result = Rejected;
};

class TestMRPModalWindow : public MRPWindow
{
public:
	void init_modal_dialog();
	std::shared_ptr<WinLineEdit> m_line_edit;
	
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
	std::shared_ptr<ReaSlider> m_slider1;
	std::shared_ptr<EnvelopeControl> m_envcontrol1;
	std::shared_ptr<WinLabel> m_label1;
	std::shared_ptr<WinLineEdit> m_edit1;
};

HWND open_win_controls_window(HWND parent);
void show_modal_dialog(HWND parent);

void shutdown_windows();
