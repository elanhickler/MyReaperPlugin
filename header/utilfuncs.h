#pragma once

#include <algorithm>
#include <ostream>
#include <functional>
#include <memory>
#include <cmath>
#include <type_traits>
#include <string>
#include <vector>
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#ifdef WIN32
#include "ppl.h"
#else
#include <dispatch/dispatch.h>
#endif

#undef min
#undef max
template <typename T>
inline T bound_value(T lower, T n, T upper)
{
	return std::max(lower, std::min(n, upper));
}

template<typename T,typename U>
inline T map_value(U valin, U inmin, U inmax, T outmin, T outmax)
{
	static_assert(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, "Only arithmetic types supported");
	return outmin + ((outmax - outmin) * ((T)valin - (T)inmin)) / ((T)inmax - (T)inmin);
}

template<typename F>
inline double map_value_shaped(double valin, double inmin, double inmax, double outmin, double outmax, F&& shapingfunction)
{
	double tempnormalized = map_value(valin, inmin, inmax, 0.0, 1.0);
	tempnormalized = bound_value(0.0,shapingfunction(tempnormalized),1.0);
	return map_value(tempnormalized, 0.0, 1.0, outmin, outmax);
}

inline bool is_point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
	return px>=rx && px<rx+rw && py>=ry && py<ry+rh;
}

template<typename T>
inline bool is_in_range(T val, T start, T end)
{
	return val >= start && val <= end;
}


inline bool is_alphaspacenumeric(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ';
}

template<typename F, typename... Ts>
inline F for_each_arg(F f, Ts&&... a)
{
	return std::initializer_list<int>{(std::ref(f)(std::forward<Ts>(a)), 0)...}, f;
}


template<typename T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	const std::size_t kMul = 0x9ddfea08eb382d69ULL;
	std::size_t a = (hasher(v) ^ seed) * kMul;
	a ^= (a >> 47);
	std::size_t b = (seed ^ a) * kMul;
	b ^= (b >> 47);
	seed = b * kMul;
}

// Get a value from a map style container without inserting an element if the key wasn't present
// and instead return default constructed value of the map value type
template<typename Key, typename Cont>
inline auto get_from_map(Cont& c, const Key& k)
{
	if (c.count(k) > 0)
		return c[k];
	return typename Cont::mapped_type();
}

std::string is_source_audio(PCM_source* src);

struct create_item_result
{
	MediaItem* item = nullptr;
	MediaItem_Take* take = nullptr;
	PCM_source* src = nullptr;
};

create_item_result create_item_with_take_and_source(MediaTrack* track, const char* fn);

class readbg
{
public:
	
	readbg() {}
	~readbg();
	readbg& operator << (const char* x)
	{
		m_buf.append(x);
		return *this;
	}
	readbg& operator << (int x)
	{
		m_buf.append(std::to_string(x));
		return *this;
	}
	readbg& operator << (size_t x)
	{
		m_buf.append(std::to_string(x));
		return *this;
	}
	readbg& operator << (int64_t x)
	{
		m_buf.append(std::to_string(x));
		return *this;
	}
	readbg& operator << (double x)
	{
		char buf[64];
		sprintf(buf,"%.2f",x);
		m_buf.append(buf);
		return *this;
	}
	
	readbg& operator << (void* x)
	{
		char buf[32];
		sprintf(buf,"%p",x);
		m_buf.append(buf);
		return *this;
	}
	
	readbg& operator << (const std::string& x)
	{
		m_buf.append(x);
		return *this;
	}
	std::string m_buf;
};

void set_readbg_decimals(int decims);

void start_or_stop_main_thread_executor(bool stop);
void execute_in_main_thread(std::function<void(void)> f);

class IValueConverter
{
public:
	virtual ~IValueConverter() {}
	virtual double fromNormalizedToValue(double x) = 0;
	virtual double toNormalizedFromValue(double x) = 0;
	virtual std::string toStringFromValue(double x) = 0;
	virtual double fromStringToValue(const std::string& x) = 0;
};

class LinearValueConverter : public IValueConverter
{
public:
	LinearValueConverter(double minval, double maxval) :
		m_min_value(minval), m_max_value(maxval) {}
	double fromNormalizedToValue(double x) override
	{ return map_value(x,0.0,1.0,m_min_value,m_max_value); }
	double toNormalizedFromValue(double x) override
	{ return map_value(x,m_min_value,m_max_value,0.0,1.0); }
	std::string toStringFromValue(double x) override
	{ return std::to_string(x); }
	double fromStringToValue(const std::string& x) override
	{
		return bound_value(m_min_value, atof(x.c_str()), m_max_value);
	}
private:
	double m_min_value = 0.0;
	double m_max_value = 1.0;
};

