// check if the build environment is Windows, and then include the Windows API header,
// else include SWELL which provides functions with the same names but are implemented
// for another operating system (namely, OS-X)
#ifdef _WIN32
#include <windows.h>
#pragma warning ( disable : 4267 ) // size_t to int
#pragma warning ( disable : 4244 ) // double to int
#pragma warning ( disable : 4800 ) // 'unsigned __int64': forcing value to bool 'true' or 'false' (performance warning)
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin/reaper_plugin_functions.h"

#include "utilfuncs.h"
#include "reaper_action_helper.h"
#include "reaper_function_helper.h"

#include <stdio.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>

reaper_plugin_info_t* g_plugin_info = nullptr;
REAPER_PLUGIN_HINSTANCE g_hInst; // handle to the dll instance. could be useful for making win32 API calls
HWND g_parent; // global variable that holds the handle to the Reaper main window, useful for various win32 API calls

#include "main.hpp" 
#include "reascript.hpp" /*** HERE THE FUNCTIONS DO THEIR WORK ***/

void doAction1() {
	ShowMessageBox("Hello World!", "Reaper extension", 0);
}

void doAction2(action_entry& act) {
	// this action does nothing else but toggles the variable that keeps track of the toggle state
	// so it's useless as such but you can see the action state changing in the toolbar buttons and the actions list
	if (act.m_togglestate == ToggleOff)
		act.m_togglestate = ToggleOn;
	else act.m_togglestate = ToggleOff;
	// store new state of toggle action to ini file immediately
	char buf[8];
	// the REAPER api for ini-files only deals with strings, so form a string from the action
	// toggle state number.
	int toggletemp = 0;
	if (act.m_togglestate == ToggleOn)
		toggletemp = 1;
	sprintf(buf, "%d", toggletemp);
	SetExtState("simple_extension", "toggleaction_state", buf, true);
}

void doAction3(action_entry& act) {
	readbg() << "action in cycle state " << act.m_cycle_state << "\n";
	act.m_cycle_state = (act.m_cycle_state + 1) % 3;
}

void doChangeItemPitchesAction(action_entry& act)
{
	int num_sel_items = CountSelectedMediaItems(nullptr);
	for (int i = 0; i < num_sel_items; ++i)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr, i);
		MediaItem_Take* take = GetActiveTake(item);
		if (take != nullptr)
		{
			double pch = bound_value(-12.0,
				map_value(act.m_ex_val, 0, 127, -12.0, 12.0),12.0);
			SetMediaItemTakeInfo_Value(take, "D_PITCH", pch);
		}
	}
	UpdateArrange();
}

