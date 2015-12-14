/******************************************************************************
/ ReaScript.cpp
/
/ Copyright (c) 2012 Jeffos
/
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

/*DEFINE EXPORT FUNCTIONS HERE*/
int AddTwoNumbers(int n1, int n2) { return n1 + n2; }

// Include generated file from reascript_vararg.php
#ifdef _WIN32
#pragma warning(push, 0)
#pragma warning(disable: 4800) // disable "forcing value to bool..." warnings
#endif
#include "reascript_vararg.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Important, keep APIFUNC() as it is defined: both scripts reascript_vararg.php
// and reascript_python.pl parse g_apidefs to generate variable argument wrappers
// for EEL and Lua (reascript_vararg.h, automatically generated at compilation time
// on OSX), as well as python wrappers (sws_python.py, automatically generated at
// compilation time both on Win & OSX).

#define APIFUNC(x) (void*)x,#x,(void*)__vararg_ ## x,"APIvararg_" #x "","API_" #x "","APIdef_" #x ""
#define CAPIFUNC(x) (void*)x,#x,NULL,NULL,"API_" #x "",NULL // export to C/C++ only

struct APIdef {
	void* func;
	const char* func_name;
	void* func_vararg;
	const char* regkey_vararg;
	const char* regkey_func;
	const char* regkey_def;
	const char* ret_val;
	const char* parm_types;

	// if additionnal data are needed, add them below (see top remark)
	const char* parm_names;
	const char* help;
	char* dyn_def; // used for dynamic allocations/cleanups
};

// Add functions to array
APIdef g_apidefs[] =
{
	{ APIFUNC(AddTwoNumbers), "int", "int,int", "n1,n2", "Add one to input and return the value", },
	{ 0, } // denote end of table
};

///////////////////////////////////////////////////////////////////////////////

// register exported functions
bool RegisterExportedFuncs(reaper_plugin_info_t* _rec) {
	bool ok = (_rec!=NULL);
	int i=-1;
	while (ok && g_apidefs[++i].func) {
		ok &= (_rec->Register(g_apidefs[i].regkey_func, g_apidefs[i].func) != 0);
		if (g_apidefs[i].regkey_vararg && g_apidefs[i].func_vararg) {
			ok &= (_rec->Register(g_apidefs[i].regkey_vararg, g_apidefs[i].func_vararg) != 0);
		}
	}
	return ok;
}

// unregister exported functions
void UnregisterExportedFuncs() {
	char tmp[512];
	int i=-1;
	while (g_apidefs[++i].func) {
		snprintf(tmp, sizeof(tmp), "-%s", g_apidefs[i].regkey_func);
		plugin_register(tmp, g_apidefs[i].func);
	}
}

// register exported function definitions (html documentation)
bool RegisterExportedAPI(reaper_plugin_info_t* _rec) {
	bool ok = (_rec!=NULL);
	int i=-1;
	char tmp[8*1024];
	while (ok && g_apidefs[++i].func) {
		if (g_apidefs[i].regkey_def) {
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "%s\r%s\r%s\r%s", g_apidefs[i].ret_val, g_apidefs[i].parm_types, g_apidefs[i].parm_names, g_apidefs[i].help);
			char* p = g_apidefs[i].dyn_def = strdup(tmp);
			while (*p) { if (*p=='\r') *p='\0'; p++; }
			ok &= (_rec->Register(g_apidefs[i].regkey_def, g_apidefs[i].dyn_def) != 0);
		}
	}
	return ok;
}