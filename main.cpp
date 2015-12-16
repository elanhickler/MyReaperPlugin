// check if the build environment is Windows, and then include the Windows API header,
// else include SWELL which provides functions with the same names but are implemented
// for another operating system (namely, OS-X)
#ifdef _WIN32
#include <windows.h>
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
			if (REAPERAPI_LoadAPI(rec->GetFunc) > 0) { return 0; /*todo: proper error*/ }

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
			
			// Add action to show dialog box
			add_action("Show my first dialog", "EXAMPLE_ACTION_SHOW_DIALOG1", ToggleOff, [](action_entry&) 
			{
				HWND hw = open_my_first_modeless_dialog(g_parent);
				if (hw==NULL)
				{
					ShowConsoleMsg("Failed to create dialog window\n");
				}
			});
			
			// Add actions to show lice window
			add_action("Show my Lice dialog", "EXAMPLE_ACTION_SHOW_DIALOG2", ToggleOff, [](action_entry&)
			{
				open_lice_dialog(g_parent);
			});

			// Add functions
			add_function(MRP_DoublePointer, "MRP_DoublePointer", "double", "double,double", "n1,n2", "add two numbers and return value");
			add_function(MRP_IntPointer, "MRP_DoublePointer", "int", "int,int", "n1,n2", "add two numbers and return value");

			if (!rec->Register("hookcommand", (void*)hookCommandProc)) { /*todo: error*/ }
			if (!rec->Register("toggleaction", (void*)toggleActionCallback)) { /*todo: error*/ }
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
