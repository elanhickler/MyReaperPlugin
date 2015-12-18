#include "utilfuncs.h"

// This macro helps in dealing with double-type returns. For some reason doubles must be treated very different.
#define return(v) double* lhs_arg = In(arg[arg_sz-1]); *lhs_arg = (v); return (void*)lhs_arg

function_entry MRP_DoublePointer("double", "double,double", "n1,n2", [](void** arg, int arg_sz) {
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_IntPointer("int", "int,int", "n1,n2", [](void** arg, int arg_sz) {
	int* n1 = In(arg[0]);
	int* n2 = In(arg[1]);

	return Out(*n1+*n2);
},
"add two numbers"
);

function_entry MRP_CalculateEnvelopeHash("int", "TrackEnvelope*", "env", [](void** arg, int arg_sz) {
	TrackEnvelope* env = (TrackEnvelope*)arg[0];
	if (env == nullptr)
		return Out(0);
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
	return Out((int)seed);
},
"This <i>function</i> isn't really <b>correct...</b> it calculates a 64 bit hash "
"but returns it as a 32 bit int. Should reimplement this. "
"Or rather, even more confusingly : The hash will be 32 bit when building "
"for 32 bit architecture and 64 bit when building for 64 bit architecture! "
"It comes down to how size_t is of different size between the 32 and 64 bit "
"architectures."
);

function_entry MRP_DoublePointerAsInt("int", "double,double", "n1,n2", [](void** arg, int arg_sz) {
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return Out(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_CastDoubleToInt("int", "double,double", "n1,n2", [](void** arg, int arg_sz) {
	int n1 = (double)In(arg[0]);
	int n2 = (double)In(arg[1]);

	return Out(n1+n2);
},
"add two numbers"
);

function_entry MRP_CastIntToDouble("double", "int,int", "n1,n2", [](void** arg, int arg_sz) {
	double n1 = (int)In(arg[0]);
	double n2 = (int)In(arg[1]);

	return(n1 + n2);
},
"add two numbers"
);

#undef return