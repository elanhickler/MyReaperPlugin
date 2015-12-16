#pragma once

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <string>
#include <functional>
#include <vector>

class function_entry { // Little C++ class to deal with the functions
public:
	function_entry(void* func, std::string func_name, std::string ret_val,
		std::string par_types, std::string par_names, std::string html_help,
		bool c_func_only = false);

	void* func_c_api;  // function pointer for c api
	void* func_script; // function pointer for reascript
	std::string regkey_vararg; // registry type/name for func_vararg
	std::string regkey_func;   // registry type/name for func
	std::string regkey_def;    // registry type/name for dyn_def
	std::string document;      // documentation for function
	bool c_only;
};

void impl_add_function(void* func, std::string func_name, std::string ret_val, std::string par_types, std::string par_names, std::string html_help, bool c_func_only);
template<typename F>
inline void add_function(F func, std::string func_name, std::string ret_val, std::string par_types, std::string par_names, std::string html_help, bool c_func_only = false)
{
	impl_add_function((void*)func, func_name, ret_val, par_types, par_names, html_help, c_func_only);
}

// Register exported function and html documentation
bool RegisterExportedFuncs(reaper_plugin_info_t* rec);

// Unregister exported functions
void UnregisterExportedFuncs();

struct In { // Helpers for creating export functions
	void* v;

	In(void* const& v) : v(v) {}

	operator double() { return *(double*)v; }
	operator double*() { return (double*)v; }

	operator int() { return (int)(INT_PTR)v; }
	operator int*() { return (int*)(INT_PTR)&v; }

	operator bool() { return *(bool*)v; }
	operator bool*() { return (bool*)v; }

	operator char() { return *(char*)v; }
	operator char*() { return (char*)v; }
	operator const char*() { return (const char*)v; }
};
inline void* Out(int a);
inline void* Out(bool a);
inline void* Out(const char* a);
inline void* Out(double a);