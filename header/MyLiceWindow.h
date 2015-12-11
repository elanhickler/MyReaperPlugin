#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"

class LiceControl
{
public:
	virtual ~LiceControl() {}
	virtual void paint(LICE_IBitmap* bitmap) = 0;
};

class TestControl : public LiceControl
{
public:
	void paint(LICE_IBitmap* bm);
};

class LiceWindow
{
public:

};
