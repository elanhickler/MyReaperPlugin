#include "xendynamicsprocessor.h"
#include "WDL/WDL/db2val.h"
#include "picojson/picojson.h"
#include <fstream>

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
	if (m_audio_view_painter.paint(ev.bm, -1.0, -1.0, 0, 0, getWidth(), getHeight()) == false)
	{
		LICE_FillRect(ev.bm, 0, 0, ev.bm->getWidth(), ev.bm->getHeight(), LICE_RGBA(0, 0, 0, 255));	
	}
	if (m_data.m_datapoints.size() < 2)
		return;
	if (m_show_analysis_curve == false)
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
	m_importbut = std::make_shared<WinButton>(this, "Import item");
	add_control(m_importbut);
	m_renderbut = std::make_shared<WinButton>(this, "Render");
	add_control(m_renderbut);
	m_renderbut->GenericNotifyCallback = [this](GenericNotifications)
	{
		render_dynamics_transform();
		write_transformed_to_file();
	};
	m_analysiscontrol1 = std::make_shared<VolumeAnalysisControl>(this);
	add_control(m_analysiscontrol1);
	m_analysiscontrol2 = std::make_shared<VolumeAnalysisControl>(this);
	add_control(m_analysiscontrol2);
	m_transformenvelope1 = std::make_shared<breakpoint_envelope>("Dynamics curve", LICE_RGBA(0, 255, 0, 255));
	m_transformenvelope1->add_point({ 0.0,0.0, envbreakpoint::Power }, true);
	m_transformenvelope1->add_point({ 0.5,0.5, envbreakpoint::Power }, true);
	m_transformenvelope1->add_point({ 1.0,1.0, envbreakpoint::Power }, true);
	m_envelopecontrol1 = std::make_shared<EnvelopeControl>(this);
	m_envelopecontrol1->add_envelope(m_transformenvelope1);
	m_envelopecontrol1->GenericNotifyCallback = [this](GenericNotifications reason)
	{
		if (reason != GenericNotifications::ObjectMoved)
		{
			m_analysiscontrol2->setShowAnalysisCurve(false);
			render_dynamics_transform();
			save_state();
		}
		else
		{
			m_analysiscontrol2->setShowAnalysisCurve(true);
			do_dynamics_transform_visualization();
		}
	};
	add_control(m_envelopecontrol1);
	m_importbut->GenericNotifyCallback = [this](GenericNotifications)
	{
		import_item();
	};
	m_window_sizes = { 1.0,2.0,5.0,10.0,20.0,50.0,100.0,200.0, 500.0 };
	m_windowsizecombo1 = std::make_shared<WinComboBox>(this);
	for (int i = 0; i < m_window_sizes.size(); ++i)
	{
		char buf[20];
		sprintf(buf, "%.1f ms", m_window_sizes[i]);
		m_windowsizecombo1->addItem(buf, i);
	}
	m_windowsizecombo1->setSelectedIndex(5);
	m_windowsizecombo1->SelectedChangedCallback = [this](int index)
	{
		if (index >= 0)
		{
			import_item();
			m_analysiscontrol2->setShowAnalysisCurve(false);
			render_dynamics_transform();
			save_state();
		}
	};
	add_control(m_windowsizecombo1);
	m_windowsizelabel1 = std::make_shared<WinLabel>(this, "Window size",true);
	add_control(m_windowsizelabel1);

	m_slider1 = std::make_shared<ReaSlider>(this);
	m_slider1->SliderValueCallback = [this](GenericNotifications reason, double v)
	{
		if (reason == GenericNotifications::AfterManipulation)
		{
			render_dynamics_transform();
			save_state();
		}
	};
    add_control(m_slider1);
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
	m_slider1->setBounds({ 365,5,200,20 });
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
	if (m_acc == nullptr)
		return;
	if (m_acc->isLoaded()==true)
	{
		auto av = m_acc->getRange();
		volume_analysis_data* srcdata = m_analysiscontrol1->getAnalysisData();
		int numdatapoints = srcdata->m_datapoints.size();
		int64_t audiocounter = 0;
		int numchans = av.numberOfChannels();
		int64_t numframes = av.numberOfFrames();
		int windowsize = srcdata->m_windowsize;
		double prevwindowgain = 0.0;
		m_transformed_audio.resize(numchans*numdatapoints*windowsize);
		breakpoint_envelope env("Volume changes");
		
		for (int i = 0; i < numdatapoints; ++i)
		{
			const double srcval0 = srcdata->m_datapoints[i].m_abs_peak;
			int peakpos = srcdata->m_datapoints[i].m_max_peak_pos;
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
						av.getSampleRef(k, index) *= interpolated;
						//buf[index*numchans + k] *= interpolated;
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
				double p1 = m_slider1->getValue();
				env.add_point({ (double)(i*windowsize+peakpos)/av.sampleRate(),gainfactor, envbreakpoint::Power,p1 },false);
				continue;
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
						m_transformed_audio[index*numchans+k]=av.getSample(k, index) * interpolated;
						//buf[index*numchans + k] *= interpolated;
					}
					++audiocounter;
				}
				prevwindowgain = gainfactor;
			}
		}
		env.sort_points();
		int overcounter = 0;
		double max_sample = 0.0;
		for (int64_t i = 0; i < numframes; ++i)
		{
			double gain = env.interpolate((double)i / av.sampleRate());
			for (int j = 0; j < numchans; ++j)
			{
				double s = av.getSample(j, i)*gain;
				m_transformed_audio[i*numchans + j] = bound_value(-1.0,s,1.0);
				double abs_sample = fabs(s);
				if (abs_sample > 1.0)
				{
					++overcounter;
					if (abs_sample > max_sample)
						max_sample = abs_sample;
				}
			}
		}
		if (overcounter > 0)
			readbg() << overcounter << " samples went over! " << max_sample << "\n";
		audiobuffer_view<double> taview(m_transformed_audio.data(), m_acc->numberOfFrames(),
			m_acc->numberOfChannels(), m_acc->sampleRate());
		m_analysiscontrol2->setAudioView(taview);
	}
}