template<typename T>
class copy_on_write
{
public:
	copy_on_write() { m_x = std::make_shared<T>(); }
	copy_on_write(T x) { m_x = std::make_shared<T>(x); }
	copy_on_write(const copy_on_write& other)
	{
		m_x = other.m_x;
	}
	//copy_on_write(copy_on_write&&) = delete;
	/*
	copy_on_write& operator=(const copy_on_write& other)
	{
		detach_if_needed();
		m_x = other.m_x;
	}
	*/
	copy_on_write& operator=(copy_on_write&& other)
	{
		std::swap(m_x, other.m_x);
		return *this;
	}
	const T& read() const { return *m_x; }
	T& write() 
	{ 
		detach_if_needed();
		return *m_x; 
	}
private:
	std::shared_ptr<T> m_x;
	void detach_if_needed()
	{
		if (m_x.unique() == false)
			m_x = std::make_shared<T>(*m_x);
	}
};

class reaper_track_range
{
public:
	struct track_iterator
	{
		track_iterator(ReaProject* proj, bool last)
			: m_proj(proj), m_last(last) 
		{
			m_proj_num_tracks = CountTracks(m_proj);
			if (m_last == true)
				m_cur_track = m_proj_num_tracks;
		}
		ReaProject* m_proj = nullptr;
		int m_cur_track = 0;
		bool m_last = false;
		int m_proj_num_tracks = 0;
		reaper_track_range* m_range = nullptr;
		track_iterator& operator++()
		{
			++m_cur_track;
			if (m_cur_track >= m_proj_num_tracks)
			{
				m_last = true;
				m_cur_track = m_proj_num_tracks;
			}
			return *this;
		}
		bool operator==(const track_iterator& rhs)
		{
			return m_proj == rhs.m_proj && m_cur_track == rhs.m_cur_track && m_last == rhs.m_last;
		}
		bool operator!=(const track_iterator& rhs)
		{
			return !((*this) == rhs);
		}
		MediaTrack* operator*()
		{
			if (m_cur_track<m_proj_num_tracks)
				return GetTrack(m_proj, m_cur_track);
			return nullptr;
		}
	};
	reaper_track_range(ReaProject* proj = nullptr) : m_proj(proj) 
	{
	}
	track_iterator begin()
	{
		if (CountTracks(m_proj) > 0)
			return track_iterator(m_proj,false);
		return track_iterator(m_proj,true);
	}
	track_iterator end()
	{
		return track_iterator(m_proj,true);
	}
private:
	ReaProject* m_proj = nullptr;
};

inline bool fuzzy_compare(double p1, double p2)
{
	return (fabs(p1 - p2) * 1000000000000. <= std::min(fabs(p1), fabs(p2)));
}

namespace MRP
{
	enum class Anchor
	{
		TopLeft, TopMiddle, TopRight,
		MiddleLeft, MiddleMiddle, MiddleRight,
		BottomLeft, BottomMiddle, BottomRight
	};

	template<typename T>
	class GenericPoint
	{
	public:
		GenericPoint() : m_x(T()), m_y(T()) {}
		GenericPoint(T x, T y) : m_x(x), m_y(y) {}
		T x() const noexcept { return m_x; }
		T y() const noexcept { return m_y; }
		void setX(T x) { m_x = x; }
		void setY(T y) { m_y = y; }
	private:
		T m_x;
		T m_y;
	};

	using Point = GenericPoint<int>;

	template<typename T>
	class GenericSize
	{
	public:
		GenericSize() : m_w(T()), m_h(T()) {}
		GenericSize(T w, T h) : m_w(w), m_h(h) {}
		T getWidth() const noexcept { return m_w; }
		T getHeight() const noexcept { return m_h; }
		void setWidth(T w) { m_w = w; };
		void setHeight(T h) { m_h = h; }
		bool isValid() const noexcept { return m_w > 0 && m_h > 0; }
	private:
		T m_w;
		T m_h;
	};

	using Size = GenericSize<int>;

	template<typename T>
	class GenericRectangle
	{
	public:
		GenericRectangle() : m_x(T()), m_w(T()), m_y(T()), m_h(T()) {}
		GenericRectangle(T x, T y, T w, T h) :
			m_x(x), m_w(w), m_y(y), m_h(h) {}
		T getX() const noexcept { return m_x; }
		T getY() const noexcept { return m_y; }
		T getRight() const noexcept { return m_x + m_w; }
		T getBottom() const noexcept { return m_y + m_h; }
		T getMiddleX() const noexcept { return m_x + m_w / 2; }
		T getMiddleY() const noexcept { return m_y + m_h / 2; }
		T getWidth() const noexcept { return m_w; }
		T getHeight() const noexcept { return m_h; }
		bool operator==(const GenericRectangle<T> rhs)
		{
			return m_x == rhs.m_x && m_y == rhs.m_y &&
				m_w == rhs.m_w && m_h == rhs.m_h;
		}
		bool operator!=(const GenericRectangle<T> rhs)
		{
			return !((*this) == rhs);
		}
		GenericPoint<T> getTopLeft() const noexcept
		{
			return GenericPoint<T>(m_x, m_y);
		}

