#pragma once

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <memory>

class MRP_AudioDSP
{
public:
	virtual ~MRP_AudioDSP() {}
	virtual bool is_prepared() { return false; }
	// called before calls to process_audio begin
	virtual void prepare_audio(int numchans, double sr, int expected_max_bufsize) {}
	// called when new audio is needed
	virtual void process_audio(double* buf, int nframes) = 0;
	// called when audio is stopped and more call calls to process_audio
	virtual void release_audio() {}
};

class MyTestAudioDSP : public MRP_AudioDSP
{
public:
	~MyTestAudioDSP()
	{
		//OutputDebugString("MyTestAudioDSP dtor");
	}
	double m_osc_phase = 0.0;
	bool m_is_prepared = false;
	int m_nch = 0;
	double m_sr = 0;
	bool is_prepared() { return m_is_prepared; }
	void prepare_audio(int numchans, double sr, int expected)
	{
		m_osc_phase = 0;
		m_nch = numchans;
		m_sr = sr;
		m_is_prepared = true;
	}
	void process_audio(double* buf, int nframes)
	{
		for (int i = 0; i < nframes; ++i)
		{
			double sample = sin(2 * 3.141592653 / 44100.0*440.0*m_osc_phase);
			for (int j = 0; j < m_nch; ++j)
				buf[i*m_nch + j] = sample*0.2;
			m_osc_phase += 1.0;
		}
	}
	void release_audio()
	{
		m_is_prepared = false;
	}
};

class MRP_PCMSource : public PCM_source
{
public:
	MRP_PCMSource(std::shared_ptr<MRP_AudioDSP> dsp) : m_dsp(dsp) {}
	~MRP_PCMSource()
	{
		//OutputDebugString("MRP_PCMSource dtor");
	}
	// Inherited via PCM_source
	PCM_source * Duplicate() override;

	bool IsAvailable() override;

	const char * GetType() override;

	bool SetFileName(const char * newfn) override;

	int GetNumChannels() override;

	double GetSampleRate() override;

	double GetLength() override;

	int PropertiesWindow(HWND hwndParent) override;

	void GetSamples(PCM_source_transfer_t * block) override;

	void GetPeakInfo(PCM_source_peaktransfer_t * block) override;

	void SaveState(ProjectStateContext * ctx) override;

	int LoadState(const char * firstline, ProjectStateContext * ctx) override;

	void Peaks_Clear(bool deleteFile) override;

	int PeaksBuild_Begin() override;

	int PeaksBuild_Run() override;

	void PeaksBuild_Finish() override;

	int Extended(int call, void *parm1, void *parm2, void *parm3) override;
private:
	std::shared_ptr<MRP_AudioDSP> m_dsp;
};

void test_pcm_source(int op);