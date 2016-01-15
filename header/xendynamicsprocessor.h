#pragma once

#include "lice_control.h"
#include "mrpwindows.h"
#include <vector>
#include <memory>
#include "mrp_audioaccessor.h"

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

template<typename AudioView>
inline volume_analysis_data analyze_audio_volume(int windowsize,
	AudioView av)
{
	int viewnch = av.numberOfChannels();
	volume_analysis_data result;
	result.m_windowsize = windowsize;
	result.m_numch = viewnch;
	int64_t counter = 0;
	double total_max_peak = 0.0;
	int64_t viewframes = av.numberOfFrames();
	while (counter < viewframes)
	{
		double maxsample = 0.0;
		int peakpos = 0;
		for (int i = 0; i < windowsize; ++i)
		{
			for (int j = 0; j < viewnch; ++j)
			{
				int64_t index = counter + j;
				
				if (index >= viewframes)
					break;
				double abs_sample = fabs(av.getSample(j,index));
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

using namespace mrp::experimental;

class VolumeAnalysisControl : public LiceControl
{
public:
	VolumeAnalysisControl(MRPWindow* parent);
	void setAnalysisData(volume_analysis_data data);
	void setAudioView(audiobuffer_view<double> v)
	{
		m_audio_view_painter = AudioViewPainter<audiobuffer_view<double>>(v);
		repaint();
	}
	void paint(PaintEvent& ev) override;
	volume_analysis_data* getAnalysisData() { return &m_data; }
	void setShowAnalysisCurve(bool b) { m_show_analysis_curve = b; repaint(); }
	bool getShowAnalysisCurve() const { return m_show_analysis_curve; }
	std::string getType() const override { return "VolumeAnalysisControl"; }
private:
	bool m_show_analysis_curve = true;
	volume_analysis_data m_data;
	AudioViewPainter<audiobuffer_view<double>> m_audio_view_painter;
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
	std::shared_ptr<ReaSlider> m_slider1;
	std::vector<double> m_window_sizes;
	void do_dynamics_transform_visualization();
	void render_dynamics_transform();
	void write_transformed_to_file();
	void import_item();
	bool m_envelope_is_db = false;
	void save_state();
	void load_state();
	std::shared_ptr<MRPAudioAccessor> m_acc;
	std::vector<double> m_transformed_audio;
};

void show_dynamics_processor_window(HWND parent);