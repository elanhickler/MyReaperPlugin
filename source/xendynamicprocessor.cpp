#include "xendynamicsprocessor.h"
#include "WDL/WDL/db2val.h"
#include "picojson/picojson.h"

VolumeAnalysisControl::VolumeAnalysisControl(MRPWindow* parent) : LiceControl(parent)
{

}

void VolumeAnalysisControl::setAnalysisData(volume_analysis_data data)
{
	m_data = std::move(data);
	repaint();
}

void VolumeAnalysisControl::paint(PaintEvent& ev)
{
	LICE_FillRect(ev.bm, 0, 0, ev.bm->getWidth(), ev.bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	if (m_data.m_datapoints.size() < 2)
		return;
	for (int i = 0; i < m_data.m_datapoints.size()-1; ++i)
	{
		double xcor0 = (double)getWidth() / m_data.m_datapoints.size()*i;
		double xcor1 = (double)getWidth() / m_data.m_datapoints.size()*(i+1);
		double v0 = 1.0 - m_data.m_datapoints[i].m_abs_peak;
		double v1 = 1.0 - m_data.m_datapoints[i+1].m_abs_peak;
		double ycor0 = v0*getHeight();
		double ycor1 = v1*getHeight();
		LICE_FLine(ev.bm, xcor0, ycor0, xcor1, ycor1, LICE_RGBA(255, 255, 255, 255),1.0f,0,false);
	}
}

DynamicsProcessorWindow::DynamicsProcessorWindow(HWND parent) : MRPWindow(parent, "Dynamics processor")
{
	m_importbut = std::make_shared<WinButton>(this,"Import item");
	add_control(m_importbut);
	m_renderbut = std::make_shared<WinButton>(this, "Render");
	add_control(m_renderbut);
	m_renderbut->GenericNotifyCallback = [this](GenericNotifications)
	{
		render_dynamics_transform();
	};
	m_analysiscontrol1 = std::make_shared<VolumeAnalysisControl>(this);
	add_control(m_analysiscontrol1);
	m_analysiscontrol2 = std::make_shared<VolumeAnalysisControl>(this);
	add_control(m_analysiscontrol2);
	m_transformenvelope1 = std::make_shared<breakpoint_envelope>("Dynamics curve", LICE_RGBA(0, 255, 0, 255));
	m_transformenvelope1->add_point({ 0.0,0.0, envbreakpoint::Power },true);
	m_transformenvelope1->add_point({ 0.5,0.5, envbreakpoint::Power },true);
	m_transformenvelope1->add_point({ 1.0,1.0, envbreakpoint::Power }, true);
	m_envelopecontrol1 = std::make_shared<EnvelopeControl>(this);
	m_envelopecontrol1->add_envelope(m_transformenvelope1);
	m_envelopecontrol1->GenericNotifyCallback = [this](GenericNotifications reason)
	{
		do_dynamics_transform_visualization();
		if (reason != GenericNotifications::ObjectMoved)
			save_state();
	};
	add_control(m_envelopecontrol1);
	m_importbut->GenericNotifyCallback = [this](GenericNotifications)
	{
		import_item();
	};
	m_window_sizes = { 1.0,2.0,5.0,10.0,20.0,50.0,100.0,200.0, 500.0 };
	m_windowsizecombo1 = std::make_shared<WinComboBox>(this);
	for (int i = 0; i < m_window_sizes.size();++i)
	{
		char buf[20];
		sprintf(buf, "%.1f ms",m_window_sizes[i]);
		m_windowsizecombo1->addItem(buf, i);
	}
	m_windowsizecombo1->setSelectedIndex(5);
	m_windowsizecombo1->SelectedChangedCallback = [this](int index)
	{
		if (index >= 0)
		{
			import_item();
		}
	};
	add_control(m_windowsizecombo1);
	m_windowsizelabel1 = std::make_shared<WinLabel>(this, "Window size");
	add_control(m_windowsizelabel1);
	load_state();
}

void DynamicsProcessorWindow::resized()
{
	int w = getSize().getWidth();
	int h = getSize().getHeight();
	int envw = 400;
	m_analysiscontrol1->setBounds({ 0, 25, w/2-envw/2, h-25 });
	m_analysiscontrol2->setBounds({ w/2+envw/2, 25, w / 2 - envw / 2, h - 25 });
	m_envelopecontrol1->setBounds({ w/2-envw/2+5,25,envw-10,envw });
	m_importbut->setBounds({ 5,2,70,20 });
	m_renderbut->setBounds({ 80,2,70,20 });
	m_windowsizelabel1->setBounds({ 155,5,100,20 });
	m_windowsizecombo1->setBounds({ 260,2,100,20 });
}

void DynamicsProcessorWindow::do_dynamics_transform_visualization()
{
	volume_analysis_data* srcdata = m_analysiscontrol1->getAnalysisData();
	volume_analysis_data destdata;
	int numdatapoints = srcdata->m_datapoints.size();
	destdata.m_datapoints.resize(numdatapoints);
	for (int i = 0; i < numdatapoints; ++i)
	{
		double gainfactor = 1.0;
		const double srcval = srcdata->m_datapoints[i].m_abs_peak;
		if (m_envelope_is_db == true)
		{
			double srcvaldb = VAL2DB(srcval);
			double srcvaldbnorm = map_value(srcvaldb, -96.0, 0.0, 0.0, 1.0);
			double envnormval = m_transformenvelope1->interpolate(srcvaldbnorm);
			double envdbvalue = -96.0 + 96.0*envnormval;
			double envgainval = exp((envdbvalue)*0.11512925464970228420089957273422);
			double dbdiff = envdbvalue - srcvaldb;
			gainfactor = exp((dbdiff)*0.11512925464970228420089957273422);
			double destval = srcval * gainfactor;
			destdata.m_datapoints[i].m_abs_peak = destval;
		}
		else
		{
			double envnormval = m_transformenvelope1->interpolate(srcval);
			double diff = envnormval - srcval;
			gainfactor = 0.0;
			if (srcval>0.0001)
				gainfactor = envnormval / srcval;
			double destval = srcval * gainfactor;
			destdata.m_datapoints[i].m_abs_peak = destval;
		}
		
		destdata.m_datapoints[i].m_time_stamp = srcdata->m_datapoints[i].m_time_stamp;
		
	}
	m_analysiscontrol2->setAnalysisData(destdata);
}

void DynamicsProcessorWindow::render_dynamics_transform()
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(nullptr, 0);
	MediaItem_Take* take = GetActiveTake(item);
	if (take != nullptr)
	{
		PCM_source* src = GetMediaItemTake_Source(take);
		if (src != nullptr)
		{
			int64_t numframes = src->GetLength()*src->GetSampleRate();
			std::vector<double> buf(numframes*src->GetNumChannels());
			PCM_source_transfer_t transfer = { 0 };
			transfer.length = numframes;
			transfer.nch = src->GetNumChannels();
			transfer.samplerate = src->GetSampleRate();
			transfer.samples = buf.data();
			src->GetSamples(&transfer);
			volume_analysis_data* srcdata = m_analysiscontrol1->getAnalysisData();
			int numdatapoints = srcdata->m_datapoints.size();
			int64_t audiocounter = 0;
			int numchans = src->GetNumChannels();
			int windowsize = srcdata->m_windowsize;
			double prevwindowgain = 0.0;
			for (int i = 0; i < numdatapoints; ++i)
			{
				const double srcval0 = srcdata->m_datapoints[i].m_abs_peak;
				if (m_envelope_is_db == true)
				{
					double srcvaldb0 = VAL2DB(srcval0);
					double srcvaldbnorm0 = map_value(srcvaldb0, -96.0, 0.0, 0.0, 1.0);
					double envnormval0 = m_transformenvelope1->interpolate(srcvaldbnorm0);
					double envdbvalue0 = -96.0 + 96.0*envnormval0;
					double envgainval0 = exp((envdbvalue0)*0.11512925464970228420089957273422);
					double diff0 = envgainval0 - srcval0;
					double dbdiff0 = envdbvalue0 - srcvaldb0;
					const double gainfactor0 = exp((dbdiff0)*0.11512925464970228420089957273422);
					for (int j = 0; j < windowsize; ++j)
					{
						double ramped = 1.0;
						if (j < windowsize / 2)
							ramped = 1.0 / (windowsize / 2)*j;
						else ramped = 1.0; // 1.0 - (1.0 / (windowsize / 2)*(j - windowsize / 2));
						double gain0 = prevwindowgain;
						double gain1 = gainfactor0;
						double gaindiff = gain1 - gain0;
						double interpolated = gain0 + gaindiff*ramped;
						int64_t index = audiocounter;
						if (index >= numframes)
							break;
						for (int k = 0; k < numchans; ++k)
						{
							buf[index*numchans + k] *= interpolated;
						}
						++audiocounter;
					}
				}
				else
				{
					double envnormval = m_transformenvelope1->interpolate(srcval0);
					double diff = envnormval - srcval0;
					double gainfactor = 0.0;
					if (srcval0>0.0001)
						gainfactor = envnormval / srcval0;
					for (int j = 0; j < windowsize; ++j)
					{
						double ramped = 1.0;
						if (j <= windowsize / 2)
							ramped = 1.0 / (windowsize / 2)*j;
						else ramped = 1.0; // 1.0 - ((1.0 / (windowsize / 2))*(j - windowsize / 2));
						double gain0 = prevwindowgain;
						double gain1 = gainfactor;
						double gaindiff = gain1 - gain0;
						double interpolated = gain0 + gaindiff*ramped;
						int64_t index = audiocounter;
						if (index >= numframes)
							break;
						for (int k = 0; k < numchans; ++k)
						{
							buf[index*numchans + k] *= interpolated;
						}
						++audiocounter;
					}
					prevwindowgain = gainfactor;
				}
			}
			char cfg[] = { 'e','v','a','w', 32, 0 };
			char ppbuf[2048];
			GetProjectPath(ppbuf, 2048);
			GUID theguid;
			genGuid(&theguid);
			char guidtxt[64];
			guidToString(&theguid, guidtxt);
			std::string outfn = std::string(ppbuf) + "/" + guidtxt + ".wav";
			PCM_sink* sink = PCM_Sink_Create(outfn.c_str(), 
				cfg, sizeof(cfg), numchans, src->GetSampleRate(), false);
			if (sink != nullptr)
			{
				std::vector<double> sinkbuf(numchans*numframes);
				std::vector<double*> sinkbufptrs(numchans);
				for (int i = 0; i < numchans; ++i)
					sinkbufptrs[i] = &sinkbuf[i*numframes];
				for (int i = 0; i < numframes; ++i)
				{
					for (int j = 0; j < numchans; ++j)
					{
						sinkbufptrs[j][i] = buf[i*numchans + j];
					}
				}
				sink->WriteDoubles(sinkbufptrs.data(), numframes, numchans, 0, 1);
				delete sink;
				InsertMedia(outfn.c_str(), 3);
			}
			//readbg() << "render finished\n";
		}
	}
}

