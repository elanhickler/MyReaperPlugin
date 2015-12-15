#ifndef utilfuncs_h
#define utilfuncs_h

#include <algorithm>
#include <ostream>

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

inline bool is_alphaspacenumeric(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c < 'Z') || c == ' ';
}

class readbgbuf : public std::streambuf
{
public:
	int overflow(int c);
	std::streamsize xsputn(const char *buffer, std::streamsize n);
};

class readbg : public std::ostream
{
	readbgbuf buf;
public:
	readbg() :std::ostream(&buf) { }
};

class NoCopyNoMove
{
public:
	NoCopyNoMove(){}
	NoCopyNoMove(const NoCopyNoMove&) = delete;
	NoCopyNoMove& operator=(const NoCopyNoMove&) = delete;
	NoCopyNoMove(NoCopyNoMove&&) = delete;
	NoCopyNoMove& operator=(NoCopyNoMove&&) = delete;
};
#endif /* utilfuncs_h */
