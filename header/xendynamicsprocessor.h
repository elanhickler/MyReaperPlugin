#pragma once

#include "lice_control.h"
#include "mrpwindows.h"
#include <vector>
#include <memory>

class volume_analysis_data_point
{
public:
	volume_analysis_data_point() {}
	volume_analysis_data_point(int64_t ts, double v, int mppos) 
		: m_time_stamp(ts), m_abs_peak(v), m_max_peak_pos(mppos) {}
	int64_t m_time_stamp = 0;
	double m_abs_peak = 0.0;
	int m_max_peak_pos = 0;
};

class volume_analysis_data
{
public:
	std::vector<volume_analysis_data_point> m_datapoints;
	int m_numch = 0;
	int64_t m_numframes = 0;
	int m_sr = 0;
	int m_windowsize = 0;
	double m_total_max_peak = 0.0;
};

inline volume_analysis_data analyze_audio_volume(int windowsize,
	int numchannels, double* buffer, int64_t numframes)
{
	volume_analysis_data result;
	result.m_windowsize = windowsize;
	result.m_numch = numchannels;
	int64_t counter = 0;
	double total_max_peak = 0.0;
	while (counter < numframes)
	{
		double maxsample = 0.0;
		int peakpos = 0;
		for (int i = 0; i < windowsize; ++i)
		{
			for (int j = 0; j < numchannels; ++j)
			{
				int64_t index = (counter*numchannels) + j;
				if (index >= numframes*numchannels)
					break;
				double abs_sample = fabs(buffer[index]);
				if (abs_sample > maxsample)
				{
					maxsample = abs_sample;
					peakpos = i;
				}
				total_max_peak = std::max(total_max_peak, abs_sample);
			}
			++counter;
		}
		result.m_datapoints.emplace_back(counter, maxsample, peakpos);
		
	}
	result.m_total_max_peak = total_max_peak;
	return result;
}

class VolumeAnalysisControl : public LiceControl
{
public:
	VolumeAnalysisControl(MRPWindow* parent);
	void setAnalysisData(volume_analysis_data data);
	void paint(PaintEvent& ev) override;
	volume_analysis_data* getAnalysisData() { return &m_data; }
	std::string getType() const override { return "VolumeAnalysisControl"; }
private:
	volume_analysis_data m_data;
};

class DynamicsProcessorWindow : public MRPWindow
{
public:
	DynamicsProcessorWindow(HWND parent);
	void resized() override;
private:
	std::shared_ptr<VolumeAnalysisControl> m_analysiscontrol1;
	std::shared_ptr<VolumeAnalysisControl> m_analysiscontrol2;
	std::shared_ptr<EnvelopeControl> m_envelopecontrol1;
	std::shared_ptr<WinButton> m_importbut;
	std::shared_ptr<WinButton> m_renderbut;
	std::shared_ptr<breakpoint_envelope> m_transformenvelope1;
	std::shared_ptr<WinLabel> m_windowsizelabel1;
	std::shared_ptr<WinComboBox> m_windowsizecombo1;
	std::vector<double> m_window_sizes;
	void do_dynamics_transform_visualization();
	void render_dynamics_transform();
	void import_item();
	bool m_envelope_is_db = false;
	void save_state();
	void load_state();
};

void show_dynamics_processor_window(HWND parent);