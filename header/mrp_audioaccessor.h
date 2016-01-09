#pragma once

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include "utilfuncs.h"
#include <memory>
#include <vector>

template<typename T>
class pointer_range
{
public:
	pointer_range() {}
	pointer_range(T* data, int64_t datalen, int datachans,
		double datasr, int64_t start, int64_t rangelen) : m_data(data)
	{
		m_datalen = datalen;
		m_start = start;
		m_len = rangelen;
		m_nch = datachans;
		m_sr = datasr;
	}
	const T& getSample(int chan, int64_t index) const noexcept
	{
		return m_data[(m_start + index)*m_nch + chan];
	}
	const T& getSampleSafe(int chan, int64_t index) const noexcept
	{
		int64_t temp = (m_start + index)*m_nch + chan;
		if (temp >= m_start && temp < m_start + m_len)
			return m_data[temp];
		return m_dummysample;
	}
	int numberOfChannels() const noexcept { return m_nch; }
	double sampleRate() const noexcept { return m_sr;  }
	int64_t size() const noexcept { return m_len; }
	int64_t numberOfFrames() const noexcept { return m_len; }
	T* getData() { return m_data; }
private:
	T* m_data = nullptr;
	int m_nch = 0;
	int64_t m_start = 0;
	int64_t m_len = 0;
	int64_t m_datalen = 0;
	double m_sr = 0.0;
	double m_dummysample;
};

