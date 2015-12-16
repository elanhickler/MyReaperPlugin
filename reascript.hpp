#include "utilfuncs.h"

static void* MRP_DoublePointer(void** arg, int arg_sz) {//return:double parameters:double,double
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);
	double* n3 = In(arg[arg_sz-1]);

	*n3 = *n1 + *n2;

	return n3;
}

static void* MRP_IntPointer(void** arg, int arg_sz) {//return:int parameters:int,int
	int* n1 = In(arg[0]);
	int* n2 = In(arg[1]);

	return Out(*n1+*n2);
}

// This function isn't really correct...it calculates a 64 bit hash
// but returns it as a 32 bit int. Should reimplement this.
static void* MRP_CalculateEnvelopeHash(void** arg, int arg_sz)
{
	TrackEnvelope* env = (TrackEnvelope*)arg[0];
	if (env == nullptr)
		return Out(0);
	int numpoints = CountEnvelopePoints(env);
	size_t seed = 0;
	for (int i = 0; i < numpoints; ++i)
	{
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
}


static void* MRP_DoublePointerAsInt(void** arg, int arg_sz) {//return:int parameters:double,double
	double* n1 = In(arg[0]);
	double* n2 = In(arg[1]);

	return Out(*n1 + *n2);
}

static void* MRP_CastDoubleToInt(void** arg, int arg_sz) {//return:int parameters:double,double
	int n1 = (double)In(arg[0]);
	int n2 = (double)In(arg[1]);

	return Out(n1+n2);
}

static void* MRP_CastIntToDouble(void** arg, int arg_sz) {//return:double parameters:int,int
	double n1 = (int)In(arg[0]);
	double n2 = (int)In(arg[1]);
	double* n3 = In(arg[2]);

	*n3 = n1 + n2;

	return n3;
}