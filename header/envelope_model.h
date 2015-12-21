#pragma once

#include <vector>
#include <algorithm>
#include <string>

class envbreakpoint
{
public:
	envbreakpoint() {}
	envbreakpoint(double x, double y) : m_x(x), m_y(y) {}
	double get_x() const noexcept { return m_x; }
	double get_y() const noexcept { return m_y; }
	void set_x(double x) noexcept { m_x = x; }
	void set_y(double y) noexcept { m_y = y; }
	int get_status() const { return m_status; }
	void set_status(int x) { m_status = x; }
private:
	double m_x = 0.0;
	double m_y = 0.0;
	double m_p1 = 0.0;
	double m_p2 = 0.0;
	int m_status = 0;
};

class breakpoint_envelope
{
public:
	breakpoint_envelope() {}
	breakpoint_envelope(std::string name, int color = 0) : m_name(name), m_color(color) {}
	int get_num_points() const noexcept { return (int)m_points.size(); }
	const envbreakpoint& get_point(int index) const noexcept { return m_points[index]; }
	envbreakpoint& get_point(int index) noexcept { return m_points[index]; }
	void add_point(envbreakpoint pt, bool dosortnow)
	{
		m_points.push_back(pt);
		if (dosortnow == true)
			sort_points();
	}
	void remove_point(int index)
	{
		if (index >= 0 && index < m_points.size())
		{
			m_points.erase(m_points.begin() + index);
		}
	}
	template<typename F>
	inline void remove_points_conditionally(F&& f)
	{
		m_points.erase(std::remove_if(std::begin(m_points), std::end(m_points), f), std::end(m_points));
	}
	void sort_points()
	{
		auto comparator = [](const envbreakpoint& a, const envbreakpoint& b) { return a.get_x() < b.get_x(); };
		std::stable_sort(m_points.begin(), m_points.end(), comparator);
	}
	double interpolate(double t) const noexcept
	{
		if (m_points.empty() == true)
			return 0.0;
		if (t <= m_points.front().get_x())
			return m_points.front().get_y();
		if (t >= m_points.back().get_x())
			return m_points.back().get_y();
		envbreakpoint pt(t, 0.0);
		auto it = std::lower_bound(m_points.begin(), m_points.end(), pt, [](const envbreakpoint& a, const envbreakpoint& b)
		{
			return a.get_x() < b.get_x();
		});
		--it;
		
		double x0 = it->get_x();
		double y0 = it->get_y();
		double x1;
		double y1;
		
		++it;
		if (it != m_points.end())
		{
			x1 = it->get_x();
			y1 = it->get_y();
		}
		else
		{
			x1 = x0;
			y1 = y0;
		}
		double valdiff = y1 - y0;
		double timediff = x1 - x0;
		if (timediff < 0.0001)
			timediff = 0.0001;
		double offset_x = t-x0;
		return y0+valdiff*(1.0/timediff)*offset_x;
	}
	auto begin() { return m_points.begin(); }
	auto end() { return m_points.end(); }
	void setName(std::string name) { m_name = name; }
	std::string getName() const { return m_name; }
	void setColor(int c) { m_color = c; }
	int getColor() const { return m_color; }
private:
	std::vector<envbreakpoint> m_points;
	std::string m_name;
	int m_color = 0;
};

//using breakpoint_envelope = basic_breakpoint_envelope<simple_aux_data>;