class MRPAudioAccessor
{
public:
	enum SourceType
	{
		ST_None,
		ST_Take,
		ST_Track,
		ST_PCMSource
	};
	MRPAudioAccessor() {}
	MRPAudioAccessor(MediaItem* item, int takeindex = -1)
	{
		MediaItem_Take* take = nullptr;
		if (takeindex == -1)
			take = GetActiveTake(item);
		else take = GetTake(item, takeindex);
		init_from_take(take);
	}
	MRPAudioAccessor(MediaItem_Take* take)
	{
		init_from_take(take);
	}
	MRPAudioAccessor(MediaTrack* track)
	{
		init_from_track(track);
	}
	MRPAudioAccessor(PCM_source* source, bool clonesource)
	{
		init_from_pcm_source(source,clonesource);
	}
	// No copies...
	MRPAudioAccessor(const MRPAudioAccessor&) = delete;
	MRPAudioAccessor& operator = (const MRPAudioAccessor&) = delete;
	// But moves should be fine
	MRPAudioAccessor(MRPAudioAccessor&& other) = delete;
	/*
	{
		std::swap(other.m_sourcetype, m_sourcetype);
		std::swap(other.m_valid, m_valid);
		std::swap(other.m_audio_buffer, m_audio_buffer);
		std::swap(other.m_audio_accessor, m_audio_accessor);
	}
	*/
	MRPAudioAccessor& operator = (MRPAudioAccessor&&) = delete;
	~MRPAudioAccessor()
	{
		if (m_audio_accessor != nullptr)
			DestroyAudioAccessor(m_audio_accessor);
		delete m_source;
	}
	bool isValid() const noexcept { return m_valid; }
	bool isLoaded() const noexcept { return m_audio_loaded; }
	int numberOfChannels() const noexcept { return m_orig_nch; }
	double sampleRate() const noexcept { return m_orig_sr; }
	int64_t numberOfFrames() const noexcept { return m_num_frames_avail; }
	std::string errorString() const { return m_error_string; }
	void loadAudioToMemory()
	{
		if (m_valid == false)
			return;
		if ((m_sourcetype == ST_Take || m_sourcetype == ST_Track) && m_audio_accessor != nullptr)
		{
			double t0 = GetAudioAccessorStartTime(m_audio_accessor);
			double t1 = GetAudioAccessorEndTime(m_audio_accessor);
			double len = t1 - t0;
			int64_t lenframes = m_orig_sr * len;
			m_audio_buffer.resize(lenframes*m_orig_nch);
			if (GetAudioAccessorSamples(m_audio_accessor,
				m_orig_sr, m_orig_nch, 0.0, lenframes, m_audio_buffer.data()) == 1)
			{
				m_num_frames_avail = lenframes;
				m_audio_loaded = true;
			}
		} 
		else if (m_sourcetype == ST_PCMSource && m_source != nullptr)
		{
			int64_t lenframes = m_source->GetSampleRate() * m_source->GetLength();
			m_audio_buffer.resize(lenframes*m_orig_nch);
			PCM_source_transfer_t transfer = { 0 };
			transfer.length = lenframes;
			transfer.nch = m_source->GetNumChannels();
			transfer.samplerate = m_source->GetSampleRate();
			transfer.samples = m_audio_buffer.data();
			m_source->GetSamples(&transfer);
			if (transfer.samples_out > 0)
			{
				m_num_frames_avail = transfer.samples_out;
				m_audio_loaded = true;
			}
		}
	}
	const double& getSample(int channel, int64_t index) const noexcept
	{
		return m_audio_buffer[index*m_orig_nch + channel];
	}
	double& getSample(int channel, int64_t index) noexcept
	{
		return m_audio_buffer[index*m_orig_nch + channel];
	}
	const double& getSampleSafe(int channel, int64_t index) const noexcept
	{
		if (index>=0 && index<m_num_frames_avail && channel<m_orig_nch)
			return m_audio_buffer[index*m_orig_nch + channel];
		return m_dummysample;
	}
	double& getSampleSafe(int channel, int64_t index) noexcept
	{
		if (index>=0 && index<m_num_frames_avail && channel<m_orig_nch)
			return m_audio_buffer[index*m_orig_nch + channel];
		return m_dummysample;
	}
	pointer_range<double> getRange(int64_t start, int64_t len)
	{
		return pointer_range<double>(m_audio_buffer.data(), m_num_frames_avail, m_orig_nch, m_orig_sr, start, len);
	}
private:
	std::vector<double> m_audio_buffer;
	AudioAccessor* m_audio_accessor = nullptr;
	PCM_source* m_source = nullptr;
	MediaItem_Take* m_take = nullptr;
	MediaTrack* m_track = nullptr;
	int m_orig_nch = 0;
	double m_orig_sr = 0.0;
	int64_t m_num_frames_avail = 0;
	SourceType m_sourcetype = ST_None;
	bool m_valid = false;
	bool m_audio_loaded = false;
	std::string m_error_string;
	double m_dummysample = 0.0;
	void init_from_take(MediaItem_Take* take)
	{
		if (take == nullptr)
			return;
		PCM_source* src = GetMediaItemTake_Source(take);
		m_orig_nch = src->GetNumChannels();
		m_orig_sr = src->GetSampleRate();
		m_take = take;
		AudioAccessor* acc = CreateTakeAudioAccessor(take);
		if (acc != nullptr)
		{
			m_audio_accessor = acc;
			m_valid = true;
			m_sourcetype = ST_Take;
		}
	}
	void init_from_track(MediaTrack* track)
	{
		
	}
	void init_from_pcm_source(PCM_source* source, bool clonesource)
	{
		if (source == nullptr)
			return;
		if (clonesource == true)
			m_source = source->Duplicate();
		else m_source = source;
		m_valid = true;
		m_sourcetype = ST_PCMSource;
		m_orig_nch = m_source->GetNumChannels();
		m_orig_sr = m_source->GetSampleRate();
	}
};

template<typename RangeType>
inline void save_range_to_file(const RangeType& acc, std::string fn)
{
	char cfg[] = { 'e','v','a','w', 32, 0 };
	PCM_sink* sink = PCM_Sink_Create(fn.c_str(),
		cfg, sizeof(cfg), acc.numberOfChannels(), acc.sampleRate(), false);
	if (sink != nullptr)
	{
		std::vector<double> sinkbuf(acc.numberOfChannels()*acc.numberOfFrames());
		std::vector<double*> sinkbufptrs(acc.numberOfChannels());
		for (int i = 0; i < acc.numberOfChannels(); ++i)
			sinkbufptrs[i] = &sinkbuf[i*acc.numberOfFrames()];
		for (int i = 0; i < acc.numberOfFrames(); ++i)
		{
			for (int j = 0; j < acc.numberOfChannels(); ++j)
			{
				sinkbufptrs[j][i] = acc.getSample(j, i);
			}
		}
		sink->WriteDoubles(sinkbufptrs.data(), acc.numberOfFrames(), acc.numberOfChannels(), 0, 1);
		delete sink;
	}
}

