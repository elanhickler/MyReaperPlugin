#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

int readbgbuf::overflow(int c)
{
	if (c != traits_type::eof()) {
		char ch[2] = { traits_type::to_char_type(c), 0 };
		ShowConsoleMsg(ch);
	}
	return c;
}

std::streamsize readbgbuf::xsputn(const char *buffer, std::streamsize n)
{
	std::string buf(buffer, buffer + n);
	ShowConsoleMsg(buf.c_str());
	return n;
}
