#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include <memory>
#include <functional>

class ReaperDialog
{
public:
	ReaperDialog(HWND parent, int dialogresource);
	virtual ~ReaperDialog();
	std::function<bool(HWND, UINT, WPARAM, LPARAM)> DialogProc;
	
private:
	HWND m_hwnd = NULL;
	static INT_PTR CALLBACK dlgproc(HWND, UINT, WPARAM, LPARAM);
};



HWND open_my_first_modeless_dialog(HWND parent);
HWND open_lice_dialog(HWND parent);