void DynamicsProcessorWindow::write_transformed_to_file()
{
	if (m_acc == nullptr)
		return;
	audiobuffer_view<double> taview(m_transformed_audio.data(), m_acc->numberOfFrames(),
		m_acc->numberOfChannels(), m_acc->sampleRate());
	char cfg[] = { 'e','v','a','w', 32, 0 };
	char ppbuf[2048];
	GetProjectPath(ppbuf, 2048);
	GUID theguid;
	genGuid(&theguid);
	char guidtxt[64];
	guidToString(&theguid, guidtxt);
	std::string outfn = std::string(ppbuf) + "/" + guidtxt + ".wav";
	int numchans = taview.numberOfChannels();
	int64_t numframes = taview.numberOfFrames();
	PCM_sink* sink = PCM_Sink_Create(outfn.c_str(),
		cfg, sizeof(cfg), numchans, taview.sampleRate(), false);
	if (sink != nullptr)
	{
		const int diskbufsize = 65536;
		std::vector<double> sinkbuf(numchans*diskbufsize);
		std::vector<double*> sinkbufptrs(taview.numberOfChannels());
		for (int i = 0; i < numchans; ++i)
			sinkbufptrs[i] = &sinkbuf[i*diskbufsize];
		int64_t diskoutcounter = 0;
		while (diskoutcounter < numframes)
		{
			int framestowrite = diskbufsize;
			for (int i = 0; i < framestowrite; ++i)
			{
				for (int j = 0; j < numchans; ++j)
				{
					sinkbufptrs[j][i] = taview.getSample(j, diskoutcounter+i);
				}
			}
			sink->WriteDoubles(sinkbufptrs.data(), framestowrite, numchans, 0, 1);
			diskoutcounter += diskbufsize;
		}
		delete sink;
		InsertMedia(outfn.c_str(), 3);
	}
}

