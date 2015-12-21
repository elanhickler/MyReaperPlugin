
#include "MyLiceWindow.h"
#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

bool g_popupmenushowing = false;

TestControl::TestControl(HWND parent, bool delwhendraggedoutside) :
	LiceControl(parent), m_delete_point_when_dragged_outside(delwhendraggedoutside) 
{
	m_font.SetFromHFont(CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void TestControl::paint(LICE_IBitmap * bm)
{
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
		menu.add_menu_item("First action", []() { readbg() << "first action chosen\n"; });
		if (m_hot_point >= 0)
		{
			menu.add_menu_item("Control last touched parameter with X position", [this]()
			{
				update_touched_fx(m_points[m_hot_point].m_x_target);
			});
			menu.add_menu_item("Control last touched parameter with Y position", [this]()
			{
				update_touched_fx(m_points[m_hot_point].m_y_target);
			});
			menu.add_menu_item("Remove point", [this]()
			{
				m_points.erase(m_points.begin() + m_hot_point);
				m_hot_point = -1;
				repaint();
			});
			
		}
		for (int i = 0; i < 10; ++i)
		{
			menu.add_menu_item(std::to_string(i + 1), [i]()
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

WaveformControl::WaveformControl(HWND parent) : LiceControl(parent)
{
	setWantsFocus(true);
	m_font.SetFromHFont(CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void WaveformControl::paint(LICE_IBitmap* bm)
{
	if (m_src!=nullptr)
	{
		if (m_src->IsAvailable()==false)
		{
			LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
			{
				RECT r;
				r.left = 0;
				r.right = bm->getWidth()-10;
				r.top = 0;
				r.bottom = bm->getHeight();
#ifdef WIN32
				m_font.DrawTextA(bm, "Source OFFLINE", -1, &r, DT_CENTER| DT_NOCLIP);
#else
				m_font.DrawText(bm, "Source OFFLINE", -1, &r, DT_CENTER| DT_NOCLIP);
#endif
				//LICE_DrawText(bm, 25, 25, "SOURCE OFFLINE", LICE_RGBA(255, 255, 255, 255), 1.0f, 0);
			}
			
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
		{
			RECT r;
			r.left = 0;
			r.right = bm->getWidth();
			r.top = 20;
			r.bottom = 45;
#ifdef WIN32
			m_font.DrawTextA(bm, "Source NULL", -1, &r, DT_TOP|DT_LEFT);
#else
			m_font.DrawText(bm, "Source NULL", -1, &r, DT_TOP|DT_LEFT);
#endif
			//LICE_DrawText(bm, 25, 25, "SOURCE NULL", LICE_RGBA(255, 255, 255, 255), 1.0f, 0);
		}
		
	}
}

void WaveformControl::mousePressed(const MouseEvent & ev)
{
	m_mouse_down = true;
	m_drag_start_x = ev.m_x;
}

void WaveformControl::mouseMoved(const MouseEvent & ev)
{
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
		if (ChangeNotifyCallback) ChangeNotifyCallback("Changed time selection");
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
			if (ChangeNotifyCallback) ChangeNotifyCallback("Changed source");
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
				if (ChangeNotifyCallback) ChangeNotifyCallback("Changed source");
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

EnvelopeControl::EnvelopeControl(HWND parent) : LiceControl(parent) 
{
	m_font.SetFromHFont(CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"));
	m_font.SetTextColor(LICE_RGBA(255, 255, 255, 255));
}

void MRP_DrawTextHelper(LICE_IBitmap* bm, LICE_CachedFont* font, std::string txt, int x, int y, int w, int h)
{
	RECT r;
	r.left = x;
	r.right = x+w;
	r.top = y;
	r.bottom = y+h;
#ifdef WIN32
	font->DrawTextA(bm, txt.c_str(), -1, &r, DT_TOP | DT_LEFT);
#else
	font->DrawText(bm, txt.c_str(), -1, &r, DT_TOP | DT_LEFT);
#endif
}

void EnvelopeControl::paint(LICE_IBitmap* bm)
{
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	if (m_env == nullptr)
	{
		MRP_DrawTextHelper(bm, &m_font, "No envelope", 5, 5, bm->getWidth(), bm->getHeight());
		return;
	}
	/*
	if (m_env.unique()==true)
	{
		MRP_DrawTextHelper(bm, &m_font, "Envelope is orphaned (may be a bug)", 5, 5, bm->getWidth(), bm->getHeight());
		return;
	}
	*/
	if (m_text.empty() == false)
	{
		MRP_DrawTextHelper(bm, &m_font, m_text.c_str(), 5, 5, bm->getWidth(), bm->getHeight());
	}
	const float linethickness = 0.75f;
	for (int i = 0; i < m_env->get_num_points(); ++i)
	{
		const envbreakpoint& pt = m_env->get_point(i);
		double xcor = map_value(pt.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
		double ycor = (double)getHeight() - map_value(pt.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
		//if (pt.get_status() == 0)
		//	g.drawRect((float)xcor - 4.0, (float)ycor - 4.0, 8.0f, 8.0f, 1.0f);
		//else g.fillRect((float)xcor - 4.0, (float)ycor - 4.0, 8.0f, 8.0f);
		if (i==m_node_to_drag)
			LICE_DrawRect(bm, xcor - 4.0, ycor - 4.0, 8, 8, LICE_RGBA(255, 0, 0, 255));
		else LICE_DrawRect(bm, xcor - 4.0, ycor - 4.0, 8, 8, LICE_RGBA(0, 255, 0, 255));
		envbreakpoint pt1;
		if (i + 1 < m_env->get_num_points())
		{
			pt1 = m_env->get_point(i + 1);
			double xcor1 = map_value(pt1.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
			double ycor1 = (double)getHeight() - map_value(pt1.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
			LICE_Line(bm, xcor, ycor, xcor1, ycor1, LICE_RGBA(255, 255, 255, 255), 1.0f);
		}
		if (i == 0 && pt.get_x() >= 0.0)
		{
			LICE_Line(bm, 0.0, ycor, xcor, ycor, LICE_RGBA(255, 255, 255, 255), 1.0f);
		}
		if (i == m_env->get_num_points() - 1 && pt.get_x() < 1.0)
		{
			LICE_Line(bm, xcor, ycor, getWidth(), ycor, LICE_RGBA(255, 255, 255, 255),1.0f);
		}
	}

}

void EnvelopeControl::mousePressed(const MouseEvent& ev)
{
	if (m_env == nullptr)
		return;
	m_mouse_down = true;
	m_node_to_drag = find_hot_envelope_point(ev.m_x, ev.m_y);
	if (m_node_to_drag == -1)
	{
		double normx = map_value((double)ev.m_x, 0.0, (double)getWidth(), m_view_start_time, m_view_end_time);
		double normy = map_value((double)getHeight() - ev.m_y, 0.0, (double)getHeight(), m_view_start_value, m_view_end_value);
		m_env->add_point({ normx,normy }, true);
		m_mouse_down = false;
		repaint();
	}

}

void EnvelopeControl::mouseMoved(const MouseEvent& ev)
{
	if (m_env == nullptr)
		return;
	
	if (m_mouse_down == true)
	{
		if (m_node_to_drag >= 0)
		{
			envbreakpoint& pt = m_env->get_point(m_node_to_drag);
			double left_bound = m_view_start_time;
			double right_bound = m_view_end_time;
			if (m_node_to_drag > 0)
			{
				left_bound = m_env->get_point(m_node_to_drag - 1).get_x();
			}
			if (m_node_to_drag < m_env->get_num_points() - 1)
			{
				right_bound = m_env->get_point(m_node_to_drag + 1).get_x();
			}
			double normx = map_value((double)ev.m_x, 0.0, (double)getWidth(), m_view_start_time, m_view_end_time);
			double normy = map_value((double)getHeight() - ev.m_y, 0.0, (double)getHeight(), m_view_start_value, m_view_end_value);
			pt.set_x(bound_value(left_bound + 0.001, normx, right_bound - 0.001));
			pt.set_y(bound_value(0.0, normy, 1.0));
			//m_node_that_was_dragged = m_node_to_drag;
			repaint();
			return;

		}
	}
	else
	{
		int oldindex = m_node_to_drag;
		m_node_to_drag = find_hot_envelope_point(ev.m_x, ev.m_y);
		if (oldindex != m_node_to_drag)
			repaint();
	}
}

void EnvelopeControl::mouseReleased(const MouseEvent& ev)
{
	if (m_env == nullptr)
		return;
	m_mouse_down = false;
	m_node_to_drag = -1;
}

int EnvelopeControl::find_hot_envelope_point(double xcor, double ycor)
{
	if (m_env == nullptr)
		return -1;
	for (int i = 0; i < m_env->get_num_points(); ++i)
	{
		const envbreakpoint& pt = m_env->get_point(i);
		double ptxcor = map_value(pt.get_x(), m_view_start_time, m_view_end_time, 0.0, (double)getWidth());
		double ptycor = (double)getHeight() - map_value(pt.get_y(), m_view_start_value, m_view_end_value, 0.0, (double)getHeight());
		if (is_point_in_rect(xcor, ycor, ptxcor - 4, ptycor - 4, 8, 8) == true)
		{
			return i;
		}
	}
	return -1;
}


void EnvelopeControl::set_envelope(std::shared_ptr<breakpoint_envelope> env)
{
	m_env = env;
	repaint();
}

bool EnvelopeControl::keyPressed(const ModifierKeys& modkeys, int keycode)
{
	if (keycode == 'R' && m_env!=nullptr)
	{
		std::string err = pitch_bend_selected_item(m_env);
		if (err.empty() == false)
		{
			m_text = err;
		}
		else m_text = "Pitch bend OK!";
		repaint();
		return true;
	}
	return false;
}

std::string pitch_bend_selected_item(std::shared_ptr<breakpoint_envelope> env)
{
	if (CountSelectedMediaItems(nullptr) == 0)
		return "No item selected";
	MediaItem* item = GetSelectedMediaItem(nullptr, 0);
	MediaItem_Take* take = GetActiveTake(item);
	if (take == nullptr)
		return "Item has no active take";
	PCM_source* src = GetMediaItemTake_Source(take);
	if (src == nullptr)
		return "Take has no media source";
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
	//int mode = m_resampler_mode;
	//m_resampler->Extended(RESAMPLE_EXT_SETRSMODE, (void*)mode, 0, 0);
	double counter = 0.0;
	while (counter < src->GetLength())
	{
		double normpos = 1.0 / src->GetLength()*counter;
		double semitones = -12.0 + 24.0*env->interpolate(normpos);
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
		for (int i = 0; i < numoutchans; ++i)
		{
			for (int j = 0; j < resampled_out; ++j)
			{
				diskoutbufptrs[i][j] = procbuf[j*numoutchans + i];
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
