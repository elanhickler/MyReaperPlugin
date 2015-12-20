
#include "MyLiceWindow.h"
#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

bool g_popupmenushowing = false;

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
	if (m_test_text.size()>0)
		LICE_DrawText(bm, 5, 5, m_test_text.c_str(), LICE_RGBA(255, 255, 255, 255), 1.0f, 0);
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
}

void WaveformControl::paint(LICE_IBitmap* bm)
{
	if (m_src!=nullptr)
	{
		if (m_src->IsAvailable()==false)
		{
			LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
			LICE_DrawText(bm, 25, 25, "SOURCE OFFLINE", LICE_RGBA(255,255,255,255), 1.0f, 0);
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
		GetPeaksBitmap(&peaktrans, m_peaks_gain,bm->getWidth(),bm->getHeight(),bm);
		double sel_x0 = map_value(m_sel_start, m_view_start, m_view_end, 0.0, (double)getWidth());
		double sel_x1 = map_value(m_sel_end, m_view_start, m_view_end, 0.0, (double)getWidth());
		LICE_FillRect(bm, sel_x0, 0, sel_x1 - sel_x0, getHeight(), LICE_RGBA(255, 255, 255, 255), 0.5f, 0);
	} else
	{
		LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
		LICE_DrawText(bm, 25, 25, "SOURCE NULL", LICE_RGBA(255,255,255,255), 1.0f, 0);
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
		repaint();
		//readbg() << "sel is " << t0 << " " << t1 << "\n";
	}
	else
	{
		m_hot_sel_edge = get_hot_time_sel_edge(ev.m_x, ev.m_y);
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
	if (which == 100 && m_src != nullptr)
		return m_src->GetLength();
	return 0.0;
}
