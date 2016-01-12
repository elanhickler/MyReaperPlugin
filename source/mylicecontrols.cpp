
#include "mylicecontrols.h"
#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "WDL/WDL/lineparse.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include <cmath>

bool g_popupmenushowing = false;

void MRP_DrawTextHelper(LICE_IBitmap* bm, LICE_CachedFont* font, std::string txt, int x, int y, int w, int h, int flags=DT_TOP|DT_LEFT)
{
	RECT r;
	r.left = x;
	r.right = x + w;
	r.top = y;
	r.bottom = y + h;
#ifdef WIN32
	font->DrawTextA(bm, txt.c_str(), -1, &r, flags);
#else
	font->DrawText(bm, txt.c_str(), -1, &r, flags);
#endif
}

TestControl::TestControl(MRPWindow* parent, bool delwhendraggedoutside) :
	LiceControl(parent), m_delete_point_when_dragged_outside(delwhendraggedoutside) 
{
	m_font.SetFromHFont(CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void TestControl::paint(PaintEvent& ev)
{
	LICE_IBitmap* bm = ev.bm;
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	for (int i=0;i<m_points.size();++i)
	{
		LICE_pixel color=LICE_RGBA(255,255,255,255);
		if (m_hot_point==i)
			color=LICE_RGBA(255, 0, 0, 255);
		auto& e = m_points[i];
		LICE_FillCircle(bm, e.m_x, e.m_y, m_circlesize, color);
	}
	if (m_test_text.size() > 0)
	{
		RECT r;
		r.left = 0;
		r.right = bm->getWidth();
		r.top = 0;
		r.bottom = 25;
#ifdef WIN32
		m_font.DrawTextA(bm, m_test_text.c_str(), m_test_text.size(), &r, 0);
#else
		m_font.DrawText(bm, m_test_text.c_str(), m_test_text.size(), &r, 0);
#endif
	}
}

int TestControl::find_hot_point(int x, int y)
{
	for (int i = m_points.size()-1; i > -1; --i)
	{
		const point& pt=m_points[i];
		if (is_point_in_rect(x,y,pt.m_x-m_circlesize,pt.m_y-m_circlesize,2*m_circlesize,2*m_circlesize)==true)
		{
			return i;
		}
	}
	return -1;
}

void update_touched_fx(fx_param_t& entry)
{
	int trackout = 0;
	int fxout = 0;
	int paramout = 0;
	if (GetLastTouchedFX(&trackout, &fxout, &paramout) == true)
	{
		entry.tracknum = trackout;
		entry.fxnum = fxout;
		entry.paramnum = paramout;
		//readbg() << trackout << " " << fxout << " " << paramout << "\n";
	}
}

void TestControl::mouseDoubleClicked(const MouseEvent &ev)
{
	if (m_hot_point==-1)
	{
		m_points.push_back({ ev.m_x,ev.m_y });
		m_hot_point=(int)m_points.size()-1;
		repaint();
	}
}

void TestControl::mousePressed(const MouseEvent& ev)
{
	if (g_popupmenushowing == true)
	{
		g_popupmenushowing = false;
		return;
	}
	if (ev.m_mb == MouseEvent::MBLeft && ev.m_modkeys.isModifierKeyDown(MKAppleOrWindowsKey) == true)
	{
		readbg() << "you pressed left button with Windows key down\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBLeft && ev.m_modkeys.isModifierKeyDown(MKControl) == true)
	{
		readbg() << "you pressed left button with control key down\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBLeft && ev.m_modkeys.isModifierKeyDown(MKShift) == true)
	{
		readbg() << "you pressed left button with shift key down\n";
		m_mousedown = true;
		return;
	}
	if (ev.m_mb == MouseEvent::MBLeft && ev.m_modkeys.isModifierKeyDown(MKAlt) == true)
	{
		readbg() << "you pressed left button with alt key down\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBRight && ev.m_modkeys.areModifiersDown({ MKAlt, MKShift }))
	{
		readbg() << "you pressed right button with alt and shift keys down\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBMiddle)
	{
		readbg() << "you pressed the middle button!\n";
		return;
	}
	if (ev.m_mb == MouseEvent::MBRight)
	{
		PopupMenu menu(getWindowHandle());
		menu.add_menu_item("First action", [](PopupMenu::CheckState) { readbg() << "first action chosen\n"; });
		if (m_hot_point >= 0)
		{
			menu.add_menu_item("Control last touched parameter with X position", [this](PopupMenu::CheckState)
			{
				update_touched_fx(m_points[m_hot_point].m_x_target);
			});
			menu.add_menu_item("Control last touched parameter with Y position", [this](PopupMenu::CheckState)
			{
				update_touched_fx(m_points[m_hot_point].m_y_target);
			});
			menu.add_menu_item("Remove point", [this](PopupMenu::CheckState)
			{
				m_points.erase(m_points.begin() + m_hot_point);
				m_hot_point = -1;
				repaint();
			});
			
		}
		for (int i = 0; i < 10; ++i)
		{
			menu.add_menu_item(std::to_string(i + 1), [i](PopupMenu::CheckState)
			{
				readbg() << "You chose number " << i + 1 << "\n";
			});
		}
		g_popupmenushowing = true;
		menu.execute(ev.m_x, ev.m_y);
		return;
	}
	m_mousedown=true;
	
}

void TestControl::mouseMoved(const MouseEvent& ev)
{
	if (m_mousedown == true && ev.m_modkeys.isModifierKeyDown(MKShift) == true)
	{
		readbg() << "mouse dragged with shift " << ev.m_x << " " << ev.m_y << "\n";
		return;
	}
	if (m_mousedown==false)
	{
		int found=find_hot_point(ev.m_x, ev.m_y);
		if (found!=m_hot_point)
		{
			m_hot_point=found;
			repaint();
		}
	} else
	{
		if (m_hot_point>=0)
		{
			if (m_delete_point_when_dragged_outside == true)
			{
				if (is_point_in_rect(m_points[m_hot_point].m_x, m_points[m_hot_point].m_y, 
					-30, -30, getWidth()+60, getHeight()+60) == false)
				{
					m_points.erase(m_points.begin() + m_hot_point);
					m_hot_point = -1;
					repaint();
					return;
				}
			}
			m_points[m_hot_point].m_x = ev.m_x; // bound_value(0, x, getWidth());
			m_points[m_hot_point].m_y = ev.m_y; // bound_value(0, y, getHeight());
			if (PointMovedCallback)
			{
				double normx = bound_value(0.0, 1.0 / getWidth()*ev.m_x, 1.0);
				double normy = bound_value(0.0, 1.0 / getHeight()*ev.m_y, 1.0);
				PointMovedCallback(m_hot_point, normx, normy);
			}
			repaint();
		}
	}
}

void TestControl::mouseReleased(const MouseEvent& ev)
{
	m_mousedown=false;
}

void TestControl::mouseWheel(int x, int y, int delta)
{
	float temp = 1.0f;
	if (delta<0)
		temp=-1.0f;
	m_circlesize = bound_value(1.0f, m_circlesize+temp, 100.0f);
	repaint();
}

void TestControl::shift_points(double x, double y)
{
	for (auto& e : m_points)
	{
		e.m_x += x;
		e.m_y += y;
	}
}

std::vector<point> g_points_clipboard;

bool TestControl::keyPressed(const ModifierKeys& modkeys, int keycode)
{
	if (keycode >= KEY_F1 && keycode <= KEY_F12)
	{
		readbg() << "F" << (keycode - KEY_F1)+1 << " pressed\n";
	}
	if (keycode == 'C' && modkeys.isModifierKeyDown(MKControl) == true)
	{
		g_points_clipboard = m_points;
	}
	if (keycode == 'V' && modkeys.isModifierKeyDown(MKControl) == true)
	{
		for (auto& e : g_points_clipboard)
			m_points.push_back(e);
	}
	if (keycode == KEY_BACKSPACE)
	{
		if (modkeys.isModifierKeyDown(MKAlt) == true)
		{
			m_points.clear();
			m_hot_point = -1;
		}
		else
		{
			if (m_hot_point >= 0)
			{
				m_points.erase(m_points.begin() + m_hot_point);
				m_hot_point = -1;
			}
		}
	}
	if (modkeys.noneDown() == true)
	{
		if (keycode == KEY_LEFT)
			shift_points(-1.0, 0.0);
		if (keycode == KEY_RIGHT)
			shift_points(1.0, 0.0);
		if (keycode == KEY_UP)
			shift_points(0.0, -1.0);
		if (keycode == KEY_DOWN)
			shift_points(0.0, 1.0);
		repaint();
		return false;
	}
	if (modkeys.isModifierKeyDown(MKControl) == true && (keycode==KEY_LEFT || keycode==KEY_RIGHT))
	{
		int step = 1;
		if (keycode == KEY_LEFT)
			step = -1;
		int newhot = m_hot_point + step;
		if (newhot == m_points.size())
			newhot = 0;
		if (newhot < 0)
			newhot = m_points.size() - 1;
		m_hot_point = newhot;
	}
	
	repaint();
	return false;
}

fx_param_t * TestControl::getFXParamTarget(int index, int which)
{
	if (index >= 0 && index < m_points.size())
	{
		if (which == 0)
			return &m_points[index].m_x_target;
		if (which == 1)
			return &m_points[index].m_y_target;
	}
	return nullptr;
}

WaveformControl::WaveformControl(MRPWindow* parent) : LiceControl(parent)
{
	setWantsFocus(true);
	m_font.SetFromHFont(CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void WaveformControl::paint(PaintEvent& ev)
{
	LICE_IBitmap* bm = ev.bm;
	if (m_src!=nullptr)
	{
		if (m_src->IsAvailable()==false)
		{
			LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
			MRP_DrawTextHelper(bm, &m_font, "Source offline", 2, 2, bm->getWidth(), 30);
			return;
		}
		// Somewhat inefficient to do all this here on each paint, but will suffice for now
		m_minpeaks.resize(bm->getWidth()*m_src->GetNumChannels());
		m_maxpeaks.resize(bm->getWidth()*m_src->GetNumChannels());
		PCM_source_peaktransfer_t peaktrans = {0};
		peaktrans.nchpeaks=m_src->GetNumChannels();
		peaktrans.samplerate=m_src->GetSampleRate();
		peaktrans.start_time=m_view_start;
		peaktrans.peaks=m_maxpeaks.data();
		peaktrans.peaks_minvals=m_minpeaks.data();
		peaktrans.peaks_minvals_used=1;
		peaktrans.numpeak_points=bm->getWidth();
		peaktrans.peakrate=(double)bm->getWidth()/(m_view_end-m_view_start);
		m_src->GetPeakInfo(&peaktrans);
		if (m_use_reaper_peaks_drawing == true)
		{
			GetPeaksBitmap(&peaktrans, m_peaks_gain, bm->getWidth(), bm->getHeight(), bm);
		}
		else
		{
			if (m_src->GetNumChannels() > 0)
			{
				LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
				int nch = m_src->GetNumChannels();
				double peakshei = (double)getHeight() / nch;
				double peakshalfhei = peakshei / 2;
				double yoffs = peakshalfhei;
				for (int j = 0; j < nch; ++j)
				{
					for (int i = 0; i < peaktrans.numpeak_points; ++i)
					{
						double ycor0 = peakshalfhei * m_maxpeaks[i*nch+j];
						double ycor1 = peakshalfhei * m_minpeaks[i*nch+j];
						if (m_maxpeaks[i] > 0.5 || m_minpeaks[i] < -0.5)
							LICE_Line(bm, i, yoffs-ycor0, i, yoffs-ycor1, LICE_RGBA(255, 0, 0, 255), 1.0f, 0, true);
						else
							LICE_Line(bm, i, yoffs-ycor0, i, yoffs-ycor1, LICE_RGBA(0, 255, 0, 255), 1.0f, 0, true);
					}
					yoffs += peakshei;
				}
			}
		}
		double sel_x0 = map_value(m_sel_start, m_view_start, m_view_end, 0.0, (double)getWidth());
		double sel_x1 = map_value(m_sel_end, m_view_start, m_view_end, 0.0, (double)getWidth());
		LICE_FillRect(bm, sel_x0, 0, sel_x1 - sel_x0, getHeight(), LICE_RGBA(255, 255, 255, 255), 0.5f, 0);
	} else
	{
		LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
		MRP_DrawTextHelper(bm, &m_font, "Source null", 2, 2, bm->getWidth(), 30);
	}
}

void WaveformControl::mousePressed(const MouseEvent & ev)
{
	if (m_src == nullptr)
		return;
	m_mouse_down = true;
	m_drag_start_x = ev.m_x;
}

void WaveformControl::mouseMoved(const MouseEvent & ev)
{
	if (m_src == nullptr)
		return;
	if (m_mouse_down == true)
	{
		if (m_hot_sel_edge == 0)
		{
			double t0 = map_value((double)ev.m_x, (double)0.0, (double)getWidth(), m_view_start, m_view_end);
			double t1 = map_value((double)m_drag_start_x, (double)0.0, (double)getWidth(), m_view_start, m_view_end);
			m_sel_start = t0;
			m_sel_end = t1;
		}
		if (m_hot_sel_edge == -1)
		{
			double t0 = map_value((double)ev.m_x, (double)0.0, (double)getWidth(), m_view_start, m_view_end);
			m_sel_start = t0;
		}
		if (m_hot_sel_edge == 1)
		{
			double t0 = map_value((double)ev.m_x, (double)0.0, (double)getWidth(), m_view_start, m_view_end);
			m_sel_end = t0;
		}
		if (m_sel_end < m_sel_start)
		{
			std::swap(m_sel_start, m_sel_end);
			if (m_hot_sel_edge == -1)
				m_hot_sel_edge = 1;
			else if (m_hot_sel_edge == 1)
				m_hot_sel_edge = -1;
		}
		if (GenericNotifyCallback) GenericNotifyCallback(GenericNotifications::TimeRange);
		repaint();
		//readbg() << "sel is " << t0 << " " << t1 << "\n";
	}
	else
	{
		m_hot_sel_edge = get_hot_time_sel_edge(ev.m_x, ev.m_y);
		if (m_hot_sel_edge != 0)
		{
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		} else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
}

void WaveformControl::mouseReleased(const MouseEvent & ev)
{
	if (m_src == nullptr)
		return;
	m_mouse_down = false;
	m_hot_sel_edge = 0;
}

void WaveformControl::mouseDoubleClicked(const MouseEvent& ev)
{
	if (CountSelectedMediaItems(nullptr)>0)
	{
		MediaItem* item = GetSelectedMediaItem(nullptr,0);
		MediaItem_Take* take = GetActiveTake(item);
		if (take!=nullptr)
		{
			setSource(GetMediaItemTake_Source(take));
			if (GenericNotifyCallback) GenericNotifyCallback(GenericNotifications::Contents);
		}
	}
}

bool WaveformControl::keyPressed(const ModifierKeys& mods, int code)
{
	if (code>='0' && code<='9')
	{
		int index = code-'0';
		if (index == 0)
			index = 9;
		else --index;
		if (CountSelectedMediaItems(nullptr)>index)
		{
			MediaItem* item = GetSelectedMediaItem(nullptr,index);
			MediaItem_Take* take = GetActiveTake(item);
			if (take!=nullptr)
			{
				setSource(GetMediaItemTake_Source(take));
				if (GenericNotifyCallback) GenericNotifyCallback(GenericNotifications::Contents);
			}
		}
	}
	return false;
}

void WaveformControl::setSource(PCM_source* src)
{
	if (src==nullptr)
	{
		m_src=nullptr;
		repaint();
		return;
	}
	m_src=std::shared_ptr<PCM_source>(src->Duplicate());
	// Pretty bad to do it like this, blocking the GUI thread...
	// OTOH these days with SSDs and fast processors, maybe it usually doesn't take a long time
	if (m_src->PeaksBuild_Begin()!=0) // should build peaks
	{
		while (true)
		{
			if (m_src->PeaksBuild_Run()==0)
				break;
		}
		m_src->PeaksBuild_Finish();
	}
	m_view_start = 0.0;
	m_view_end = m_src->GetLength();
	repaint();
}

void WaveformControl::setAudioAccessor(std::shared_ptr<AudioAccessor> accessor)
{
	if (accessor != nullptr)
		m_src = nullptr;
	m_accessor = accessor;
	double time = GetAudioAccessorStartTime(m_accessor.get());
	double endtime = GetAudioAccessorEndTime(m_accessor.get());
	int blocksize = 16384;
	int sr = 44100;
	int numchans = 2;
	std::vector<double> buffer(blocksize*numchans);
	while (time < endtime)
	{
		GetAudioAccessorSamples(m_accessor.get(), sr, numchans, time, blocksize, buffer.data());
		time += (double)blocksize / sr;
	}
}

void WaveformControl::setFloatingPointProperty(int which, double val)
{
	if (which == 0)
	{
		m_peaks_gain = bound_value(0.01, val, 8.0);
	}
	if (m_src != nullptr)
	{
		if (which == 1)
			m_view_start = bound_value(0.0, val, m_view_end - 0.01);
		if (which == 2)
			m_view_end = bound_value(m_view_start + 0.01, val, m_src->GetLength());
		if (which == 3)
			m_sel_start = bound_value(0.0, val, m_src->GetLength());
		if (which == 4)
			m_sel_end = bound_value(0.0, val, m_src->GetLength());
		if (m_sel_start > m_sel_end)
			std::swap(m_sel_start, m_sel_end);
		if (which == 64)
		{
			if (val < 0.5)
				m_use_reaper_peaks_drawing = true;
			else m_use_reaper_peaks_drawing = false;
		}
	}
	repaint();
}



int WaveformControl::get_hot_time_sel_edge(int x, int y)
{
	int sel_x0 = map_value(m_sel_start, m_view_start, m_view_end, 0.0, (double)getWidth());
	int sel_x1 = map_value(m_sel_end, m_view_start, m_view_end, 0.0, (double)getWidth());
	if (is_point_in_rect(x, y, sel_x0 - 10, 0, 20, getHeight()) == true)
		return -1;
	if (is_point_in_rect(x, y, sel_x1 - 10, 0, 20, getHeight()) == true)
		return 1;
	return 0;
}

double WaveformControl::getFloatingPointProperty(int which)
{
	if (which == 0)
		return m_peaks_gain;
	if (which == 1)
		return m_view_start;
	if (which == 2)
		return m_view_end;
	if (which == 3)
		return m_sel_start;
	if (which == 4)
		return m_sel_end;
	if (which == 100 && m_src != nullptr)
		return m_src->GetLength();
	return 0.0;
}

EnvelopeControl::EnvelopeControl(MRPWindow* parent) : LiceControl(parent)
{
	m_font.SetFromHFont(CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void EnvelopeControl::paint(PaintEvent& ev)
{
	LICE_IBitmap* bm = ev.bm;
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	if (m_envs.empty() == true)
	{
		MRP_DrawTextHelper(bm, &m_font, "No envelope", 5, 5, bm->getWidth(), bm->getHeight());
		return;
	}
	if (m_wave_painter != nullptr)
	{
		if (m_wave_painter->getSource() != nullptr)
		{
			double srclen = m_wave_painter->getSource()->GetLength();
			m_wave_painter->paint(bm, srclen*m_view_start_time, srclen*m_view_end_time,
				0, 0, bm->getWidth(), bm->getHeight());
		}
		
	}
	int envnameycor = 5;
	if (m_text.empty() == false)
	{
		MRP_DrawTextHelper(bm, &m_font, m_text.c_str(), 5, 5, bm->getWidth(), 25);
		envnameycor = 30;
	}
	if (m_active_envelope >= 0)
	{
		std::string envname = m_envs[m_active_envelope]->getName();
		MRP_DrawTextHelper(bm, &m_font, envname.c_str(), 5, envnameycor, bm->getWidth(), 25);
	}
	const float linethickness = 0.75f;
	const int subsegments = 30;
	for (int j = 0; j < m_envs.size(); ++j)
	{
		breakpoint_envelope* m_env = m_envs[j].get();
		int envlinecolor = LICE_RGBA(255, 255, 255, 255);
		if (j != m_active_envelope)
			envlinecolor = LICE_RGBA(128, 128, 128, 255);
		for (int i = 0; i < m_env->get_num_points(); ++i)
		{
			const envbreakpoint& pt = m_env->get_point(i);
			double xcor = map_value(pt.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
			double ycor = (double)getHeight() - map_value(pt.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
			if (i == m_node_to_drag.second && j == m_node_to_drag.first)
				LICE_FillRect(bm, xcor - 4.0, ycor - 4.0, 8, 8, m_env->getColor());
			else LICE_DrawRect(bm, xcor - 4.0, ycor - 4.0, 8, 8, m_env->getColor());
			envbreakpoint pt1;
			if (i + 1 < m_env->get_num_points())
			{
				pt1 = m_env->get_point(i + 1);
				double xcor1 = map_value(pt1.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
				double ycor1 = (double)getHeight() - map_value(pt1.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
				for (int k = 0; k < subsegments; ++k)
				{
					double normt = 1.0 / subsegments*k;
					double xcoroffset = (xcor1 - xcor)*normt;
					double normv = get_shaped_value(normt, pt.get_shape(), pt.get_param1(), pt.get_param2());
					double ycorrange = ycor1 - ycor;
					double subxcor0 = xcor + xcoroffset;
					double subycor0 = ycor + ycorrange*normv;
					normt = 1.0 / subsegments*(k+1);
					xcoroffset = (xcor1 - xcor)*normt;
					normv = get_shaped_value(normt, pt.get_shape(), pt.get_param1(), pt.get_param2());
					double subxcor1 = xcor + xcoroffset;
					double subycor1 = ycor + ycorrange*normv;
					LICE_FLine(bm, subxcor0, subycor0, subxcor1, subycor1, envlinecolor, 1.0f);
				}
			}
			if (i == 0 && pt.get_x() >= 0.0)
			{
				LICE_FLine(bm, 0.0, ycor, xcor, ycor, envlinecolor, 1.0f);
			}
			if (i == m_env->get_num_points() - 1 && pt.get_x() < 1.0)
			{
				LICE_FLine(bm, xcor, ycor, getWidth(), ycor, envlinecolor, 1.0f);
			}
		}
	}
	if (m_enabled == false)
	{
		LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255), 0.5f);
	}
}

void EnvelopeControl::mousePressed(const MouseEvent& ev)
{
	if (m_envs.empty() == true)
		return;
	if (ev.m_mb == MouseEvent::MBRight)
		return;
	m_mouse_xy_at_press = { ev.m_x,ev.m_y };
	m_point_was_moved = false;
	m_mouse_down = true;
	m_segment_to_adjust = find_hot_envelope_segment(ev.m_x, ev.m_y);
	if (m_segment_to_adjust.second >= 0 && ev.m_modkeys.isModifierKeyDown(ModifierKey::MKAlt) == true)
	{
		m_segment_p1_at_mouse_press = m_envs[m_segment_to_adjust.first]->get_point(m_segment_to_adjust.second).get_param1();
		return;
	}
	m_node_to_drag = find_hot_envelope_point(ev.m_x, ev.m_y);
	if (m_node_to_drag.second == -1)
	{
		double normx = map_value((double)ev.m_x, 0.0, (double)getWidth(), m_view_start_time, m_view_end_time);
		double normy = map_value((double)getHeight() - ev.m_y, 0.0, (double)getHeight(), m_view_start_value, m_view_end_value);
		if (m_active_envelope >= 0)
		{
			m_envs[m_active_envelope]->add_point({ normx,normy,envbreakpoint::Power }, true);
			if (GenericNotifyCallback) 
				GenericNotifyCallback(GenericNotifications::ObjectAdded);
			m_node_to_drag = find_hot_envelope_point(ev.m_x, ev.m_y);
			repaint();
			return;
		}
	}
	if (m_node_to_drag.first>=0 && m_node_to_drag.second >= 0 && ev.m_modkeys.isModifierKeyDown(MKAlt) == true)
	{
		m_envs[m_node_to_drag.first]->remove_point(m_node_to_drag.second);
		m_node_to_drag = { -1,-1 };
		if (GenericNotifyCallback) 
			GenericNotifyCallback(GenericNotifications::ObjectRemoved);
		
		repaint();
	}
}

void EnvelopeControl::mouseMoved(const MouseEvent& ev)
{
	if (m_envs.empty() == true)
		return;
	
	if (m_mouse_down == true)
	{
		if (ev.m_modkeys.isModifierKeyDown(ModifierKey::MKAlt) && m_segment_to_adjust.second >= 0)
		{
			double xdelta = m_mouse_xy_at_press.first-ev.m_x;
			breakpoint_envelope* m_env = m_envs[m_segment_to_adjust.first].get();
			envbreakpoint& pt = m_env->get_point(m_segment_to_adjust.second);
			double newp1 = bound_value(0.0, m_segment_p1_at_mouse_press + xdelta*0.005, 1.0);
			pt.set_param1(newp1);
			m_point_was_moved = true;
			if (m_notify_on_point_move == true && GenericNotifyCallback)
				GenericNotifyCallback(GenericNotifications::ObjectMoved);
			repaint();
			return;
		}
		if (m_node_to_drag.second >= 0)
		{
			breakpoint_envelope* m_env = m_envs[m_node_to_drag.first].get();
			envbreakpoint& pt = m_env->get_point(m_node_to_drag.second);
			double left_bound = m_view_start_time;
			double right_bound = m_view_end_time;
			if (m_node_to_drag.second > 0)
			{
				left_bound = m_env->get_point(m_node_to_drag.second - 1).get_x();
			}
			if (m_node_to_drag.second < m_env->get_num_points() - 1)
			{
				right_bound = m_env->get_point(m_node_to_drag.second + 1).get_x();
			}
			double normx = map_value((double)ev.m_x, 0.0, (double)getWidth(), m_view_start_time, m_view_end_time);
			double normy = map_value((double)getHeight() - ev.m_y, 0.0, (double)getHeight(), m_view_start_value, m_view_end_value);
			pt.set_x(bound_value(left_bound + 0.001, normx, right_bound - 0.001));
			pt.set_y(bound_value(0.0, normy, 1.0));
			if (m_notify_on_point_move == true && GenericNotifyCallback) 
				GenericNotifyCallback(GenericNotifications::ObjectMoved);
			m_point_was_moved = true;
			repaint();
			return;

		}
	}
	else
	{
		m_segment_to_adjust = find_hot_envelope_segment(ev.m_x, ev.m_y);
		//if (m_segment_to_adjust.second >= 0)
		//	readbg() << "cursor at env segment " << m_segment_to_adjust.second << "\n";
		if (m_segment_to_adjust.second < 0)
		{
			auto oldindex = m_node_to_drag;
			m_node_to_drag = find_hot_envelope_point(ev.m_x, ev.m_y);
			if (oldindex != m_node_to_drag)
				repaint();
		}
	}
}

void EnvelopeControl::mouseReleased(const MouseEvent& ev)
{
	if (m_envs.empty() == true)
		return;
	m_mouse_down = false;
	m_node_to_drag = { -1,-1 };
	if (m_point_was_moved == true)
	{
		m_point_was_moved = false;
		if (GenericNotifyCallback) 
			GenericNotifyCallback(GenericNotifications::AfterManipulation);
	}
}

void EnvelopeControl::fitEnvelopeTimeRangesIntoView()
{
	double mintime = std::numeric_limits<double>::max();
	double maxtime = std::numeric_limits<double>::min();
	for (int i = 0; i < m_envs.size(); ++i)
	{
		// Technically the first point of the envelope should be the minimum time
		// and the last point the maximum time, but just in case the points sorting
		// has got messed up or something, we tediously calculate the min and max
		// time by iterating all the points
		for (int j = 0; j < m_envs[i]->get_num_points(); ++j)
		{
			double ptime = m_envs[i]->get_point(j).get_x();
			if (ptime > maxtime)
				maxtime = ptime;
			if (ptime < mintime)
				mintime = ptime;
		}		
	}
	m_view_start_time = mintime;
	m_view_end_time = maxtime;
	repaint();
}

double EnvelopeControl::getFloatingPointProperty(int index)
{
	if (m_envs.size()>0 && m_active_envelope>=0)
	{
		if (index >= 0 && index < 999)
		{
			if (index < m_envs[m_active_envelope]->get_num_points())
				return m_envs[m_active_envelope]->get_point(index).get_x();
		}
		if (index >= 1000 && index < 1999)
		{
			int offsettedindex = index - 1000;
			if (offsettedindex < m_envs[m_active_envelope]->get_num_points())
				return m_envs[m_active_envelope]->get_point(offsettedindex).get_y();
		}
	}
	return 0.0;
}

void EnvelopeControl::setFloatingPointProperty(int index, double val)
{
	if (m_envs.size()>0 && m_active_envelope >= 0)
	{
		if (index >= 0 && index < 999)
		{
			if (index < m_envs[m_active_envelope]->get_num_points())
				m_envs[m_active_envelope]->get_point(index).set_x(val);
		}
		if (index >= 1000 && index < 1999)
		{
			int offsettedindex = index - 1000;
			if (offsettedindex < m_envs[m_active_envelope]->get_num_points())
				m_envs[m_active_envelope]->get_point(offsettedindex).set_y(val);
		}
		repaint();
	}
}

int EnvelopeControl::getIntegerProperty(int which)
{
	if (which == 0)
		return m_envs.size();
	if (which == 1)
		return m_active_envelope;
	if (which == 2)
		return m_notify_on_point_move;
	if (which >= 100 && which < 199)
	{
		int offsetted = which - 100;
		if (offsetted >= 0 && offsetted < m_envs.size())
			return m_envs[offsetted]->get_num_points();
	}
	if (which >= 200 && which < 299)
	{
		int offsetted = which - 200;
		if (offsetted >= 0 && offsetted < m_envs.size())
			return m_envs[offsetted]->getColor();
	}
	return 0;
}

void EnvelopeControl::setIntegerProperty(int which, int v)
{
	if (which == 1)
	{
		if (m_envs.size() > 0 && v >= 0 && v < m_envs.size())
			m_active_envelope = v;
	}
	if (which == 2)
	{
		if (v == 0)
			m_notify_on_point_move = false;
		else m_notify_on_point_move = true;
		return;
	}
	if (which >= 200 && which < 299)
	{
		int offsetted = which - 200;
		if (offsetted >= 0 && offsetted < m_envs.size())
			m_envs[offsetted]->setColor(LICE_RGBA_FROMNATIVE(v));
	}
	repaint();
}

void EnvelopeControl::setStringProperty(int which, std::string txt)
{
	if (which == 0 && m_envs.size() > 0)
		m_envs[0]->setName(txt);
	repaint();
}

template<typename F>
inline void for_lines_of_string(const std::string& msg, F&& f)
{
	size_t lastlinestart = 0;
	for (size_t i = 0; i < msg.size(); ++i)
	{
		if (msg[i] == '\n' || i == msg.size() - 1)
		{
			std::string line;
			size_t lastindex = i;
			if (i == msg.size() - 1)
				lastindex = i + 1;
			for (size_t j = lastlinestart; j < lastindex; ++j)
				if (j<msg.size())
					line.push_back(msg[j]);
			f(line);
			lastlinestart = i + 1;
		}
	}
}

void EnvelopeControl::sendStringCommand(const std::string & msg)
{
	if (m_envs.size() > 0 && m_active_envelope>=0)
	{
		LineParser lp;
		bool dosort = false;
		for_lines_of_string(msg, [&lp,this,&dosort](const auto& line) 
		{ 
			lp.parse(line.c_str());
			int numtoks = lp.getnumtokens();
			if (numtoks == 1 && strcmp(lp.gettoken_str(0), "CLEAR") == 0)
			{
				m_envs[m_active_envelope]->remove_all_points();
			}
			else if (numtoks == 3 && strcmp(lp.gettoken_str(0), "ADDPT") == 0)
			{
				double ptx = lp.gettoken_float(1);
				double pty = lp.gettoken_float(2);
				m_envs[m_active_envelope]->add_point({ ptx,pty }, false);
				dosort = true;
			}
			else if (numtoks == 3 && strcmp(lp.gettoken_str(0), "DELINTIMERANGE") == 0)
			{
				double t0 = lp.gettoken_float(1);
				double t1 = lp.gettoken_float(2);
				m_envs[m_active_envelope]->remove_points_conditionally([t0, t1](auto& pt)
				{
					return pt.get_x() >= t0 && pt.get_x() < t1;
				});
				
			}
		});
		if (dosort==true)
			m_envs[m_active_envelope]->sort_points();
		repaint();
	}
}

void EnvelopeControl::setEnabled(bool b)
{
	m_enabled = b;
	repaint();
	WinControl::setEnabled(b);
}

std::pair<int, int> EnvelopeControl::find_hot_envelope_point(double xcor, double ycor)
{
	if (m_envs.empty() == true)
		return{ -1,-1 };
	for (int j = m_envs.size()-1; j >= 0 ; --j)
	{
		breakpoint_envelope* m_env = m_envs[j].get();
		for (int i = 0; i < m_env->get_num_points(); ++i)
		{
			const envbreakpoint& pt = m_env->get_point(i);
			double ptxcor = map_value(pt.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
			double ptycor = (double)getHeight() - map_value(pt.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
			if (is_point_in_rect(xcor, ycor, ptxcor - 6, ptycor - 6, 12, 12) == true)
			{
				return{ j,i };
			}
		}
	}
	return{ -1,-1 };
}

std::pair<int, int> EnvelopeControl::find_hot_envelope_segment(double xcor, double ycor)
{
	if (m_envs.empty() == true)
		return{ -1,-1 };
	for (int j = m_envs.size() - 1; j >= 0; --j)
	{
		breakpoint_envelope* m_env = m_envs[j].get();
		for (int i = 0; i < m_env->get_num_points()-1; ++i)
		{
			const envbreakpoint& pt0 = m_env->get_point(i);
			const envbreakpoint& pt1 = m_env->get_point(i+1);
			double ptxcor0 = map_value(pt0.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
			double ptxcor1 = map_value(pt1.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
			
			if (xcor>=ptxcor0+6.0 && xcor<ptxcor1-6.0)
			{
				return{ j,i };
			}
		}
	}
	return{ -1,-1 };
}


void EnvelopeControl::add_envelope(std::shared_ptr<breakpoint_envelope> env)
{
	m_envs.push_back(env);
	m_active_envelope = m_envs.size() - 1;
	repaint();
}

std::shared_ptr<breakpoint_envelope> EnvelopeControl::getEnvelope(int index)
{
	if (index == -1 && m_active_envelope >= 0 && m_active_envelope<m_envs.size())
		return m_envs[m_active_envelope];
	if (index >= 0 && index < m_envs.size())
		return m_envs[index];
	return nullptr;
}

void EnvelopeControl::set_waveformpainter(std::shared_ptr<WaveformPainter> painter)
{
	m_wave_painter = painter;
	repaint();
}

bool PitchBenderEnvelopeControl::keyPressed(const ModifierKeys& modkeys, int keycode)
{
	if (keycode >= '1' && keycode <= '4')
	{
		double startdelta = 0.0;
		double enddelta = 0.0;
		if (keycode == '1')
			startdelta = -0.05;
		if (keycode == '2')
			startdelta = 0.05;
		if (keycode == '3')
			enddelta = -0.05;
		if (keycode == '4')
			enddelta = 0.05;
		m_view_start_time = bound_value(0.0, m_view_start_time + startdelta, m_view_end_time - 0.001);
		m_view_end_time = bound_value(m_view_start_time + 0.001, m_view_end_time + enddelta, 1.0);
		repaint();
	}
	if (keycode == 'N' && m_envs.empty() == false)
	{
		++m_active_envelope;
		if (m_active_envelope == m_envs.size())
			m_active_envelope = 0;
		repaint();
	}
	if (keycode == 'R' && m_envs.empty()==false)
	{
		std::string err = pitch_bend_selected_item(m_envs[0],m_envs[1], m_resampler_mode);
		if (err.empty() == false)
		{
			m_text = std::string("Render error : ")+err;
		}
		else m_text = "Pitch bend OK!";
		repaint();
		return true;
	}
	if (keycode == 'I')
	{
		if (CountSelectedMediaItems(nullptr) > 0)
		{
			MediaItem* item = GetSelectedMediaItem(nullptr, 0);
			MediaItem_Take* take = GetActiveTake(item);
			if (take == nullptr)
				return true;
			PCM_source* src = GetMediaItemTake_Source(take);
			std::string explanation = is_source_audio(src);
			if (explanation.empty() == true)
			{
				if (m_wave_painter == nullptr)
				{
					m_wave_painter = std::make_shared<WaveformPainter>(getWidth());
				}
				m_wave_painter->set_source(src);
				m_text = std::string("Imported media OK (")+src->GetType()+")";
				repaint();
			}
			else
			{
				m_text = std::string("Import error : ") + explanation;
				repaint();
			}
			
		}
		else
		{
			m_text = "No item selected";
			repaint();
		}
		
	}
	return false;
}

void PitchBenderEnvelopeControl::mousePressed(const MouseEvent & ev)
{
	EnvelopeControl::mousePressed(ev);
	if (ev.m_mb == MouseEvent::MBRight)
	{
		PopupMenu popmenu(getWindowHandle());
		popmenu.add_menu_item("Project default", [this](PopupMenu::CheckState) { m_resampler_mode = -1; });
		int i = 0;
		while (true)
		{
			const char* modetext = Resample_EnumModes(i);
			if (modetext == nullptr)
				break;
			popmenu.add_menu_item(modetext, [i, this](PopupMenu::CheckState) { m_resampler_mode = i; });
			++i;
		}
		popmenu.execute(ev.m_x, ev.m_y);
	}
}

std::string pitch_bend_selected_item(std::shared_ptr<breakpoint_envelope> pchenv, 
	std::shared_ptr<breakpoint_envelope> volenv,int rsmode)
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return "No item selected";
	MediaItem* item = GetSelectedMediaItem(nullptr, 0);
	MediaItem_Take* take = GetActiveTake(item);
	if (take == nullptr)
		return "Item has no active take";
	PCM_source* src = GetMediaItemTake_Source(take);
	std::string explanation = is_source_audio(src);
	if (explanation.empty() == false)
		return explanation;
	REAPER_Resample_Interface* m_resampler = m_resampler = Resampler_Create();
	m_resampler->Reset();
	int bufsize = 128;
	int numoutchans = src->GetNumChannels();
	int outsamplerate = src->GetSampleRate();
	std::vector<double> procbuf(bufsize * numoutchans);
	std::vector<double> diskoutbuf(bufsize*numoutchans);
	std::vector<double*> diskoutbufptrs(numoutchans);
	for (int i = 0; i < numoutchans; ++i)
		diskoutbufptrs[i] = &diskoutbuf[i*bufsize];
	char cfg[] = { 'e','v','a','w', 32, 0 };
	static int fncounter = 0;
	char pathbuf[2048];
	GetProjectPath(pathbuf, 2048);
	std::string outfn = std::string(pathbuf)+ "/bendout_" + std::to_string(fncounter) + ".wav";
	++fncounter;
	PCM_sink* sink = PCM_Sink_Create(outfn.c_str(), cfg, sizeof(cfg), numoutchans, outsamplerate, true);
	int mode = rsmode;
	m_resampler->Extended(RESAMPLE_EXT_SETRSMODE, (void*)mode, 0, 0);
	double counter = 0.0;
	while (counter < src->GetLength())
	{
		double normpos = 1.0 / src->GetLength()*counter;
		double semitones = -12.0 + 24.0*pchenv->interpolate(normpos);
		double ratio = 1.0 / pow(1.05946309436, semitones);
		m_resampler->SetRates(src->GetSampleRate(), src->GetSampleRate()*ratio);
		double* resbuf = nullptr;
		int wanted = m_resampler->ResamplePrepare(bufsize, numoutchans, &resbuf);
		PCM_source_transfer_t transfer = { 0 };
		transfer.time_s = counter;
		transfer.length = wanted;
		transfer.nch = numoutchans;
		transfer.samplerate = outsamplerate;
		transfer.samples = resbuf;
		src->GetSamples(&transfer);
		int resampled_out = m_resampler->ResampleOut(procbuf.data(), wanted, bufsize, numoutchans);
		for (int j = 0; j < resampled_out; ++j)
		{ 
			// Interpolating per sample here seems to make things pretty slow on debug builds, 
			// but release builds seem ok...
			double gain = volenv->interpolate(normpos);
			for (int i = 0; i < numoutchans; ++i)
			{
				diskoutbufptrs[i][j] = procbuf[j*numoutchans + i]*gain;
			}
		}
		sink->WriteDoubles(diskoutbufptrs.data(), resampled_out, numoutchans, 0, 1);
		counter += (double)wanted / outsamplerate;
	}
	delete sink;
	delete m_resampler;
	InsertMedia(outfn.c_str(), 3);
	//if (m_adjust_item_length == true)
		Main_OnCommand(40612, 0);
	return std::string();
}

bool WaveformPainter::paint(LICE_IBitmap * bm, double starttime, double endtime, int x, int y, int w, int h)
{
	if (m_src == nullptr)
		return false;
	int nch = m_src->GetNumChannels();
	if (nch < 1)
		return false;
	if (m_minpeaks.size() < w * nch)
	{
		m_minpeaks.resize(w*nch);
		m_maxpeaks.resize(w*nch);
	}
	if (w != m_last_w || starttime != m_last_start || endtime != m_last_end)
	{
		//readbg() << "WaveformPainter " << w << " " << starttime << " " << endtime << "\n";
		m_peaks_transfer.nchpeaks = m_src->GetNumChannels();
		m_peaks_transfer.samplerate = m_src->GetSampleRate();
		m_peaks_transfer.start_time = starttime;
		m_peaks_transfer.peaks = m_maxpeaks.data();
		m_peaks_transfer.peaks_minvals = m_minpeaks.data();
		m_peaks_transfer.peaks_minvals_used = 1;
		m_peaks_transfer.numpeak_points = w;
		m_peaks_transfer.peakrate = (double)w / (endtime - starttime);
		m_src->GetPeakInfo(&m_peaks_transfer);
		m_last_w = w; m_last_start = starttime; m_last_end = endtime;
	}

	GetPeaksBitmap(&m_peaks_transfer, 1.0, w, h, bm);
	return true;
}

EnvelopeGeneratorEnvelopeControl::EnvelopeGeneratorEnvelopeControl(MRPWindow* parent) :
	EnvelopeControl(parent)
{
	auto env = std::make_shared<breakpoint_envelope>("LFO",LICE_RGBA(0,255,0,255));
	env->add_point({ 0.0,1.0 }, false);
	env->add_point({ 0.99,0.0 }, false);
	env->sort_points();
	add_envelope(env);
	GenericNotifyCallback = [this,env](GenericNotifications reason)
	{
		TrackEnvelope* reaenv = GetSelectedEnvelope(nullptr);
		if (reaenv != nullptr)
		{
			bool add_undo = true;
			// We don't want to add Reaper undo entries while dragging the envelope points...
			if (reason == GenericNotifications::ObjectMoved)
				add_undo = false;
			DeleteEnvelopePointRange(reaenv, 0.0, 10.0);
			double last_time = 0.0;
			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < env->get_num_points(); ++j)
				{
					double pt_time = env->get_point(j).get_x();
					double pt_value = env->get_point(j).get_y();
					bool sort = false;
					InsertEnvelopePoint(reaenv, i*1.0+pt_time, pt_value, 0, 0.0, false, &sort);
				}
				
			}
			Envelope_SortPoints(reaenv);
			if (add_undo == true)
				Undo_OnStateChangeEx("Generate envelope points", UNDO_STATE_TRACKCFG, - 1);
			UpdateArrange();
		}
		
	};
}

ZoomScrollBar::ZoomScrollBar(MRPWindow* parent) : LiceControl(parent)
{

}

void ZoomScrollBar::paint(PaintEvent& ev)
{
	LICE_FillRect(ev.bm, 0, 0, getWidth(), getHeight(), LICE_RGBA(100, 100, 100, 255), 1.0f);
	int x0 = getWidth()*m_start;
	int x1 = getWidth()*m_end;
	int thumbcolor = LICE_RGBA(200, 200, 200, 255);
	if (m_hot_area == ha_handle)
		thumbcolor = LICE_RGBA(220, 220, 220, 255);
	LICE_FillRect(ev.bm, x0, 0, x1-x0, getHeight(), thumbcolor, 1.0f);
	if (m_hot_area == ha_left_edge)
		LICE_FillRect(ev.bm, x0, 0, 10, getHeight(), LICE_RGBA(255, 255, 255, 255), 1.0f);
	if (m_hot_area == ha_right_edge)
		LICE_FillRect(ev.bm, x1-10, 0, 10, getHeight(), LICE_RGBA(255, 255, 255, 255), 1.0f);
	
}

void ZoomScrollBar::mousePressed(const MouseEvent & ev)
{
	m_mouse_down = true;
	m_drag_start_x = ev.m_x;
}

void ZoomScrollBar::mouseMoved(const MouseEvent & ev)
{
	if (m_mouse_down == false)
	{
		auto temp = get_hot_area(ev.m_x, ev.m_y);
		if (temp != m_hot_area)
		{
			m_hot_area = temp;
			repaint();
		}
	}
	else
	{
		if (m_hot_area == ha_left_edge)
		{
			double new_left_edge = 1.0 / getWidth()*ev.m_x;
			m_start = bound_value(0.0, new_left_edge, m_end - 0.01);
			repaint();
		}
		if (m_hot_area == ha_right_edge)
		{
			double new_right_edge = 1.0 / getWidth()*ev.m_x;
			m_end = bound_value(m_start + 0.01, new_right_edge, 1.0);
			repaint();
		}
		if (m_hot_area == ha_handle)
		{
			double delta = 1.0 / getWidth()*(ev.m_x - m_drag_start_x);
			double old_start = m_start;
			double old_end = m_end;
			double old_len = m_end - m_start;
			m_start = bound_value(0.0, m_start + delta, 1.0 - old_len);
			m_end = bound_value(old_len, m_end + delta, m_start + old_len);
			m_drag_start_x = ev.m_x;
			repaint();
		}
		if (RangeChangedCallback)
			RangeChangedCallback(m_start, m_end);
		if (GenericNotifyCallback)
			GenericNotifyCallback(GenericNotifications::TimeRange);
	}
}

void ZoomScrollBar::mouseReleased(const MouseEvent & ev)
{
	m_mouse_down = false;
}

void ZoomScrollBar::mouseLeave()
{
	if (m_mouse_down == false)
	{
		m_hot_area = ha_none;
		repaint();
	}
	//readbg() << "the mouse has left ZoomScrollBar " << this << "\n";
}

void ZoomScrollBar::onRefreshTimer()
{
	if (isCursorOver()==false)
	{
		if (m_hot_area != ha_none)
		{
			m_hot_area = ha_none;
			repaint();
		}
	}
}

double ZoomScrollBar::getFloatingPointProperty(int index)
{
	if (index == 0)
		return m_start;
	if (index == 1)
		return m_end;
	return 0.0;
}

void ZoomScrollBar::setFloatingPointProperty(int index, double v)
{
	if (index == 0)
		m_start = bound_value(0.0, v, 1.0);
	if (index == 1)
		m_end = bound_value(0.0, v, 1.0);
	if (m_start > m_end)
		std::swap(m_start, m_end);
	repaint();
}

ZoomScrollBar::hot_area ZoomScrollBar::get_hot_area(int x, int y)
{
	int x0 = getWidth()*m_start;
	int x1 = getWidth()*m_end;
	if (is_in_range(x, x0 - 5, x0 + 5))
		return ha_left_edge;
	if (is_in_range(x, x1 - 5, x1 + 5))
		return ha_right_edge;
	if (is_in_range(x, x0 + 5, x1 - 5))
		return ha_handle;
	return ha_none;

}

ProgressControl::ProgressControl(MRPWindow * parent) : LiceControl(parent)
{
	m_font.SetFromHFont(CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
	m_timer.set_callback([this]() { repaint(); });
	m_timer.start(100);
}

void ProgressControl::paint(PaintEvent & ev)
{
	LICE_FillRect(ev.bm, 0, 0, ev.bm->getWidth(), ev.bm->getHeight(), 0);
	double prog = m_progress_val.load();
	int progw = ev.bm->getWidth() * prog;
	LICE_FillRect(ev.bm, 0, 0, progw, ev.bm->getHeight(), LICE_RGBA(0,200,200,255));
	char buf[100];
	sprintf(buf, "%d %%", (int)(prog*100.0));
	MRP_DrawTextHelper(ev.bm, &m_font, buf, 2, 2, ev.bm->getWidth(), ev.bm->getHeight(), DT_VCENTER |DT_CENTER);
}

void ProgressControl::setProgressValue(double v)
{
	v = bound_value(0.0, v, 1.0);
	m_progress_val.store(v);
}
