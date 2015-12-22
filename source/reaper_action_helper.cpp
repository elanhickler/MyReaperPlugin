#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include "reaper_action_helper.h"

extern reaper_plugin_info_t* g_plugin_info;

action_entry::action_entry(std::string description, std::string idstring, toggle_state togst, std::function<void(action_entry&)> func) :
m_desc(description), m_id_string(idstring), m_func(func), m_togglestate(togst) {
	if (g_plugin_info != nullptr) {
		m_accel_reg.accel ={ 0,0,0 };
		m_accel_reg.desc = m_desc.c_str();
		m_accel_reg.accel.cmd = m_command_id = g_plugin_info->Register("command_id", (void*)m_id_string.c_str());
		g_plugin_info->Register("gaccel", &m_accel_reg);
	}
}

/* Use (shared) pointers for the action entries to prevent certain complications, that is
 complications with how the action_entry objects should be copied or even moved if they
 were handled as C++ values. This way we can just create them once and make them point to the pointers and that's it.
 */
std::vector<std::shared_ptr<action_entry>> g_actions;

std::shared_ptr<action_entry> add_action(std::string name, std::string id, toggle_state togst, std::function<void(action_entry&)> f) {
	auto entry = std::make_shared<action_entry>(name, id, togst, f);
	g_actions.push_back(entry);
	return entry;
}

// Reaper calls back to this when it wants to execute an action registered by the extension plugin
// This version of the callback provides more information such as mousewheel direction/amount and MIDI CC
// value
bool hookCommandProcEx(KbdSectionInfo *sec, int command, int val, int valhw, int relmode, HWND hwnd)
{
	// it might happen Reaper calls with 0 for the command and if the action
	// registration has failed the plugin's command id would also be 0
	// therefore, check the plugins command id is not 0 and then if it matches with
	// what Reaper called with
	
	// We only do main window actions, so don't even bother iterating if the kbd section
	// isn't the main window one
	if (sec != nullptr && strcmp(sec->name,"Main") == 0)
	{
		//readbg() << "hookCommandProcEx : " << command << " " << val << " " << valhw << " " << relmode << "\n";
		for (auto& e : g_actions) {
			if (e->m_command_id != 0 && e->m_command_id == command) {
				e->m_ex_val = val;
				e->m_ex_valhw = valhw;
				e->m_ex_relmode = relmode;
				e->m_func(*e);
				return true;
			}
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
