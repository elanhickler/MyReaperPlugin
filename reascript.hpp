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

/*
When documenting API function parameters:
- if a (char*,int) pair is encountered, name them buf, buf_sz
- if a (const char*,int) pair is encountered, buf, buf_sz as well
- if a lone basicType *, use varNameOut or varNameIn or  varNameInOptional (if last parameter(s))
At the moment (REAPER v5pre6) the supported parameter types are:
- int, int*, bool, bool*, double, double*, char*, const char*
- AnyStructOrClass* (handled as an opaque pointer)
At the moment (REAPER v5pre6) the supported return types are:
- int, bool, double, const char*
- AnyStructOrClass* (handled as an opaque pointer)
*/

struct APIdef {
	void* func;                // function to export to C
	const char* func_name;     // function name
	void* func_vararg;         // function to export to EEL,LUA,PY
	const char* regkey_vararg; // registry type/name for func_vararg
	const char* regkey_func;   // registry type/name for func
	const char* regkey_def;    // registry type/name for dyn_def
	std::string ret_val;       // return type, parameter types
	std::string parm_types;    // csv of parameter types
	std::string parm_names;    // csv of names for parameter types
	std::string html_help;     // help text for function
	std::string dyn_def;       // used for dynamic allocations/cleanups
};

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
void* Out(int a) { return (void*)(INT_PTR)a; }
void* Out(bool a) { return (void*)(INT_PTR)a; }
void* Out(const char* a) { return (void*)(INT_PTR)a; }
void* Out(double a) { return (void*)(INT_PTR)a; }

/*** DEFINE EXPORT FUNCTIONS HERE ***/
static void* DoublePointer(void** arg, int arg_sz) {//return:double parameters:double,double
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);
	double* n3 = In(arg[arg_sz-1]);

	*n3 = *n1 + *n2;

	return n3;
}

static void* IntPointer(void** arg, int arg_sz) {//return:int parameters:int,int
	int* n1 = In(arg[0]);
	int* n2 = In(arg[1]);

	return Out(*n1+*n2);
}

static void* DoublePointerAsInt(void** arg, int arg_sz) {//return:int parameters:double,double
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return Out(*n1 + *n2);
}

static void* CastDoubleToInt(void** arg, int arg_sz) {//return:int parameters:double,double
	int n1 = (double)In(arg[0]);
	int n2 = (double)In(arg[1]);

	return Out(n1+n2);
}

static void* CastIntToDouble(void** arg, int arg_sz) {//return:double parameters:int,int
	double n1 = (int)In(arg[0]);
	double n2 = (int)In(arg[1]);
	double* n3 = In(arg[2]);

	*n3 = n1 + n2;

	return n3;
}

#define  APIFUNC(x) (void*)x, #x, (void*) ## x, "APIvararg_" #x "", "API_" #x "", "APIdef_" #x ""
#define CAPIFUNC(x) (void*)x, #x,         NULL,               NULL, "API_" #x "",            NULL // export to C/C++ only
/*** REGISTER EXPORT FUNCTIONS HERE ***/
APIdef g_apidefs[] =
{
	{ APIFUNC(DoublePointer), "double", "double,double", "n1,n2", "Add one to input and return the value", },
	{ APIFUNC(IntPointer), "int", "int,int", "n1,n2", "Add one to input and return the value", },
	{ APIFUNC(DoublePointerAsInt), "int", "double,double", "n1,n2", "Add one to input and return the value", },
	{ APIFUNC(CastDoubleToInt), "int", "double,double", "n1,n2", "Add one to input and return the value", },
	{ APIFUNC(CastIntToDouble), "double", "int,int", "n1,n2", "Add one to input and return the value", },
	{ 0, } // denote end of table
};
#undef APIFUNC
#undef CAPIFUNC