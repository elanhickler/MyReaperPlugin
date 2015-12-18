#pragma once

using reaper_function_t = void*(*)(void**, int);

class function_entry { // Little C++ class to deal with the functions
public:
	function_entry(std::string ret_val, std::string par_types, std::string par_names, reaper_function_t func,
		std::string html_help, bool c_func_only = false);

	void* func_c_api;  // function pointer for c api
	void* func_script; // function pointer for reascript
	std::string regkey_vararg; // registry type/name for func_vararg
	std::string regkey_func;   // registry type/name for func
	std::string regkey_def;    // registry type/name for dyn_def
	std::string document;      // documentation for function
	bool c_only;
	reaper_function_t m_function;
};

void add_function(function_entry& f, const std::string& name);

// Register exported function and html documentation
bool RegisterExportedFuncs(reaper_plugin_info_t* rec);

// Unregister exported functions
void UnregisterExportedFuncs();