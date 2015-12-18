#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <string>
#include <functional>
#include <vector>

#include "reaper_function_helper.h"

std::vector<function_entry> g_functions;

function_entry::function_entry(std::string ret_val, std::string par_types, std::string par_names, reaper_function_t func,
	std::string html_help, bool c_func_only) : m_function(func), c_only(c_func_only) {
	func_c_api = (void*)m_function;
	func_script = (void*)m_function;
	document = ret_val + '\0' + par_types + '\0' + par_names + '\0' + html_help + '\0';
}

void add_function(function_entry& f, const std::string& name) {
	f.regkey_func = "API_" + name;
	f.regkey_vararg = "APIvararg_" + name;
	f.regkey_def = "APIdef_" + name;
	g_functions.push_back(f);
}

bool RegisterExportedFuncs(reaper_plugin_info_t* rec) {
	bool ok = rec != 0;
	for (auto& f : g_functions) {
		if (!ok) break;

		ok &= rec->Register(f.regkey_func.c_str(), f.func_c_api) != 0;
		ok &= rec->Register(f.regkey_def.c_str(), &f.document[0]) != 0;

		if (f.c_only) continue;

		ok &= rec->Register(f.regkey_vararg.c_str(), f.func_script) != 0;
	}
	return ok;
}

void UnregisterExportedFuncs() {
	for (auto& f : g_functions) {
		plugin_register(std::string("-"+f.regkey_func).c_str(), f.func_c_api);
	}
}