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

#include <stdio.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>

reaper_plugin_info_t* g_plugin_info = nullptr;
REAPER_PLUGIN_HINSTANCE g_hInst; // handle to the dll instance. could be useful for making win32 API calls
HWND g_parent; // global variable that holds the handle to the Reaper main window, useful for various win32 API calls

enum toggle_state { CannotToggle, ToggleOff, ToggleOn };

// Little C++ class to deal with the actions
class action_entry {
public:
	action_entry(std::string description, std::string idstring, toggle_state togst, std::function<void(action_entry&)> func) :
		m_desc(description), m_id_string(idstring), m_func(func), m_togglestate(togst) {
		if (g_plugin_info != nullptr) {
			m_accel_reg.accel ={ 0,0,0 };
			m_accel_reg.desc = m_desc.c_str();
			m_accel_reg.accel.cmd = m_command_id = g_plugin_info->Register("command_id", (void*)m_id_string.c_str());
			g_plugin_info->Register("gaccel", &m_accel_reg);
		}
	}
	action_entry(const action_entry&) = delete; // prevent copying
	action_entry& operator=(const action_entry&) = delete; // prevent copying
	action_entry(action_entry&&) = delete; // prevent moving
	action_entry& operator=(action_entry&&) = delete; // prevent moving

	int m_command_id = 0;
	gaccel_register_t m_accel_reg;
	std::function<void(action_entry&)> m_func;
	std::string m_desc;
	std::string m_id_string;
	toggle_state m_togglestate = CannotToggle;
	int m_cycle_state = 0;
};

// use (shared) pointers for the action entries to prevent certain complications
std::vector<std::shared_ptr<action_entry>> g_actions;

std::shared_ptr<action_entry> add_action(std::string name, std::string id, toggle_state togst, std::function<void(action_entry&)> f) {
	auto entry = std::make_shared<action_entry>(name, id, togst, f);
	g_actions.push_back(entry);
	return entry;
}

#include "main.hpp" /*** HERE THE ACTIONS DO THEIR WORK ***/

class function_entry { // Little C++ class to deal with the functions
public:
	function_entry(void* func, std::string func_name, std::string ret_val,
		std::string par_types, std::string par_names, std::string html_help,
		bool c_func_only = false) {
		regkey_func = "API_" + func_name;
		func_c_api = func;
		c_only = c_func_only;

		if (c_only) return;

		func_script = func;
		regkey_vararg = "APIvararg_" + func_name;
		regkey_def = "APIdef_" + func_name;
		document = ret_val + '\0' + par_types + '\0' + par_names + '\0' + html_help + '\0';
	}
	void* func_c_api;
	void* func_script;
	std::string regkey_vararg; // registry type/name for func_vararg
	std::string regkey_func;   // registry type/name for func
	std::string regkey_def;    // registry type/name for dyn_def
	std::string document;      // documentation for function
	bool c_only;
};

std::vector<function_entry> g_functions;

void add_function(void* func, std::string func_name, std::string ret_val, std::string par_types, std::string par_names, std::string html_help, bool c_func_only = false) {
	auto entry = function_entry(func, func_name, ret_val, par_types, par_names, html_help, c_func_only = false);
	g_functions.push_back(entry);
}

#include "reascript.hpp" /*** HERE THE FUNCTIONS DO THEIR WORK ***/

// Reaper calls back to this when it wants to execute an action registered by the extension plugin
bool hookCommandProc(int command, int flag) {
	// it might happen Reaper calls with 0 for the command and if the action
	// registration has failed the plugin's command id would also be 0
	// therefore, check the plugins command id is not 0 and then if it matches with
	// what Reaper called with
	for (auto& e : g_actions) {
		if (e->m_command_id != 0 && e->m_command_id == command) {
			e->m_func(*e);
			return true;
		}
	}	
	return false; // failed to run relevant action
}

// Reaper calls back to this when it wants to know an actions's toggle state
int toggleActionCallback(int command_id) {
	for (auto& e : g_actions) {
		if (command_id != 0 && e->m_togglestate != CannotToggle && e->m_command_id == command_id) {
			if (e->m_togglestate == ToggleOff)
				return 0;
			if (e->m_togglestate == ToggleOn)
				return 1;
		}
	}	
	return -1; // -1 if action not provided by this extension or is not togglable
}

// Register exported function and html documentation
bool RegisterExportedFuncs(reaper_plugin_info_t* rec) {
	bool ok = rec != 0;
	for (auto& e : g_functions) {
		if (!ok) break;

		ok &= rec->Register(e.regkey_func.c_str(), e.func_c_api) != 0;
		ok &= rec->Register(e.regkey_def.c_str(), &e.document[0]) != 0;

		if (e.c_only) continue;

		ok &= rec->Register(e.regkey_vararg.c_str(), e.func_script) != 0;
	}
	return ok;
}

// Unregister exported functions
void UnregisterExportedFuncs() {
	for (auto& e : g_functions) {
		plugin_register(std::string("-"+e.regkey_func).c_str(), e.func_c_api);
	}
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
			add_function((void*)DoublePointer, "MRP_DoublePointer", "double", "double,double", "n1,n2", "add two numbers and return value");
			add_function((void*)IntPointer, "MRP_IntPointer", "int", "int,int", "n1,n2", "add two numbers and return value");

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
