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