#ifndef utilfuncs_h
#define utilfuncs_h

#include <algorithm>
#include <ostream>
#include "../library/reaper_plugin/reaper_plugin_functions.h"

template <typename T>
inline T bound_value(T lower, T n, T upper)
{
	return std::max(lower, std::min(n, upper));
}

template<typename T>
inline T map_value(T valin, T inmin, T inmax, T outmin, T outmax)
{
	return outmin + ((outmax - outmin) * (valin - inmin)) / (inmax - inmin);
}

inline bool is_point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
	return px>=rx && px<rx+rw && py>=ry && py<ry+rh;
}

class readbgbuf : public std::streambuf
{
public:
	int overflow(int c) {
		if (c != traits_type::eof()) {
			char ch[2] = { traits_type::to_char_type(c), 0 };
			ShowConsoleMsg(ch);
		}
		return c;
	}

	std::streamsize xsputn(const char *buffer, std::streamsize n) {
		std::string buf(buffer, buffer + n);
		ShowConsoleMsg(buf.c_str());
		return n;
	}
};

class readbg : public std::ostream
{
	readbgbuf buf;
public:
	readbg() :std::ostream(&buf) { }
};

#endif /* utilfuncs_h */