void DynamicsProcessorWindow::import_item()
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(nullptr, 0);
	MediaItem_Take* take = GetActiveTake(item);
	m_acc = std::make_shared<MRPAudioAccessor>(take);
	m_acc->loadAudioToMemory();
	if (m_acc->isLoaded() == true)
	{
		double windowlen = m_window_sizes[m_windowsizecombo1->getSelectedIndex()] / 1000.0;
		auto av = m_acc->getRange();
		auto data = analyze_audio_volume(windowlen*m_acc->sampleRate(), av);
		m_analysiscontrol1->setAnalysisData(data);
		m_analysiscontrol1->setAudioView(m_acc->getRange());
		do_dynamics_transform_visualization();
	}
}

picojson::object to_json(breakpoint_envelope& env)
{
	picojson::object result;
	picojson::array ar;
	for (int i = 0; i<env.get_num_points(); ++i)
	{
		const envbreakpoint& pt = env.get_point(i);
		picojson::object nodeob;
		nodeob["x"] = picojson::value(pt.get_x());
		nodeob["y"] = picojson::value(pt.get_y());
		nodeob["sh"] = picojson::value((double)pt.get_shape());
		nodeob["p1"] = picojson::value(pt.get_param1());
		nodeob["p2"] = picojson::value(pt.get_param2());
		ar.push_back(picojson::value(nodeob));
	}
	result["nodes"] = picojson::value(ar);
	return result;
}

void init_from_json(breakpoint_envelope& env, picojson::object& ob)
{
	if (ob.size() == 0)
		return;
	picojson::array ar = ob["nodes"].get<picojson::array>();
	if (ar.size()>0)
	{
		env.remove_all_points();
		for (int i = 0; i<ar.size(); i++)
		{
			picojson::object node_ob = ar[i].get<picojson::object>();
			double x = node_ob["x"].get<double>();
			double y = node_ob["y"].get<double>();
			double p1 = node_ob["p1"].get<double>();
			double p2 = node_ob["p2"].get<double>();
			int shape = node_ob["sh"].get<double>();
			env.add_point({ x,y,(envbreakpoint::PointShape)shape,p1,p2 }, false);
		}
		env.sort_points();
	}

}

void DynamicsProcessorWindow::save_state()
{
	picojson::object top_object;
	top_object["plugin_version"] = picojson::value("1");
	top_object["dyn_envelope"] = picojson::value(to_json(*m_transformenvelope1));
	top_object["analysiswindowsize"] = picojson::value(m_window_sizes[m_windowsizecombo1->getSelectedIndex()]);
	picojson::value top_value(top_object);
	std::string fn = std::string(GetResourcePath()) + "/xenakios_dynamics_processor.json";
	std::ofstream file(fn);
	file << top_value.serialize(true);

}

void DynamicsProcessorWindow::load_state()
{
	std::string fn = std::string(GetResourcePath()) + "/xenakios_dynamics_processor.json";
	std::ifstream file(fn);
	if (file.is_open() == false)
	{
		readbg() << "could not open settings file\n";
		return;
	}
	picojson::value top_object_value;
	picojson::parse(top_object_value, file);
	picojson::object top_object = top_object_value.get<picojson::object>();
	double wsize = top_object["analysiswindowsize"].get<double>();
	for (int i = 0; i < m_window_sizes.size(); ++i)
	{
		if (fabs(wsize-m_window_sizes[i])<0.001)
		{
			m_windowsizecombo1->setSelectedIndex(i);
			break;
		}
	}
	init_from_json(*m_transformenvelope1, top_object["dyn_envelope"].get<picojson::object>());
	m_envelopecontrol1->repaint();
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