extern "C"
{
	// this is the only function that needs to be exported by a Reaper extension plugin dll
	// everything then works from function pointers and others things initialized within this
	// function
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec) {
		g_hInst=hInstance;
		if (rec) {
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0; /*todo: proper error*/
			g_plugin_info = rec;
			g_parent = rec->hwnd_main;

			// load all Reaper API functions in one go, byebye ugly IMPAPI macro!
			int error_count = REAPERAPI_LoadAPI(rec->GetFunc);
			if (error_count > 0) 
			{ 
				char errbuf[256];
				sprintf(errbuf, "Failed to load %d expected API function(s)", error_count);
				MessageBox(g_parent, errbuf, "MRP extension error", MB_OK);
				return 0;
			}
			
#ifndef WIN32
			// Perhaps to get Reaper faders on OSX...
			SWELL_RegisterCustomControlCreator((SWELL_ControlCreatorProc)rec->GetFunc("Mac_CustomControlCreator"));
#endif
			// Use C++11 lambda to call the doAction1() function that doesn't have the action_entry& as input parameter
			add_action("Simple extension test action", "EXAMPLE_ACTION_01", CannotToggle, [](action_entry&) { doAction1(); });

			// Pass in the doAction2() function directly since it's compatible with the action adding function signature
			auto togact = add_action("Simple extension togglable test action", "EXAMPLE_ACTION_02", ToggleOff, doAction2);

			// Use C++11 lambda to directly define the action code right here
			add_action("Simple extension another test action", "EXAMPLE_ACTION_03", CannotToggle,
				[](action_entry&) { ShowMessageBox("Hello from C++11 lambda!", "Reaper extension API test", 0); });

			// Add 4 actions in a loop, using C++11 lambda capture [i] to make a small customization for each action
			for (int i = 0; i < 4; ++i) {
				auto actionfunction = [i](action_entry&) {
					std::string message = "You called action " + std::to_string(i + 1);
					ShowMessageBox(message.c_str(), "Reaper extension API test", 0);
				};
				std::string desc = "Simple extension loop created action " + std::to_string(i + 1);
				std::string id = "EXAMPLE_ACTION_FROM_LOOP" + std::to_string(i);
				add_action(desc, id, CannotToggle, actionfunction);
			}
			
			// Add actions to show WinControl containing windows
			add_action("MRP : Toggle simple example window", "MRP_SHOW_WINCONTROLSIMPLETEST", ToggleOff, [](action_entry&)
			{
				toggle_simple_example_window(g_parent);
			});

			add_action("MRP : Add WinControls test window", "MRP_SHOW_WINCONTROLSTEST", ToggleOff, [](action_entry&)
			{
				open_win_controls_window(g_parent);
			});
#ifdef MODALWINDOWSWORKPROPERLY
			add_action("MRP : Show modal dialog...", "MRP_SHOW_WINMODAL", ToggleOff, [](action_entry&)
			{
				show_modal_dialog(g_parent);
			});
#endif
			add_action("MRP : Test mousewheel/MIDI CC action", "MRP_TESTWHEELMIDICC", ToggleOff, doChangeItemPitchesAction);
			
				// Add functions
#define func(f) add_function(f, #f)
			func(MRP_DoublePointer);
			func(MRP_IntPointer);
			func(MRP_CalculateEnvelopeHash);
			func(MRP_DoublePointerAsInt);
			func(MRP_CastDoubleToInt);
			func(MRP_ReturnMediaItem);
			func(MRP_DoNothing);
			func(MRP_CreateWindow);
			func(MRP_DestroyWindow);
			func(MRP_WindowSetTitle);
			func(MRP_WindowIsClosed);
			func(MRP_GetControlText);
			func(MRP_SetControlText);
			func(MRP_GetControlFloatNumber);
			func(MRP_SetControlFloatNumber);
			func(MRP_SetWindowDirty);
			func(MRP_GetWindowDirty);
			func(MRP_SetControlBounds);
			func(MRP_GetWindowPosSizeValue);
			func(MRP_WindowAddSlider);
			func(MRP_WindowAddButton);
			func(MRP_WindowAddLineEdit);
			func(MRP_WindowAddLabel);
			func(MRP_WindowAddLiceControl);
			func(MRP_WindowIsDirtyControl);
			func(MRP_WindowClearDirtyControls);
			func(MRP_CreateArray);
			func(MRP_DestroyArray);
			func(MRP_GenerateSine);
			func(MRP_WriteArrayToFile);
			func(MRP_MultiplyArrays);
			func(MRP_SetArrayValue);
			func(MRP_GetArrayValue);
#ifdef WIN32
			func(MRP_MultiplyArraysMT);
#endif
#undef func

			if (!rec->Register("hookcommand2", (void*)hookCommandProcEx)) { 
				MessageBox(g_parent, "Could not register hookcommand2", "MRP extension error", MB_OK);
			}
			if (!rec->Register("toggleaction", (void*)toggleActionCallback)) { 
				MessageBox(g_parent, "Could not register toggleaction", "MRP extension error", MB_OK);
			}
			if (!RegisterExportedFuncs(rec)) { /*todo: error*/ }

			// restore extension global settings
			// saving extension data into reaper project files is another thing and 
			// at the moment not done in this example plugin
			if (togact->m_command_id != 0) {
				const char* numberString = GetExtState("simple_extension", "toggleaction_state");
				if (numberString != nullptr) {
					int initogstate = atoi(numberString);
					if (initogstate == 1)
						togact->m_togglestate = ToggleOn;
				}
			}		
			return 1; // our plugin registered, return success
		}
		else {
			
			return 0;
		}
	}
};
