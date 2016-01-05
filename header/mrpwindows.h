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
	virtual void onModalClose() {}
	
	virtual void onRefreshTimer() {}
protected:
	HWND m_hwnd = NULL;
	HWND m_parent_hwnd = NULL;
	std::vector<std::shared_ptr<WinControl>> m_controls;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
	bool m_destroy_on_close = false;
	bool m_is_modal = false;
	ModalResult m_modal_result = Undefined;
	UINT_PTR m_helper_timer = 0;
	bool m_is_closed = true;
	void onTimer();
	void finishModal(ModalResult result);
	std::string m_modal_title;
};

/*
OK, so WinAPI modal dialogs are a complete pain to abstract into C++...
MRPModalDialog attempts to do some abstraction but it's still tricky :
1) Subclass MRPModalDialog
2) Don't do anything like add new WinControls in the constructor. There's no parent window
yet to which they could be parented.
3) Override the init_modal_dialog method. Do your control adding and initing there.
4) Override the onModalClose method. Here you have the last chance to gather any data from the WinControls that 
the modal dialog is supposed to hand back. DO NOT attempt that after the runModally method returns. The child 
WinControls are already gone at that point and things like GetWindowText etc won't work. You need to have copies
of the desired data cached in member variables. 

TestMRPModalWindow and show_modal_dialog in mrpexamplewindows.h/.cpp attempt to demonstrate how this all 
should be put together.

Aren't modal dialogs such fun? 

*/

class MRPModalDialog : public MRPWindow
{
public:
	MRPModalDialog(HWND parent, std::string title = "Untitled");
	ModalResult runModally();
	
private:
	
};

bool is_valid_mrp_window(MRPWindow* w);

void shutdown_windows();
