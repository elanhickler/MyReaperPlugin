#pragma once

#include "lice_control.h"
#include "envelope_model.h"

struct fx_param_t
{
	int tracknum = -1;
	int fxnum = 0;
	int paramnum = 0;
};

struct point
{
	point() {}
	point(int x, int y) : m_x(x), m_y(y) {}
	int m_x = 0;
	int m_y = 0;
	fx_param_t m_x_target;
	fx_param_t m_y_target;
};

// Development test control
class TestControl : public LiceControl
{
public:
	TestControl(HWND parent, bool delwhendraggedoutside = false);
	void paint(LICE_IBitmap* bm) override;
	void mousePressed(const MouseEvent& ev) override;
	void mouseDoubleClicked(const MouseEvent& ev) override;
	void mouseMoved(const MouseEvent& ev) override;
	void mouseReleased(const MouseEvent& ev) override;
	void mouseWheel(int x, int y, int delta) override;
	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;
	std::function<void(int, double, double)> PointMovedCallback;
	fx_param_t* getFXParamTarget(int index, int which);
	std::string getType() const override { return "MultiXYControl"; }
private:
	
	std::vector<point> m_points;
	int find_hot_point(int x, int y);
	int m_hot_point = -1;
	float m_circlesize = 10.0f;
	bool m_mousedown = false;
	bool m_delete_point_when_dragged_outside = false;
	std::string m_test_text;
	void shift_points(double x, double y);
	LICE_CachedFont m_font;
};

class WaveformControl : public LiceControl
{
public:
	WaveformControl(HWND parent);
	void setSource(PCM_source* src);
	void paint(LICE_IBitmap* bm) override;
	void mousePressed(const MouseEvent& ev) override;
	void mouseMoved(const MouseEvent& ev) override;
	void mouseReleased(const MouseEvent& ev) override;
	void mouseDoubleClicked(const MouseEvent& ev) override;
	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;
	std::string getType() const override { return "WaveformControl"; }
	double getFloatingPointProperty(int which) override;
	void setFloatingPointProperty(int which, double v) override;
private:
	std::shared_ptr<PCM_source> m_src;
	std::vector<double> m_minpeaks;
	std::vector<double> m_maxpeaks;
	double m_peaks_gain = 1.0;
	double m_view_start = 0.0;
	double m_view_end = 1.0;
	double m_sel_start = 0.0;
	double m_sel_end = 0.0;
	int m_drag_start_x = 0;
	int m_drag_start_y = 0;
	bool m_mouse_down = false;
	int get_hot_time_sel_edge(int x, int y);
	int m_hot_sel_edge = 0;
	bool m_use_reaper_peaks_drawing = false;
	LICE_CachedFont m_font;
};

class EnvelopeControl : public LiceControl
{
public:
	EnvelopeControl(HWND parent);
	void paint(LICE_IBitmap* bm) override;
	
	void mousePressed(const MouseEvent& ev) override;
	void mouseMoved (const MouseEvent& ev) override;
	void mouseReleased(const MouseEvent& ev) override;

	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;

	std::string getType() const override { return "BreakpointEnvelope"; }
	void set_envelope(std::shared_ptr<breakpoint_envelope> env);
private:
	std::shared_ptr<breakpoint_envelope> m_env;
	LICE_CachedFont m_font;
	bool m_mouse_down = false;
	int m_node_to_drag = -1;
	int find_hot_envelope_point(double xcor, double ycor);
	double m_view_start_time = 0.0;
	double m_view_end_time = 1.0;
	double m_view_start_value = 0.0;
	double m_view_end_value = 1.0;
};

void pitch_bend_selected_item(std::shared_ptr<breakpoint_envelope> env);
