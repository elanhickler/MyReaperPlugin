#include "utilfuncs.h"
#include <unordered_set>
struct in { // Convenience struct to cast void** to supported parameter types
	void* v;

	in(void* const& v) : v(v) {}

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

// These macros helps in dealing with the function definition and supported return types. 
// Use return_int to return anything that is not a double.
#define params void** arg, int arg_sz
#define return_double(v) { double* lhs_arg = (double*)(arg[arg_sz-1]); *lhs_arg = (v); return (void*)lhs_arg; }
#define return_int(v) return (void*)(INT_PTR)(v)
#define return_null return (void*)0

// At the moment (REAPER v5pre6) the supported parameter types are:
//  - int, int*, bool, bool*, double, double*, char*, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)
// At the moment (REAPER v5pre6) the supported return types are:
//  - int, bool, double, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)

function_entry MRP_DoublePointer("double", "double,double", "n1,n2", [](params) {
	double* n1 = (in)arg[0];
	double* n2 = (in)arg[1];

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_IntPointer("int", "int,int", "n1,n2", [](params) {
	int* n1 = (in)arg[0];
	int* n2 = (in)arg[1];

	return_int(*n1+*n2);
},
"add two numbers"
);

std::unordered_set<void*> g_active_mrp_arrays;

function_entry MRP_CreateArray("MRP_Array*", "int", "size", [](params) {
	int* arrsize = (in)arg[0];
	std::vector<double>* ret = new std::vector<double>(*arrsize);
	//readbg() << "returning array " << ret << "\n";
	g_active_mrp_arrays.insert((void*)ret);
	return (void*)ret;
},
"Create an array of 64 bit floating point numbers."
);

function_entry MRP_DestroyArray("", "MRP_Array*", "array", [](params) {
	std::vector<double>* vecptr = (std::vector<double>*)arg[0];
	//readbg() << "should delete array " << vecptr << "\n";
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		//readbg() << "script tried returning invalid pointer for destruction!\n";
		ReaScriptError("Script tried returning invalid MRP_Array for destruction");
		return (void*)nullptr;
	}
	if (vecptr != nullptr)
	{
		delete vecptr;
		g_active_mrp_arrays.erase(arg[0]);
	}
	return (void*)nullptr;
},
"Destroy a previously created MRP_Array"
);

function_entry MRP_GenerateSine("", "MRP_Array*,double,double", "array,samplerate,frequency", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("MRP_GenerateSine : passed in invalid MRP_Array");
		return (void*)nullptr;
	}
	std::vector<double>& vecref = *(std::vector<double>*)arg[0];
	double sr = bound_value(1.0,*(double*)arg[1],1000000.0);
	double hz = bound_value(0.0001,*(double*)arg[2],sr/2.0);
	int numsamples = vecref.size();
	for (int i = 0; i < numsamples; ++i)
		vecref[i] = sin(2*3.141592653*sr*hz*i);
	return (void*)nullptr;
},
"Generate a sine wave into a MRP_Array"
);

function_entry MRP_CalculateEnvelopeHash("int", "TrackEnvelope*", "env", [](params) 
{
	TrackEnvelope* env = (TrackEnvelope*)arg[0];
	if (env == nullptr)
		return_null;
	int numpoints = CountEnvelopePoints(env);
	size_t seed = 0;
	for (int i = 0; i < numpoints; ++i) {
		double pt_time = 0.0;
		double pt_val = 0.0;
		double pt_tension = 0.0;
		int pt_shape = 0;
		GetEnvelopePoint(env, i, &pt_time, &pt_val, &pt_shape, &pt_tension, nullptr);
		hash_combine(seed, pt_time);
		hash_combine(seed, pt_val);
		hash_combine(seed, pt_tension);
		hash_combine(seed, pt_shape);
	}
	return_int((int)seed);
},
"This <i>function</i> isn't really <b>correct...</b> it calculates a 64 bit hash "
"but returns it as a 32 bit int. Should reimplement this. "
"Or rather, even more confusingly : The hash will be 32 bit when building "
"for 32 bit architecture and 64 bit when building for 64 bit architecture! "
"It comes down to how size_t is of different size between the 32 and 64 bit "
"architectures."
);

function_entry MRP_ReturnMediaItem("MediaItem*", "", "", [](params) {
	return_int(GetSelectedMediaItem(0, 0));
},
"return media item"
);

function_entry MRP_DoNothing("", "", "", [](params) {
	return_null;
},
"do nothing, return null"
);

function_entry MRP_DoublePointerAsInt("int", "double,double", "n1,n2", [](params) {
	double* n1 = (in)arg[0];
	double* n2 = (in)arg[1];

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_CastDoubleToInt("int", "double,double", "n1,n2", [](params) { // This is for demonstration purposes.
	int n1 = *(double*)arg[0];                                                     // I don't know why you'd cast the type.
	int n2 = *(double*)arg[1];                                                     // Instead, just use the right input types.

	return_int(n1+n2);
},
"add two numbers"
);

function_entry MRP_CastIntToDouble("double", "int,int", "n1,n2", [](params) {
	double n1 = *(int*)arg[0];
	double n2 = *(int*)arg[1];

	return_int(n1 + n2);
},
"add two numbers"
);

#undef lambda
#undef return_double
#undef return_int
#undef return_void