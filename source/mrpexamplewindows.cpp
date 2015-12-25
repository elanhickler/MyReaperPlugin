#include "mrpexamplewindows.h"

SimpleExampleWindow* g_simple_example_window=nullptr;

HWND toggle_simple_example_window(HWND parent)
{
	if (g_simple_example_window == nullptr)
	{
		g_simple_example_window = new SimpleExampleWindow(parent,"MRP simple window");
		// Currently, if this isn't set to true, crash on Reaper quit
		g_simple_example_window->setDestroyOnClose(true);
	}
	g_simple_example_window->setVisible(true);
	return g_simple_example_window->getWindowHandle();
}

SimpleExampleWindow::SimpleExampleWindow(HWND parent, std::string title) : MRPWindow(parent,title)
{
	m_but1 = std::make_shared<WinButton>(this, "Get take name");
	m_but1->GenericNotifyCallback = [this](GenericNotifications)
	{
		if (CountSelectedMediaItems(nullptr) > 0)
		{
			MediaItem* item = GetSelectedMediaItem(nullptr, 0);
			MediaItem_Take* take = GetActiveTake(item);
			if (take != nullptr)
			{
				char buf[2048];
				GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false);
				m_edit1->setText(buf);
			}
		}
	};
	add_control(m_but1);
	m_but2 = std::make_shared<WinButton>(this, "Set take name");
	m_but2->GenericNotifyCallback = [this](GenericNotifications)
	{
		if (CountSelectedMediaItems(nullptr) > 0)
		{
			MediaItem* item = GetSelectedMediaItem(nullptr, 0);
			MediaItem_Take* take = GetActiveTake(item);
			if (take != nullptr)
			{
				GetSetMediaItemTakeInfo_String(take, "P_NAME", (char*)m_edit1->getText().c_str(), true);
				UpdateArrange();
			}
		}
	};
	add_control(m_but2);
	m_edit1 = std::make_shared<WinLineEdit>(this, "No take name yet");
	add_control(m_edit1);
	setSize(300, 90);
}

void SimpleExampleWindow::resized()
{
	MRP::Size sz = getSize();
	m_edit1->setBounds({ 5,5,sz.getWidth() - 10,20 });
	m_but1->setBounds({ 5,sz.getHeight() - 25,100,20 });
	m_but2->setBounds({ sz.getWidth()-105,sz.getHeight() - 25,100,20 });
}

SliderBankWindow::SliderBankWindow(HWND parent) : MRPWindow(parent,"MRP Slider bank")
{
	int numsliders = 16;
	for (int i = 0; i < numsliders; ++i)
	{
		slider_controls entry;
		entry.m_slider = std::make_shared<ReaSlider>(this,1.0/numsliders*i);
		entry.m_slider->SliderValueCallback=[this,i](GenericNotifications reason, double v)
		{
			if (reason==GenericNotifications::AfterManipulation)
				on_slider_value_changed(i, v);
		};
		entry.m_label = std::make_shared<WinLabel>(this, std::string("Slider " + std::to_string(i + 1)));
		entry.m_editbox = std::make_shared<WinLineEdit>(this, "foo");
		add_control(entry.m_slider);
		add_control(entry.m_editbox);
		add_control(entry.m_label);
		m_sliders.push_back(entry);
	}
	setSize(500, numsliders*25+40);
}

void SliderBankWindow::resized()
{
	MRP::Size sz = getSize();
	int labw = 90;
	int editw = 60;
	for (int i = 0; i < m_sliders.size(); ++i)
	{
		m_sliders[i].m_label->setBounds({ 5,5 + i * 25, labw , 20 });
		m_sliders[i].m_slider->setBounds({ labw+10, 5 + i * 25, sz.getWidth() - labw - editw - 20 , 20 });
		int slidright = m_sliders[i].m_slider->getBounds().getRight();
		m_sliders[i].m_editbox->setBounds({ labw + slidright + 15 ,5 + i * 25,editw, 20 });
	}
}

void SliderBankWindow::on_slider_value_changed(int slidindex, double v)
{
	readbg() << slidindex << " moved to " << v << "\n";
}

SliderBankWindow* g_sliderbankwindow = nullptr;

HWND toggle_sliderbank_window(HWND parent)
{
	if (g_sliderbankwindow == nullptr)
	{
		g_sliderbankwindow = new SliderBankWindow(parent);
		// Currently, if this isn't set to true, crash on Reaper quit
		g_sliderbankwindow->setDestroyOnClose(true);
	}
	g_sliderbankwindow->setVisible(true);
	return g_sliderbankwindow->getWindowHandle();
}

#ifdef FOOFOOZ
// keep this around for slightly longer
INT_PTR CALLBACK xycontroldlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg, "Multi XY control");
		g_xycontrol = std::make_unique<TestControl>(hwndDlg);
		g_xycontrol->PointMovedCallback = [ptr = g_xycontrol.get()](int ptindex, double x, double y)
		{
			fx_param_t* x_target = ptr->getFXParamTarget(ptindex, 0);
			fx_param_t* y_target = ptr->getFXParamTarget(ptindex, 1);
			if (x_target != nullptr && y_target != nullptr)
			{
				if (x_target->tracknum >= 1)
					TrackFX_SetParamNormalized(GetTrack(nullptr, x_target->tracknum - 1),
						x_target->fxnum, x_target->paramnum, x);
				if (y_target->tracknum >= 1)
					TrackFX_SetParamNormalized(GetTrack(nullptr, y_target->tracknum - 1),
						y_target->fxnum, y_target->paramnum, y);
			}
		};
		ShowWindow(hwndDlg, SW_SHOW);
		return TRUE;
	}
	if (uMsg == WM_CLOSE)
	{
		ShowWindow(hwndDlg, SW_HIDE);
		return TRUE;
	}
	if (uMsg == WM_SIZE)
	{
		RECT r;
		GetClientRect(hwndDlg, &r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;
		g_xycontrol->setBounds(0, 0, w, h);
		InvalidateRect(hwndDlg, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}
#endif