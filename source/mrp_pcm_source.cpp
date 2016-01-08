#include "mrp_pcm_source.h"
#include "utilfuncs.h"

PCM_source * MRP_PCMSource::Duplicate()
{
	return nullptr;
}

bool MRP_PCMSource::IsAvailable()
{
	return m_dsp!=nullptr;
}

const char * MRP_PCMSource::GetType()
{
	return "MRP_PCMSOURCE";
}

bool MRP_PCMSource::SetFileName(const char * newfn)
{
	return false;
}

int MRP_PCMSource::GetNumChannels()
{
	return 2;
}

double MRP_PCMSource::GetSampleRate()
{
	return 44100.0;
}

double MRP_PCMSource::GetLength()
{
	// Have to return some length in seconds here
	return 3.0;
}

int MRP_PCMSource::PropertiesWindow(HWND hwndParent)
{
	return 0;
}

void MRP_PCMSource::GetSamples(PCM_source_transfer_t * block)
{
	std::lock_guard<std::mutex> locker(m_mutex);
	// wasteful prezeroing if no problem, but meh for now...
	for (int i = 0; i < block->length*block->nch; ++i)
		block->samples[i] = 0.0;
	
	if (m_dsp != nullptr)
	{
		if (m_dsp->is_prepared() == true)
		{
			// Oddly enough, PCM_source doesn't have a Seek method that we could react to.
			// So a hack like this is needed to detect a seek happened...
			// This may not be completely correct but the fuzzy comparison should make it pretty close
			double t0 = m_lastpos + ((double)block->length / block->samplerate);
			if (fuzzy_compare(t0, block->time_s) == false)
			{
				// was probably seeked
				//char buf[256];
				//sprintf(buf, "pcm_source seeked %f %f %f", m_lastpos, t0, block->time_s);
				//OutputDebugString(buf);
				m_dsp->seek(block->time_s);
			}
			m_dsp->process_audio(block->samples, block->nch, block->samplerate, block->length);
		}
	}
	block->samples_out = block->length;
	m_lastpos = block->time_s;
}

void MRP_PCMSource::GetPeakInfo(PCM_source_peaktransfer_t * block)
{
}

void MRP_PCMSource::SaveState(ProjectStateContext * ctx)
{
}

int MRP_PCMSource::LoadState(const char * firstline, ProjectStateContext * ctx)
{
	return 0;
}

void MRP_PCMSource::Peaks_Clear(bool deleteFile)
{
}

int MRP_PCMSource::PeaksBuild_Begin()
{
	return 0;
}

int MRP_PCMSource::PeaksBuild_Run()
{
	return 0;
}

void MRP_PCMSource::PeaksBuild_Finish()
{
}

int MRP_PCMSource::Extended(int call, void *parm1, void *parm2, void *parm3)
{
	// not called with the Preview system
	if (call == PCM_SOURCE_EXT_ENDPLAYNOTIFY)
	{
		m_dsp->release_audio();
		return 1;
	}
	return 0;
}

std::shared_ptr<MRP_PCMSource> g_test_source;
bool g_is_playing = false;
preview_register_t g_prev_reg = { 0 };

void test_pcm_source(int op)
{
	if (op == 0)
	{
		if (g_test_source == nullptr)
		{
			auto mydsp = std::make_shared<MyTestAudioDSP>();
			g_test_source = std::make_shared<MRP_PCMSource>(mydsp);
			g_prev_reg.src = g_test_source.get();
			g_prev_reg.volume = 1.0;
			g_prev_reg.loop = true;
#ifdef WIN32
			InitializeCriticalSection(&g_prev_reg.cs);
#else
			// I wonder if pthread copies this stuff internally...
			pthread_mutexattr_t mta;
			pthread_mutexattr_init(&mta);
			pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&g_prev_reg.mutex, &mta);
#endif
		}
		if (g_is_playing == false)
		{
			g_test_source->get_dsp()->prepare_audio(2, 44100.0, 512);
			PlayPreview(&g_prev_reg);
			g_is_playing = true;
		}
		else
		{
			StopPreview(&g_prev_reg);
			g_test_source->get_dsp()->release_audio();
			g_is_playing = false;
		}
	}
	else if (op == 1) // Clean up on Reaper shutdown
	{
		if (g_is_playing == true)
			StopPreview(&g_prev_reg);
		g_test_source.reset();
#ifdef WIN32
		DeleteCriticalSection(&g_prev_reg.cs);
#else
		pthread_mutex_destroy(&g_prev_reg.mutex);
#endif
	}
}
