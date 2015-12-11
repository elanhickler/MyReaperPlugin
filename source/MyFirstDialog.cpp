#include "MyFirstDialog.h"

#include "../Visual Studio/resource.h"
#ifndef _WIN32 // MAC resources
#include "WDL/WDL/swell/swell-dlggen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_dlg"
#undef BEGIN
#undef END
#include "WDL/WDL/swell/swell-menugen.h"
#include "../Visual Studio/MyReaperPlugin.rc_mac_menu"
#endif

#include "WDL/WDL/lice/lice.h"
#include "../library/reaper_plugin/reaper_plugin_functions.h"
#include "MyLiceWindow.h"

#include <memory>

extern HINSTANCE g_hInst;

INT_PTR CALLBACK myfirstdialogproc(
	HWND   hwndDlg,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	// The "canonical" way is to use a switch case for the messages, but ifs will work fine too
	// and it's less likely to do some stupid mistakes this way
	if (uMsg == WM_INITDIALOG) // The very first message sent for the window
	{
		ShowWindow(hwndDlg, SW_SHOW);
		// SetWindowText is a "polymorphic" API that can change the visible text of various things,
		// including window titles or text edit box texts
		SetWindowText(hwndDlg, "My First Dialog");
		//SetWindowPos(hwndDlg,0,20,80,200,100,SWP_NOZORDER|SWP_NOACTIVATE);
		return TRUE;
	}
	// WM_CLOSE sent at least when the window close button is pressed
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwndDlg);
		return TRUE;
	}
	// Button clicks and various other things will cause a WM_COMMAND message to be sent
	if (uMsg == WM_COMMAND)
	{
		// Check the source of the message is our button and that the message is the button click
		if (LOWORD(wParam) == IDC_DO1 && HIWORD(wParam) == BN_CLICKED)
		{
			if (CountSelectedMediaItems(nullptr) > 0)
			{
				MediaItem* item = GetSelectedMediaItem(nullptr, 0);
				MediaItem_Take* take = GetActiveTake(item);
				if (take != nullptr)
				{
					char buf[1024];
					if (GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false)==true)
						SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), buf);
				}
				
			}
			else 
				SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), "No item selected!");
			
		}
		if (LOWORD(wParam) == IDC_DO2 && HIWORD(wParam) == BN_CLICKED)
		{
			if (CountSelectedMediaItems(nullptr) > 0)
			{
				MediaItem* item = GetSelectedMediaItem(nullptr, 0);
				MediaItem_Take* take = GetActiveTake(item);
				if (take != nullptr)
				{
					char buf[1024];
					GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), buf, 1024);
					GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, true);
					UpdateArrange();
				}
			}
		}
		return TRUE; // return true when we did handle the message
	}
	return FALSE; // return false when we didn't handle the message
}

HWND open_my_first_modeless_dialog(HWND parent)
{
	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), parent, myfirstdialogproc, NULL);
}

std::unique_ptr<TestControl> g_testcontrol;

INT_PTR CALLBACK mylicedialogproc(
	HWND   hwndDlg,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Lice Test");
		SetWindowPos(hwndDlg, NULL, 20, 60, 110, 130, SWP_NOACTIVATE | SWP_NOZORDER);
		ShowWindow(hwndDlg, SW_SHOW);
		g_testcontrol = std::make_unique<TestControl>(hwndDlg);
		return TRUE;
	}
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwndDlg);
		return TRUE;
	}
	if (uMsg == WM_SIZE)
	{
		if (g_testcontrol != nullptr)
		{
			RECT r;
			GetClientRect(hwndDlg, &r);
			g_testcontrol->setSize(r.right - r.left, r.bottom - r.top);
			InvalidateRect(hwndDlg, NULL, TRUE);
		}
		return TRUE;
	}
	return FALSE;
}




HWND open_lice_dialog(HWND parent)
{
	HWND dh = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), parent, mylicedialogproc, NULL);
	return dh;
}
