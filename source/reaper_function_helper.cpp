#include "reaper_function_helper.h"

std::vector<function_entry> g_functions;

function_entry::function_entry(void* func, std::string func_name, std::string ret_val,
	std::string par_types, std::string par_names, std::string html_help,	bool c_func_only) {
	regkey_func = "API_" + func_name;
	func_c_api = func;
	c_only = c_func_only;

	if (c_only) return;

	func_script = func;
	regkey_vararg = "APIvararg_" + func_name;
	regkey_def = "APIdef_" + func_name;
	document = ret_val + '\0' + par_types + '\0' + par_names + '\0' + html_help + '\0';
}

void impl_add_function(void* func, std::string func_name, std::string ret_val, std::string par_types, std::string par_names, std::string html_help, bool c_func_only) {
	g_functions.push_back(function_entry(func, func_name, ret_val, par_types, par_names, html_help, c_func_only));
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

void* Out(int a) { return (void*)(INT_PTR)a; }
void* Out(bool a) { return (void*)(INT_PTR)a; }
void* Out(const char* a) { return (void*)(INT_PTR)a; }
void* Out(double a) { return (void*)(INT_PTR)a; }