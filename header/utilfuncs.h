#ifndef utilfuncs_h
#define utilfuncs_h

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

#endif /* utilfuncs_h */
