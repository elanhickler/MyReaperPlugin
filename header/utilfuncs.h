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

inline bool is_point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
	return px>=rx && px<rx+rw && py>=ry && py<ry+rh;
}


#endif /* utilfuncs_h */