		void setTopLeft(GenericPoint<T> pt)
		{
			m_x = pt.x();
			m_y = pt.y();
		}

		GenericPoint<T> getTopRight() const noexcept
		{
			return GenericPoint<T>(m_x+m_w, m_y);
		}

		GenericPoint<T> getBottomLeft() const noexcept
		{
			return GenericPoint<T>(m_x, m_y+m_h);
		}

		GenericPoint<T> getBottomRight() const noexcept
		{
			return GenericPoint<T>(m_x+m_w, m_y + m_h);
		}

		GenericPoint<T> getCenter() const noexcept
		{
			return GenericPoint<T>(m_x + (m_w/2), m_y + (m_h/2));
		}

		void setX(T x) { m_x = x; }
		void setY(T y) { m_y = y;  }
		void setWidth(T w) { m_w = w;  }
		void setHeight(T h) { m_h = h;  }
		GenericRectangle<T> resized(T w, T h)
		{
			return GenericRectangle<T>(m_x, m_y, w, h);
		}
		GenericRectangle<T> moved(T x, T y)
		{
			return GenericRectangle<T>(x, y, m_w, m_h);
		}
		GenericRectangle centeredTo(T x, T y)
		{
			return GenericRectangle<T>(x - getWidth() / 2, y - getHeight() / 2, m_w, m_h);
		}
		GenericRectangle<T> leftShifted(T dx)
		{
			return GenericRectangle<T>(m_x + dx, m_y, m_w - dx, m_h);
		}
		GenericRectangle<T> rightShifted(T dx)
		{
			return GenericRectangle<T>(m_x, m_y, m_w + dx, m_h);
		}
		GenericRectangle<T> withHorizontalMargins(T margin)
		{
			return GenericRectangle<T>(m_x + margin, m_y, m_w - 2 * margin, m_h);
		}
		static GenericRectangle<T> anchoredToBottomOf(const GenericRectangle<T>& g,
			T x, T w, T h, T offset_from_bottom)
		{
			return GenericRectangle<T>(x,
				g.getBottom() - h - offset_from_bottom,
				w,
				h);
		}
		static GenericRectangle<T> anchoredTo(const GenericRectangle<T>& g, Anchor anchor, T w, T h)
		{
			if (anchor == Anchor::BottomLeft)
				return GenericRectangle<T>(g.m_x, g.getBottom() - h, w, h);
			if (anchor == Anchor::BottomRight)
				return GenericRectangle<T>(g.m_x + g.m_w - w, g.getBottom() - h, w, h);
			if (anchor == Anchor::BottomMiddle)
				return GenericRectangle<T>(g.m_x, g.getBottom() - h, g.getWidth(), h);
			return GenericRectangle<T>();
		}
		static GenericRectangle<T> fromGridPositions(const GenericRectangle<T>& g,
			int griddivs, int x0, int y0, int x1, int y1)
		{
			T nx0 = (double)g.getWidth() / griddivs * x0;
			T ny0 = (double)g.getHeight() / griddivs * y0;
			T nx1 = (double)g.getWidth() / griddivs * x1;
			T ny1 = (double)g.getHeight() / griddivs * y1;
			T nw = nx1 - nx0;
			T nh = ny1 - ny0;
			return GenericRectangle<T>(nx0, ny0, nw, nh);
		}
		bool isValid() const noexcept 
		{ 
			return getX()<getRight() && getY()<getBottom(); 
		}
	private:
		T m_x;
		T m_w;
		T m_y;
		T m_h;
	};

	using Rectangle = GenericRectangle<int>;
}
class NoCopyNoMove
{
public:
	NoCopyNoMove(){}
	NoCopyNoMove(const NoCopyNoMove&) = delete;
	NoCopyNoMove& operator=(const NoCopyNoMove&) = delete;
	NoCopyNoMove(NoCopyNoMove&&) = delete;
	NoCopyNoMove& operator=(NoCopyNoMove&&) = delete;
};

class IParallelTask : public NoCopyNoMove
{
public:
	virtual ~IParallelTask() {}
	virtual void run() = 0;
};

inline void execute_parallel_tasks(std::vector<std::shared_ptr<IParallelTask>> tasks, bool multithreaded = true)
{
	// No use in firing up the concurrency stuff if only one task
	if (tasks.size() < 2)
		multithreaded = false;
	if (multithreaded == true)
	{
#ifdef WIN32
		Concurrency::parallel_for_each(tasks.begin(), tasks.end(), [](auto t) { t->run(); });
#else
		dispatch_queue_t the_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
		void* ctx=(void*)&tasks;
		dispatch_apply_f(tasks.size(),the_queue,ctx,[](void* thectx,size_t index)
		{
			auto ptrtasks=(std::vector<std::shared_ptr<IParallelTask>>*)thectx;
			(*ptrtasks)[index]->run();
		});
#endif
	}
	else
	{
		for (auto& e : tasks)
			e->run();
	}
}