void DynamicsProcessorWindow::import_item()
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(nullptr, 0);
	MediaItem_Take* take = GetActiveTake(item);
	if (take != nullptr)
	{
		PCM_source* src = GetMediaItemTake_Source(take);
		if (src != nullptr)
		{
			int64_t numframes = src->GetLength()*src->GetSampleRate();
			std::vector<double> buf(numframes*src->GetNumChannels());
			PCM_source_transfer_t transfer = { 0 };
			transfer.length = numframes;
			transfer.nch = src->GetNumChannels();
			transfer.samplerate = src->GetSampleRate();
			transfer.samples = buf.data();
			src->GetSamples(&transfer);
			double windowlen = m_window_sizes[m_windowsizecombo1->getSelectedIndex()]/1000.0;
			auto data = analyze_audio_volume(windowlen*src->GetSampleRate(), src->GetNumChannels(), buf.data(), numframes);
			m_analysiscontrol1->setAnalysisData(data);
			do_dynamics_transform_visualization();
		}
	}
}

using ByteVector = std::vector<unsigned char>;

template<typename T>
inline void append_primitive_to_bytevector(ByteVector& bv, T x)
{
	T temp = x;
	unsigned char* ptr = (unsigned char*)&temp;
	for (size_t i = 0; i < sizeof(x); ++i)
		bv.push_back(ptr[i]);
}

