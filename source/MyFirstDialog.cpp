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
#include "reaper_plugin/reaper_plugin_functions.h"
#include "mylicecontrols.h"
#include <unordered_map>
#include <memory>
#include "utilfuncs.h"
#ifdef WIN32
#include "Commctrl.h"
#endif
extern HINSTANCE g_hInst;

#ifdef FOOFOOZ
// keep this around for slightly longer
INT_PTR CALLBACK xycontroldlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Multi XY control");
		g_xycontrol = std::make_unique<TestControl>(hwndDlg);
		g_xycontrol->PointMovedCallback = [ptr=g_xycontrol.get()](int ptindex, double x, double y)
		{
			fx_param_t* x_target = ptr->getFXParamTarget(ptindex, 0);
			fx_param_t* y_target = ptr->getFXParamTarget(ptindex, 1);
			if (x_target != nullptr && y_target != nullptr)
			{
				if (x_target->tracknum >= 1)
					TrackFX_SetParamNormalized(GetTrack(nullptr, x_target->tracknum - 1),
						x_target->fxnum, x_target->paramnum, x);
				if (y_target->tracknum >= 1)
					TrackFX_SetParamNormalized(GetTrack(nullptr, y_target->tracknum - 1),
						y_target->fxnum, y_target->paramnum, y);
			}
		};
		ShowWindow(hwndDlg, SW_SHOW);
		return TRUE;
	}
	if (uMsg == WM_CLOSE)
	{
		ShowWindow(hwndDlg, SW_HIDE);
		return TRUE;
	}
	if (uMsg == WM_SIZE)
	{
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;
		g_xycontrol->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}
#endif

void clean_up_gui()
{
	
}
