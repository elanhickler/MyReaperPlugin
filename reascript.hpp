#include "utilfuncs.h"
#include <unordered_set>
#ifdef WIN32
#include <ppl.h>
#endif
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
// Use return_int to return anything that is not a double. Alternatively, use obj for class/struct.
#define params void** arg, int arg_sz
#define return_double(v) { double* lhs_arg = (double*)(arg[arg_sz-1]); *lhs_arg = (v); return (void*)lhs_arg; }
#define return_int(v) return (void*)(INT_PTR)(v)
#define return_null return (void*)0
#define return_obj(c) return (void*)c

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
	char buf[256];
	int* arrsize = (in)arg[0];
	std::vector<double>* ret;
	try
	{
		ret = new std::vector<double>(*arrsize);
		g_active_mrp_arrays.insert((void*)ret);
		return (void*)ret;
	}
	catch (std::exception& ex)
	{
		sprintf(buf, "MRP_CreateArray : failed to create array (%s)", ex.what());
		ReaScriptError(buf);
	}
	return_null;
},
"Create an array of 64 bit floating point numbers. Note that these will leak memory if they are not later destroyed with MRP_DestroyArray!"
);

function_entry MRP_DestroyArray("void", "MRP_Array*", "array", [](params) {
	std::vector<double>* vecptr = (std::vector<double>*)arg[0];
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("Script tried passing invalid MRP_Array for destruction");
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

function_entry MRP_GenerateSine("void", "MRP_Array*,double,double", "array,samplerate,frequency", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("MRP_GenerateSine : passed in invalid MRP_Array");
		return (void*)nullptr;
	}
	std::vector<double>& vecref = *(std::vector<double>*)arg[0];
	double sr = bound_value(1.0,*(double*)arg[1],1000000.0);
	double hz = bound_value(0.0001,*(double*)arg[2],sr/2.0);
	int numsamples = vecref.size();
	for (size_t i = 0; i < numsamples; ++i)
		vecref[i] = sin(2*3.141592653/sr*hz*i);
	return (void*)nullptr;
},
"Generate a sine wave into a MRP_Array"
);

function_entry MRP_MultiplyArrays("void", "MRP_Array*,MRP_Array*,MRP_Array*", "array1, array2, array3", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0 || g_active_mrp_arrays.count(arg[1]) == 0
		|| g_active_mrp_arrays.count(arg[2]) == 0)
	{
		ReaScriptError("MRP_MultiplyArrays : passed in invalid MRP_Array(s)");
		return (void*)nullptr;
	}
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	std::vector<double>& vecref1 = *(std::vector<double>*)arg[1];
	std::vector<double>& vecref2 = *(std::vector<double>*)arg[2];
	if ((vecref0.size()==vecref1.size() && vecref1.size()==vecref2.size())==false)
	{
		ReaScriptError("MRP_MultiplyArrays : incompatible array lengths");
		return (void*)nullptr;
	}
	for (size_t i = 0; i < vecref0.size(); ++i)
		vecref2[i] = vecref0[i] * vecref1[i];
	return (void*)nullptr;
},
"Multiply 2 MRP_Arrays of same length. Result is written to 3rd array."
);
#ifdef WIN32
function_entry MRP_MultiplyArraysMT("void", "MRP_Array*,MRP_Array*,MRP_Array*", "array1, array2,array3", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0 || g_active_mrp_arrays.count(arg[1]) == 0
		|| g_active_mrp_arrays.count(arg[2]) == 0)
	{
		ReaScriptError("MRP_MultiplyArraysMT : passed in invalid MRP_Array(s)");
		return (void*)nullptr;
	}
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	std::vector<double>& vecref1 = *(std::vector<double>*)arg[1];
	std::vector<double>& vecref2 = *(std::vector<double>*)arg[2];
	if ((vecref0.size() == vecref1.size() && vecref1.size() == vecref2.size()) == false)
	{
		ReaScriptError("MRP_MultiplyArraysMT : incompatible array lengths");
		return (void*)nullptr;
	}
	Concurrency::parallel_for((size_t)0, vecref0.size(), (size_t)1, [&vecref0, &vecref1, &vecref2](size_t index) 
	{
		vecref2[index] = vecref0[index] * vecref1[index];
	});
	return (void*)nullptr;
},
"Multiply 2 MRP_Arrays of same length. Result is written to 3rd array. Uses multiple threads."
);
#else

