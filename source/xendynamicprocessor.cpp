#include "xendynamicsprocessor.h"
#include "WDL/WDL/db2val.h"

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
	m_envelopecontrol1->GenericNotifyCallback = [this](GenericNotifications)
	{
		do_dynamics_transform_visualization();
	};
	add_control(m_envelopecontrol1);
	m_importbut->GenericNotifyCallback = [this](GenericNotifications)
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
				auto data = analyze_audio_volume(1024, src->GetNumChannels(), buf.data(), numframes);
				m_analysiscontrol1->setAnalysisData(data);
			}
		}
	};
}

void DynamicsProcessorWindow::resized()
{
	int w = getSize().getWidth();
	int h = getSize().getHeight();
	int envw = 400;
	m_analysiscontrol1->setBounds({ 0, 25, w/2-envw/2, h-25 });
	m_analysiscontrol2->setBounds({ w/2+envw/2, 25, w / 2 - envw / 2, h - 25 });
	m_envelopecontrol1->setBounds({ w/2-envw/2+5,25,envw-10,envw });
	m_importbut->setBounds({ 5,2,70,23 });
	m_renderbut->setBounds({ 80,2,70,23 });
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