template<typename T>
class ReverseAudioRange
{
public:
	ReverseAudioRange() {}
	explicit ReverseAudioRange(T range) : m_sourcerange(range) 
	{
	}
	const double& getSample(int chan, int64_t index) const noexcept
	{
		int64_t reverseindex = (m_sourcerange.numberOfFrames() - 1 - index);
		return m_sourcerange.getSample(chan, reverseindex);
	}
	int numberOfChannels() const noexcept { return m_sourcerange.numberOfChannels(); }
	double sampleRate() const noexcept { return m_sourcerange.sampleRate(); }
	int64_t numberOfFrames() const noexcept { return m_sourcerange.numberOfFrames(); }
private:
	T m_sourcerange;
};

template<typename T>
ReverseAudioRange<T> reverse_range(T src)
{
	return ReverseAudioRange<T>(src);
}

template<typename T>
class SlicedAudioRange
{
public:
	SlicedAudioRange() {}
	explicit SlicedAudioRange(T range, double startseconds, double lenseconds) 
		: m_sourcerange(range), m_startseconds(startseconds), m_lenseconds(lenseconds)
	{
	}
	const double& getSample(int chan, int64_t index) const noexcept
	{
		int64_t slicedindex = m_startseconds * m_sourcerange.sampleRate() + index;
		return m_sourcerange.getSample(chan, slicedindex);
	}
	int numberOfChannels() const noexcept { return m_sourcerange.numberOfChannels(); }
	double sampleRate() const noexcept { return m_sourcerange.sampleRate(); }
	int64_t numberOfFrames() const noexcept { return m_sourcerange.sampleRate()*m_lenseconds; }
private:
	T m_sourcerange;
	double m_startseconds = 0.0;
	double m_lenseconds = 0.0;
};

template<typename T>
SlicedAudioRange<T> slice_range(T src, double start, double len)
{
	return SlicedAudioRange<T>(src,start,len);
}


template<typename T>
class MonoAudioRange
{
public:
	MonoAudioRange() {}
	explicit MonoAudioRange(T range, int whichchannel) 
		: m_sourcerange(range)
	{
		m_whichchannel = bound_value(0, whichchannel, m_sourcerange.numberOfChannels() - 1);
	}
	const double& getSample(int, int64_t index) const noexcept
	{
		return m_sourcerange.getSample(m_whichchannel, index);
	}
	int numberOfChannels() const noexcept { return 1; }
	double sampleRate() const noexcept { return m_sourcerange.sampleRate(); }
	int64_t numberOfFrames() const noexcept { return m_sourcerange.numberOfFrames(); }
private:
	T m_sourcerange;
	int m_whichchannel = 0;
};

template<typename T>
MonoAudioRange<T> mono_range(T src, int chan)
{
	return MonoAudioRange<T>(src,chan);
}

template<typename T>
class ExtractChannelsAudioRange
{
public:
	ExtractChannelsAudioRange() {}
	explicit ExtractChannelsAudioRange(T range, std::initializer_list<int> whichchans)
		: m_sourcerange(range)
	{
		m_which_channels = whichchans;
	}
	const double& getSample(int chan, int64_t index) const noexcept
	{
		int sourcechan = m_which_channels[chan];
		if (sourcechan>=0)
			return m_sourcerange.getSample(sourcechan, index);
		return m_silence_sample;
	}
	int numberOfChannels() const noexcept { return m_which_channels.size(); }
	double sampleRate() const noexcept { return m_sourcerange.sampleRate(); }
	int64_t numberOfFrames() const noexcept { return m_sourcerange.numberOfFrames(); }
private:
	T m_sourcerange;
	std::vector<int> m_which_channels;
	double m_silence_sample = 0.0;
};