#endif
function_entry MRP_SetArrayValue("void", "MRP_Array*,int,double", "array, index, value", [](params) {
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	int index = (in)arg[1];
	double v = *(double*)arg[2];
	vecref0[index] = v;
	return (void*)nullptr;
},
"Set MRP_Array element value. No safety checks done for array or index validity, so use at your own peril!"
);

function_entry MRP_GetArrayValue("double", "MRP_Array*,int", "array, index", [](params) {
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	int index = (in)arg[1];
	double value = vecref0[index];
	return_double(value);
},
"Get MRP_Array element value. No safety checks done for array or index validity, so use at your own peril!"
);

function_entry MRP_WriteArrayToFile("void", "MRP_Array*,const char*,double", "array,filename,samplerate", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("MRP_WriteArrayToFile : passed in invalid MRP_Array");
		return (void*)nullptr;
	}
	std::vector<double>& vecref = *(std::vector<double>*)arg[0];
	const char* outfn = (const char*)arg[1];
	double sr = bound_value(1.0, *(double*)arg[2], 1000000.0);
	char cfg[] = { 'e','v','a','w', 32, 0 };
	PCM_sink* sink = PCM_Sink_Create(outfn, cfg, sizeof(cfg), 1, sr, false);
	if (sink != nullptr)
	{
		double* sinkbuf[1];
		sinkbuf[0] = vecref.data();
		sink->WriteDoubles(sinkbuf, vecref.size(), 1, 0, 1);
		delete sink;
	} else
		ReaScriptError("MRP_WriteArrayToFile : could not create output file");
	return (void*)nullptr;
},
"Write MRP_Array to disk as a 32 bit floating point mono wav file"
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

function_entry MRP_CreateWindow("MRP_Window*", "const char*", "title", [](params)
{
	const char* wtitle = (const char*)arg[0];
	ReaScriptWindow* w = new ReaScriptWindow(wtitle);
	//w->setDestroyOnClose(true);
	w->setSize(400, 400);
	return_obj(w);
},
"Create window"
);

function_entry MRP_DestroyWindow("void", "MRP_Window*", "window", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	if (is_valid_reascriptwindow(w) == true)
	{
		delete w;
	}
	else ReaScriptError("Passed in MRP_Window has already been destroyed");
	return_null;
},
"Destroy window"
);

function_entry MRP_WindowIsClosed("bool", "MRP_Window*", "window", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	if (w == nullptr)
		return_int(0);
	if (w->isClosed() == true)
		return_int(1);
	return_int(0);
},
"Returns if the window has been closed and the ReaScript defer loop should likely be exited"
);

function_entry MRP_WindowSetTitle("void", "MRP_Window*,const char*", "window,title", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* wtitle = (const char*)arg[1];
	if (wtitle != nullptr)
		w->setWindowTitle(wtitle);
	return_null;
},
"Set window title"
);

function_entry MRP_WindowAddControl("void", "MRP_Window*,const char*,const char*", "window,controltypename,objectname", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* controltypename = (const char*)arg[1];
	const char* objectname = (const char*)arg[2];
	if (w != nullptr && controltypename != nullptr && objectname!=nullptr)
	{
		if (w->addControlFromName(controltypename, objectname)==true)
			return_null;
		char errbuf[256];
		sprintf(errbuf, "Could not create control %s", controltypename);
		ReaScriptError(errbuf);
	}
	return_null;
},
"Add a control to window. Controltypename is the type of control to create. Objectname must be a unique id"
);

