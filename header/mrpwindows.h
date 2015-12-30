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
#include "mrpwincontrols.h"
#include "mylicecontrols.h"

class MRPWindow
{
public:
	MRPWindow() {}
	MRPWindow(HWND parent, std::string title = "Untitled");
	virtual ~MRPWindow();
	void add_control(std::shared_ptr<WinControl> c);
	virtual void resized() {};
	virtual void closeRequested();
	bool isVisible() const;
	void setVisible(bool b);
	bool isClosed() { return m_is_closed; }
	void setWindowTitle(std::string title);
	MRP::Size getSize();
	void setPosition(int x, int y);
	void setSize(int w, int h);
	MRP::Rectangle getBounds() const;
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
	virtual void onRefreshTimer() {}
protected:
	HWND m_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	bool m_destroy_on_close = false;
	bool m_is_modal = false;
	bool m_modal_should_end = false;
	ModalResult m_modal_result = Rejected;
	UINT_PTR m_helper_timer = 0;
	bool m_is_closed = true;
};

void shutdown_windows();