inline ByteVector& operator << (ByteVector& bv, int64_t x)
{
	append_primitive_to_bytevector(bv, x);
	return bv;
}

inline ByteVector& operator << (ByteVector& bv, int x)
{
	append_primitive_to_bytevector(bv, x);
	return bv;
}

inline ByteVector& operator << (ByteVector& bv, double x)
{
	append_primitive_to_bytevector(bv, x);
	return bv;
}

inline ByteVector& operator << (ByteVector& bv, envbreakpoint x)
{
	append_primitive_to_bytevector(bv, x.get_x());
	append_primitive_to_bytevector(bv, x.get_y());
	append_primitive_to_bytevector(bv, x.get_shape());
	append_primitive_to_bytevector(bv, x.get_param1());
	append_primitive_to_bytevector(bv, x.get_param2());
	append_primitive_to_bytevector(bv, x.get_status());
	return bv;
}

template<typename T>
inline bool extract_primitive_from_bytevector(const ByteVector& bv, int& pos, T& x)
{
	if (pos >= bv.size())
		return false;
	T* ptr = (T*)&bv[pos];
	x = *ptr;
	pos += sizeof(T);
	return true;
}

template<typename T>
inline bool extract_primitives_from_bytevector_helper(const ByteVector& bv, int& pos, T& x)
{
	return extract_primitive_from_bytevector(bv, pos, x);
}