function_entry MRP_SetControlBounds("void", "MRP_Window*,const char*,double,double,double,double", "window,name,x,y,w,h", [](params)
{
	ReaScriptWindow* wptr = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	double x = (in)arg[2];
	double y = (in)arg[3];
	double w = (in)arg[4];
	double h = (in)arg[5];
	if (wptr != nullptr && cname != nullptr)
	{
		wptr->setControlBounds(cname, x, y, w, h);
	}
	return_null;
},
"Set MRP control position and size"
);

function_entry MRP_WindowIsDirtyControl("bool", "MRP_Window*,const char*", "window,controlname", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (in)arg[1];
	if (w != nullptr)
	{
		bool isdirty = w->isControlDirty(cname);
		if (isdirty == true)
			return_int(1);
	}
	return_int(0);
},
"Returns true if control was manipulated"
);

function_entry MRP_WindowClearDirtyControls("void", "MRP_Window*", "window", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	if (w != nullptr)
	{
		w->clearDirtyControls();
	}
	return_null;
},
"Clears the dirty states of the controls in a window."
);

function_entry MRP_GetControlFloatNumber("double", "MRP_Window*,const char*,int", "window,controlname,which", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	int which = (in)arg[2];
	if (w != nullptr && cname != nullptr)
	{
		return_double(w->getControlValueDouble(cname, which));
	}
	return_double(0.0);
},
"Get a floating point number associated with control. Meaning of 'which' depends on the control targeted."
);

function_entry MRP_SetControlFloatNumber("void", "MRP_Window*,const char*,int,double", "window,controlname,which,value", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	int which = (in)arg[2];
	double val = (in)arg[3];
	if (w != nullptr && cname != nullptr)
	{
		w->setControlValueDouble(cname, which, val);
		return_null;
	}
	return_null;
},
"Set a floating point number associated with control. Meaning of 'which' depends on the control targeted."
);


function_entry MRP_GetControlIntNumber("int", "MRP_Window*,const char*,int", "window,controlname,which", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	int which = (in)arg[2];
	if (w != nullptr && cname != nullptr)
	{
		return_int(w->getControlValueInt(cname, which));
	}
	return_int(0.0);
},
"Get an integer point number associated with control. Meaning of 'which' depends on the control targeted."
);

function_entry MRP_SetControlIntNumber("void", "MRP_Window*,const char*,int,int", 
	"window,controlname,which,value", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	int which = (in)arg[2];
	int val = (in)arg[3];
	if (w != nullptr && cname != nullptr)
	{
		w->setControlValueInt(cname, which, val);
	}
	return_null;
},
"Set an integer point number associated with control. Meaning of 'which' depends on the control targeted."
);

function_entry MRP_SetControlString("void", "MRP_Window*,const char*,int,const char*",
	"window,controlname,which,text", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	int which = (in)arg[2];
	const char* newtext = (const char*)arg[3];
	if (w != nullptr && cname != nullptr)
	{
		w->setControlValueString(cname, which, newtext);
		return_null;
	}
	return_null;
},
"Set a text property associated with control. Meaning of 'which' depends on the control targeted."
);



function_entry MRP_SendCommandString("void", "MRP_Window*,const char*,const char*",
	"window,controlname,commandtext", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	const char* cmdtext = (const char*)arg[2];
	if (w != nullptr && cname != nullptr)
	{
		w->sendCommandString(cname, cmdtext);
		return_null;
	}
	return_null;
},
"Send a command message to control. Currently only the envelope control understands some messages."
);

function_entry MRP_GetWindowDirty("bool", "MRP_Window*,int", "window,whichdirty", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	int which = in(arg[1]);
	if (w != nullptr)
	{
		if (which == 0 && w->m_was_resized == true)
			return_int(1);
	}
	return_int(0);
},
"Get window dirty state (ie, if something was changed in the window). which : 0 window size"
);

