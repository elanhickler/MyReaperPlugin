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