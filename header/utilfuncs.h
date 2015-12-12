#ifndef utilfuncs_h
#define utilfuncs_h

template <typename T>
T bound_value(const T& lower, const T& n, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}

#endif /* utilfuncs_h */