function_entry MRP_SetWindowDirty("void", "MRP_Window*,int,bool", "window,which,state", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	int which = in(arg[1]);
	int state = in(arg[2]);
	bool bstate = false;
	if (state == 1)
		bstate = true;
	if (w != nullptr)
	{
		if (which == 0)
			w->m_was_resized = bstate;
	}
	return_null;
},
"Set window dirty state (ie, if something was changed in the controls)"
);

function_entry MRP_GetWindowPosSizeValue("int", "MRP_Window*,int", "window,which", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	int which = in(arg[1]);
	if (w != nullptr)
	{
		int retval = 0;
		MRP::Rectangle rect = w->getBounds();
		if (which == 0)
			return_int(rect.getX());
		if (which == 1)
			return_int(rect.getY());
		if (which == 2)
			return_int(rect.getWidth());
		if (which == 3)
			return_int(rect.getHeight());
	}
	return_int(0);
},
"Get window geometry values. which : 0 x, 1 y, 2 w, 3 h"
);

#ifdef REASCRIPTGUIWORKS

function_entry MRP_GetControlText("const char*", "MRP_Window*,const char*", "window,controlname", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (const char*)arg[1];
	if (w != nullptr && cname != nullptr)
	{
		return_obj(w->getControlText(cname));
	}
	return_null;
},
"Get main text associated with control"
);

function_entry MRP_SetControlText("void", "MRP_Window*,const char*,const char*", "window,controlname,text", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* cname = (in)arg[1];
	const char* txt = (in)arg[2];
	if (w != nullptr && cname != nullptr && txt!=nullptr)
	{
		w->setControlText(cname, txt);
	}
	return_null;
},
"Set main text associated with control"
);


function_entry MRP_WindowAddSlider("void", "MRP_Window*,const char*,int", "window,name,initialvalue", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* slidname = (const char*)arg[1];
	int initval = (in)arg[2];
	if (w != nullptr && slidname!=nullptr)
	{
		w->add_slider(slidname, initval);
		return_null;
	}
	return_null;
},
"Add a Reaper slider control to window"
);


function_entry MRP_WindowAddButton("void", "MRP_Window*,const char*,const char*", "window,name,text", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* butname = (const char*)arg[1];
	const char* buttext = (const char*)arg[2];
	if (w != nullptr && butname != nullptr && buttext!=nullptr)
	{
		w->add_button(butname, buttext);
		return_null;
	}
	return_null;
},
"Add a button to window"
);

function_entry MRP_WindowAddLineEdit("void", "MRP_Window*,const char*,const char*", "window,name,text", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* butname = (const char*)arg[1];
	const char* buttext = (const char*)arg[2];
	if (w != nullptr && butname != nullptr && buttext != nullptr)
	{
		w->add_line_edit(butname, buttext);
		return_null;
	}
	return_null;
},
"Add a (single line) text edit to window"
);

function_entry MRP_WindowAddLabel("void", "MRP_Window*,const char*,const char*", "window,name,text", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* butname = (const char*)arg[1];
	const char* buttext = (const char*)arg[2];
	if (w != nullptr && butname != nullptr && buttext != nullptr)
	{
		w->add_label(butname, buttext);
		return_null;
	}
	return_null;
},
"Add a label to window"
);

function_entry MRP_WindowAddLiceControl("void", "MRP_Window*,const char*,const char*", "window,classname,name", [](params)
{
	ReaScriptWindow* w = (ReaScriptWindow*)arg[0];
	const char* classname = (const char*)arg[1];
	const char* controlname = (const char*)arg[2];
	if (w != nullptr)
	{
		w->add_custom_control(controlname,classname);
		return_null;
	}
	return_null;
},
								  "Add a custom (LiceControl) control to window"
								  );






#endif

function_entry MRP_ReturnMediaItem("MediaItem*", "", "", [](params) {
	return_obj(GetSelectedMediaItem(0, 0));
},
"return media item"
);

function_entry MRP_DoNothing("void", "", "", [](params) {
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