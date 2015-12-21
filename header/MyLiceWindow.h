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

class WaveformPainter
{
public:
	WaveformPainter(int initialnumpeaks = 500)
	{
		m_minpeaks.resize(initialnumpeaks);
		m_maxpeaks.resize(initialnumpeaks);
	}
	bool paint(LICE_IBitmap* bm, double starttime, double endtime, int x, int y, int w, int h)
	{
		if (m_src == nullptr)
			return false;
		if (m_src->GetNumChannels() < 1)
			return false;
		int nch = m_src->GetNumChannels();
		if (m_minpeaks.size() < w * nch)
		{
			m_minpeaks.resize(w*nch);
			m_maxpeaks.resize(w*nch);
		}
		PCM_source_peaktransfer_t peaktrans = { 0 };
		peaktrans.nchpeaks = m_src->GetNumChannels();
		peaktrans.samplerate = m_src->GetSampleRate();
		peaktrans.start_time = starttime;
		peaktrans.peaks = m_maxpeaks.data();
		peaktrans.peaks_minvals = m_minpeaks.data();
		peaktrans.peaks_minvals_used = 1;
		peaktrans.numpeak_points = bm->getWidth();
		peaktrans.peakrate = (double)bm->getWidth() / (endtime - starttime);
		m_src->GetPeakInfo(&peaktrans);
		GetPeaksBitmap(&peaktrans, 1.0, w, h, bm);
	}
	void set_source(PCM_source* src)
	{
		if (src == nullptr)
		{
			m_src = nullptr;
			return;
		}
		m_src = std::shared_ptr<PCM_source>(src->Duplicate());
		if (m_src != nullptr)
		{
			if (m_src->PeaksBuild_Begin() != 0) // should build peaks
			{
				while (true)
				{
					if (m_src->PeaksBuild_Run() == 0)
						break;
				}
				m_src->PeaksBuild_Finish();
			}
		}
	}
	PCM_source* getSource()
	{
		return m_src.get();
	}
private:
	std::shared_ptr<PCM_source> m_src;
	std::vector<double> m_minpeaks;
	std::vector<double> m_maxpeaks;
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

	std::string getType() const override { return "BreakpointEnvelope"; }
	void set_envelope(std::shared_ptr<breakpoint_envelope> env);
	void set_waveformpainter(std::shared_ptr<WaveformPainter> painter);
protected:
	std::shared_ptr<breakpoint_envelope> m_env;
	std::shared_ptr<WaveformPainter> m_wave_painter;
	LICE_CachedFont m_font;
	bool m_mouse_down = false;
	int m_node_to_drag = -1;
	int find_hot_envelope_point(double xcor, double ycor);
	double m_view_start_time = 0.0;
	double m_view_end_time = 1.0;
	double m_view_start_value = 0.0;
	double m_view_end_value = 1.0;
	std::string m_text;
};

class PitchBenderEnvelopeControl : public EnvelopeControl
{
public:
	PitchBenderEnvelopeControl(HWND parent) : EnvelopeControl(parent)
	{

	}
	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;
	std::string getType() const override { return "PitchBenderEnvelopeControl"; }
};

std::string pitch_bend_selected_item(std::shared_ptr<breakpoint_envelope> env);