template<typename T, typename... Ts>
inline bool extract_primitives_from_bytevector_helper(const ByteVector& bv, int& pos, T& x, Ts&... xs)
{
	extract_primitive_from_bytevector(bv, pos, x);
	extract_primitives_from_bytevector_helper(bv, pos, xs...);
	return true;
}

template<typename... Ts>
inline bool extract_primitives_from_bytevector(const ByteVector& bv, int& pos, Ts&... xs)
{
	if (pos >= bv.size())
		return false;
	extract_primitives_from_bytevector_helper(bv, pos, xs...);
	return true;
}

inline bool extract_envelope_point_from_bytevector(const ByteVector& bv, int& pos, envbreakpoint& pt)
{
	if (pos >= bv.size())
		return false;
	double ptx, pty, p1, p2 = 0.0;
	int status = 0;
	envbreakpoint::PointShape shape = envbreakpoint::Linear;
	extract_primitives_from_bytevector(bv, pos, ptx,pty,shape,p1,p2,status);
	pt.set_x(ptx);
	pt.set_y(pty);
	pt.set_shape(shape);
	pt.set_param1(p1);
	pt.set_param2(p2);
	pt.set_status(status);
	return true;
}

static int pc_base64decode(const char *src, unsigned char *dest, int destsize)
{
	int accum = 0, nbits = 0, wpos = 0;
	while (*src && wpos < destsize)
	{
		int x = 0;
		char c = *src++;
		if (c >= 'A' && c <= 'Z') x = c - 'A';
		else if (c >= 'a' && c <= 'z') x = c - 'a' + 26;
		else if (c >= '0' && c <= '9') x = c - '0' + 52;
		else if (c == '+') x = 62;
		else if (c == '/') x = 63;
		else break;

		accum = (accum << 6) | x;
		nbits += 6;

		while (nbits >= 8 && wpos < destsize)
		{
			nbits -= 8;
			dest[wpos++] = (char)((accum >> nbits) & 0xff);
		}
	}
	return wpos;
}


static void pc_base64encode(const unsigned char *in, char *out, int len)
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while (len>0)
	{
		len--;
		accum <<= 8;
		shift += 8;
		accum |= *in++;
		while (shift >= 6)
		{
			shift -= 6;
			*out++ = alphabet[(accum >> shift) & 0x3F];
		}
	}
	if (shift == 4)
	{
		*out++ = alphabet[(accum & 0xF) << 2];
		*out++ = '=';
	}
	else if (shift == 2)
	{
		*out++ = alphabet[(accum & 0x3) << 4];
		*out++ = '=';
		*out++ = '=';
	}

	*out++ = 0;
}


void DynamicsProcessorWindow::save_state()
{
	ByteVector state;
	int version = 0;
	state << version;
	state << m_transformenvelope1->get_num_points();
	for (int i = 0; i < m_transformenvelope1->get_num_points(); ++i)
	{
		const envbreakpoint& pt = m_transformenvelope1->get_point(i);
		state << pt;
	}
	std::vector<char> output(4*(state.size() / 3)+16);
	pc_base64encode(state.data(), output.data(), state.size());
	SetExtState("xenakios_dynamics_processor", "state_v0", output.data(), true);
}

void DynamicsProcessorWindow::load_state()
{
	const char* b64state = GetExtState("xenakios_dynamics_processor", "state_v0");
	if (b64state != nullptr)
	{
		int srclen = strlen(b64state);
		if (srclen > 0)
		{
			ByteVector decoded(srclen);
			pc_base64decode(b64state, decoded.data(), srclen);
			int streampos = 0;
			int version = 0;
			int numpoints = 0;
			extract_primitives_from_bytevector(decoded, streampos, version, numpoints);
			readbg() << "extracted version number " << version << "\n";
			readbg() << "extracted number of env points " << numpoints << "\n";
			if (numpoints > 0 && numpoints<1000000)
			{
				m_transformenvelope1->remove_all_points();
				for (int i = 0; i < numpoints; ++i)
				{
					envbreakpoint pt;
					extract_envelope_point_from_bytevector(decoded, streampos, pt);
					m_transformenvelope1->add_point(pt, false);
				}
				m_transformenvelope1->sort_points();
				m_envelopecontrol1->repaint();
			}
		}
		else readbg() << "base64 string zero len\n";
	}
	else readbg() << "state null\n";
	
}

DynamicsProcessorWindow* g_dynprocwindow = nullptr;

void show_dynamics_processor_window(HWND parent)
{
	if (g_dynprocwindow == nullptr)
	{
		g_dynprocwindow = new DynamicsProcessorWindow(parent);
	}
	g_dynprocwindow->setVisible(true);
	g_dynprocwindow->setSize(900, 480);
}