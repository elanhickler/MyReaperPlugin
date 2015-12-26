#pragma once

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include "envelope_model.h"
#include <memory>
#include <mutex>
#include <cmath>
/*
Reaper provides a way for extension plugins to play sound independent of Reaper's tracks.
(Example from Reaper itself : The Media Explorer.)
This requires there is a PCM_source object that the preview system can use to
render the sound. The PCM_source base class is quite beastly to implement, so here a convenience
mechanism is provided. Instead of writing your own full PCM_source subclass, you can write a subclass of
MRP_AudioDSP which is a simpler interface to implement. You can then create a MRP_PCMSource and pass your
MRP_AudioDSP instance for it.
*/

class MRP_AudioDSP
{
public:
	virtual ~MRP_AudioDSP() {}
	virtual bool is_prepared() { return false; }
	// Called before calls to process_audio begin
	virtual void prepare_audio(int numchans, double sr, int expected_max_bufsize) {}
	// Called when new audio is needed.
	// The usual realtime audio coding rules apply here : don't do anything too time consuming, try to avoid
	// doing anything that can take a "random" amount of time, like allocating memory or
	// waiting for mutexes that are around complicated unpredictable code.
	virtual void process_audio(double* buf, int nch, double sr, int nframes) = 0;
	// Called when audio is stopped and more call calls to process_audio
	virtual void release_audio() {}
	virtual void seek(double seconds) {}
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
	breakpoint_envelope m_env;
	void seek(double seconds)
	{
		m_osc_phase = 0.0;
		//OutputDebugString("was seeked");
	}
	void prepare_audio(int numchans, double sr, int expected)
	{
		//OutputDebugString("MRP prepare_audio");
		m_env.remove_all_points();
		m_env.add_point({ 0.0,1.0 }, false);
		m_env.add_point({ 2.0,0.0 }, false);
		m_env.sort_points();
		m_osc_phase = 0;
		m_nch = numchans;
		m_sr = sr;
		m_is_prepared = true;
	}
	void process_audio(double* buf, int nch, double sr, int nframes)
	{
		for (int i = 0; i < nframes; ++i)
		{
			double sample = sin(2 * 3.141592653 / sr *440.0*m_osc_phase);
			double gain = m_env.interpolate(m_osc_phase / sr);
			for (int j = 0; j < m_nch; ++j)
				buf[i*m_nch + j] = sample*0.2*gain;
			m_osc_phase += 1.0;
		}
	}
	void release_audio()
	{
		m_is_prepared = false;
		//OutputDebugString("MRP release_audio");
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
	
	void set_dsp(std::shared_ptr<MRP_AudioDSP> dsp)
	{
		// take pointer copy so that if we are currently the only owner, the switch doesn't block
		// while the old object is being destroyed
		auto old = m_dsp;
		m_mutex.lock();
		m_dsp = dsp;
		m_mutex.unlock();
		// if old was the only owner, it is now destroyed at end of scope, outside the mutex lock
	}
	
	std::shared_ptr<MRP_AudioDSP> get_dsp()
	{
		return m_dsp;
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
	// mutex guards changing the dsp object while playing back
	std::mutex m_mutex;
	double m_lastpos = 0.0;
};

void test_pcm_source(int op);