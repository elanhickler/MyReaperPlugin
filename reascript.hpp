#include "utilfuncs.h"

// At the moment (REAPER v5pre6) the supported parameter types are:
//  - int, int*, bool, bool*, double, double*, char*, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)
// At the moment (REAPER v5pre6) the supported return types are:
//  - int, bool, double, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)

// These macros helps in dealing with various return types. 
// Use return_int to return anything that is not a double.
#define lambda void** arg, int arg_sz
#define return_double(v) { double* lhs_arg = In(arg[arg_sz-1]); *lhs_arg = (v); return (void*)lhs_arg; }
#define return_int(v) return (void*)(INT_PTR)(v)
#define return_void return (void*)0

function_entry MRP_DoublePointer("double", "double,double", "n1,n2", [](lambda) {
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_IntPointer("int", "int,int", "n1,n2", [](lambda) {
	int* n1 = In(arg[0]);
	int* n2 = In(arg[1]);

	return_int(*n1+*n2);
},
"add two numbers"
);

function_entry MRP_CalculateEnvelopeHash("int", "TrackEnvelope*", "env", [](lambda) {
	TrackEnvelope* env = (TrackEnvelope*)arg[0];
	if (env == nullptr)
		return_void;
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

function_entry MRP_ReturnMediaItem("MediaItem*", "", "item", [](lambda) {
	return_int(GetSelectedMediaItem(0, 0));
},
"return media item"
);

function_entry MRP_ReturnVoid("MediaItem*", "", "item", [](lambda) {
	return_void;
},
"return nothing"
);

function_entry MRP_DoublePointerAsInt("int", "double,double", "n1,n2", [](lambda) {
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_CastDoubleToInt("int", "double,double", "n1,n2", [](lambda) {
	int n1 = (double)In(arg[0]);
	int n2 = (double)In(arg[1]);

	return_int(n1+n2);
},
"add two numbers"
);

function_entry MRP_CastIntToDouble("double", "int,int", "n1,n2", [](void** arg, int arg_sz) {
	double n1 = (int)In(arg[0]);
	double n2 = (int)In(arg[1]);

	return_int(n1 + n2);
},
"add two numbers"
);

#undef lambda
#undef return_double
#undef return_int
#undef return_void