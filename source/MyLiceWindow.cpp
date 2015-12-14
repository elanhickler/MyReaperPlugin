
#include "MyLiceWindow.h"
#include "utilfuncs.h"

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
				update_touched_fx(m_x_target);
			});
			menu.add_menu_item("Control last touched parameter with Y position", [this]()
			{
				update_touched_fx(m_y_target);
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
	if (m_hot_point==-1)
	{
		m_points.push_back({ ev.m_x,ev.m_y });
		m_hot_point=(int)m_points.size()-1;
		repaint();
	}
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

void TestControl::mouseReleased(int x, int y)
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