template<typename T>
ExtractChannelsAudioRange<T> channels_range(T src, std::initializer_list<int> chans)
{
	return ExtractChannelsAudioRange<T>(src, chans);
}

class ConcatenatedAudioRange
{
public:
	ConcatenatedAudioRange() {}
	template<typename T>
	void addRange(T range)
	{
		m_ranges.emplace_back(std::make_shared<concrete_range<T>>(range));
		m_cur_len += range.numberOfFrames();
	}
	const double& getSample(int chan, int64_t index) const noexcept
	{
		int64_t count = 0;
		// this is obviously somewhat inefficent, should use a binary search etc, but will do for now
		for (auto& e : m_ranges)
		{
			int64_t range_start = count;
			int64_t range_end = range_start + e->numFrames();
			if (index >= range_start && index < range_end)
			{
				int64_t offsetindex = index - range_start;
				if (offsetindex>=0 && offsetindex<e->numFrames())
					return e->getSample(chan, index - range_start);
			}
			count += e->numFrames();
		}
		return m_silence_sample;
	}
	int numberOfChannels() const noexcept { return 1; }
	double sampleRate() const noexcept { return 44100.0; }
	int64_t numberOfFrames() const noexcept { return m_cur_len; }
private:
	struct abstract_range
	{
		virtual ~abstract_range() {}
		virtual const double& getSample(int chan, int64_t index) = 0;
		virtual int numChans() = 0;
		virtual int64_t numFrames() = 0;
	};
	template<typename T>
	struct concrete_range : abstract_range
	{
		concrete_range(T range) : m_source(range) {}
		const double& getSample(int chan, int64_t index) override
		{
			return m_source.getSample(chan, index);
		}
		int numChans() override
		{
			return m_source.numberOfChannels();
		}
		int64_t numFrames() override
		{
			return m_source.numberOfFrames();
		}
		T m_source;
	};
	std::vector<std::shared_ptr<abstract_range>> m_ranges;
	double m_silence_sample = 0.0;
	int64_t m_cur_len = 0;
};

void test_mrp_audio_accessor()
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return;
	MRPAudioAccessor acc(GetSelectedMediaItem(nullptr, 0));
	if (acc.isValid() == true)
	{
		acc.loadAudioToMemory();
		if (acc.isLoaded() == true)
		{
			readbg() << acc.numberOfChannels() << " " << acc.numberOfFrames() << " " << acc.sampleRate() << "\n";
			static int counter = 0;
			std::string fn = std::string("C:/MusicAudio/batchtesti/irptestout_") + std::to_string(counter) + ".wav";
			auto range = acc.getRange(0, acc.numberOfFrames());
			
			//auto reversereverserange = reverse_range(reverserange);
			auto chansrange = channels_range(range, { 7,0,3 });
			readbg() << "chans range numchans " << chansrange.numberOfChannels() << "\n";
			//auto reverserange = reverse_range(chansrange);
			//readbg() << "reverse range numchans " << reverserange.numberOfChannels() << "\n";
			ConcatenatedAudioRange conc;
			conc.addRange(acc.getRange(0, 44100));
			conc.addRange(reverse_range(acc.getRange(0*44100, 1*44100)));
			conc.addRange(acc.getRange(0 * 44100, 8 * 44100));
			//auto outputrange = slice_range(reverse_range(channels_range(range, { -1 , 7 , 0 , 3 , -1 })), 1.0,3.0);
			auto outputrange = channels_range(conc, { -1,-1,0,0 });
			double bench_t0 = time_precise();
			save_range_to_file(outputrange, fn);
			double bench_t1 = time_precise();
			double bench_ms = (bench_t1 - bench_t0)*1000.0;
			++counter;
			readbg() << "render done in " << bench_ms << " milliseconds\n";
		}
		else readbg() << "MRPAudioAccessor failed to load audio to memory\n";
	}
	else readbg() << "MRPAudioAccessor not valid\n";
	
}
