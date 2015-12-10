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
	if (uMsg == WM_INITDIALOG)
	{
		ShowWindow(hwndDlg, SW_SHOW);
		SetWindowText(hwndDlg, L"My First Dialog");
		return TRUE;
	}
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwndDlg);
		return TRUE;
	}
	if (uMsg == WM_COMMAND)
	{
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
						SetWindowTextA(GetDlgItem(hwndDlg, IDC_EDIT1), buf);
				}
				
			}
			else 
				SetWindowTextA(GetDlgItem(hwndDlg, IDC_EDIT1), "No item selected!");
			
		}
		return TRUE;
	}
	return FALSE;
}

HWND open_my_first_modeless_dialog(HWND parent)
{
	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), parent, myfirstdialogproc, NULL);
}
