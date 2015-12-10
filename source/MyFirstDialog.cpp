#include "MyFirstDialog.h"
#include "../Visual Studio/resource.h"
#include "../library/reaper_plugin/reaper_plugin_functions.h"
extern HINSTANCE g_hInst;

INT_PTR CALLBACK myfirstdialogproc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
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
		return TRUE; // return true when we did handle the message
	}
	return FALSE; // return false when we didn't handle the message
}

HWND open_my_first_modeless_dialog(HWND parent)
{
	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), parent, myfirstdialogproc, NULL);
}
