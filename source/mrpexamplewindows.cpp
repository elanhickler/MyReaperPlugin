#include "mrpexamplewindows.h"
#include <random>

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
	m_last_project_change_count = GetProjectStateChangeCount(nullptr);
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
	
	m_but3 = std::make_shared<WinButton>(this, "Refresh item list");
	m_but3->GenericNotifyCallback = [this](GenericNotifications)
	{
		populate_listbox();
	};
	add_control(m_but3);
	
	m_but4 = std::make_shared<WinButton>(this, "Rem sel listbox item");
	m_but4->GenericNotifyCallback = [this](GenericNotifications)
	{
		int selindex = m_listbox1->getSelectedIndex();
		if (selindex >= 0)
		{
			m_listbox1->removeItem(selindex);
			m_itemmap.erase(m_listbox1->userIDfromIndex(selindex));
		}
	};
	add_control(m_but4);
	
	m_edit1 = std::make_shared<WinLineEdit>(this, "No take name yet");
	add_control(m_edit1);
	m_listbox1 = std::make_shared<WinListBox>(this);
	
	m_listbox1->SelectedChangedCallback = [this](int index) mutable
	{
		if (index >= 0)
		{
			std::string temp = m_listbox1->getItemText(index);
			readbg() << "you chose " << temp << " from the listbox\n";
			int user_id = m_listbox1->userIDfromIndex(index);
			MediaItem* itemfromlist = m_itemmap[user_id];
			if (ValidatePtr((void*)itemfromlist, "MediaItem*") == true)
			{
				m_edit1->setText(std::string("You chose item with mem address " +
					std::to_string((uint64_t)itemfromlist) + " from the listbox"));
			}
			else m_edit1->setText(("You chose an item from listbox that's no longer valid!"));
		}
		else readbg() << "you managed to choose no item from the listbox\n";
	};
	add_control(m_listbox1);
	add_control(m_listbox1);
	setSize(500, 500);
}

void SimpleExampleWindow::populate_listbox()
{
	int old_index = m_listbox1->getSelectedIndex();
	m_listbox1->clearItems();
	m_itemmap.clear();
	int numitems = CountMediaItems(nullptr);
	
	for (int i=0;i<numitems;++i)
	{
		MediaItem* item = GetMediaItem(nullptr,i);
		MediaItem_Take* take = GetActiveTake(item);
		if (item!=nullptr)
		{
			char namebuf[1024];
			if (GetSetMediaItemTakeInfo_String(take,"P_NAME",namebuf,false))
			{
				m_listbox1->addItem(namebuf, i);
				// Note that the item pointers stored into this map
				// may easily become invalid if the items are removed by the user etc...
				// It doesn't matter in this code as we don't dereference the pointers in any way yet.
				// Note how the validation can be done in the button3 handler lambda!
				m_itemmap[i]=item;
			}
			
		}
	}
	//readbg() << "listbox has " << m_listbox1->numItems() << " items\n";
}

void SimpleExampleWindow::onRefreshTimer()
{
	return;
	int new_count = GetProjectStateChangeCount(nullptr);
	if (m_last_project_change_count != new_count)
	{
		m_last_project_change_count = new_count;
		populate_listbox();
		//readbg() << "project has changed! " << m_last_project_change_count << "\n";
	}
}

void SimpleExampleWindow::resized()
{
	MRP::Size sz = getSize();
	m_edit1->setBounds({ 5,5,sz.getWidth() - 10,20 });
	m_but1->setBounds({ 5, 30 , 100 , 20 });
	m_but2->setBounds({ sz.getWidth()-105, 30 ,100,20 });
	m_but3->setBounds({ 105, 30 ,120,20 });
	m_but4->setBounds({ 230, 30 ,120,20 });
	m_listbox1->setBounds({ 5, 55, sz.getWidth() - 10, sz.getHeight()-60 });
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
	set_readbg_decimals(3);
	readbg() << slidindex << " moved to " << v << "\n";
}

SliderBankWindow* g_sliderbankwindow = nullptr;

HWND toggle_sliderbank_window(HWND parent)
{
	if (is_valid_mrp_window(g_sliderbankwindow)==false)
		g_sliderbankwindow=nullptr;
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

class FFTSizesValueConverter : public IValueConverter
{
public:
	FFTSizesValueConverter() {}
	const std::array<int, 8> c_fft_sizes{ 128,256,512,1024,2048,4096,8192,16384 };
	// Inherited via IValueConverter
	virtual double fromNormalizedToValue(double x) override
	{
		return c_fft_sizes[x*(c_fft_sizes.size() - 1)];
	}

	virtual double toNormalizedFromValue(double x) override
	{
		for (int i = 0; i<c_fft_sizes.size() - 1; ++i)
		{
			if (x >= c_fft_sizes[i] && x<c_fft_sizes[i + 1])
				return 1.0 / c_fft_sizes.size() *i;
		}
		return 0.0;
	}

	virtual std::string toStringFromValue(double x) override
	{
		return std::to_string((int)x);
	}

	virtual double fromStringToValue(const std::string & x) override
	{
		return c_fft_sizes[0];
	}

};

inline double skip_near_zero(double x, double threshold)
{
	if (x >= -threshold && x <= threshold)
	{
		if (x < 0.0)
			return -threshold;
		if (x >= 0.0)
			return threshold;
	}
	return x;
}

void generate_items_sequence(std::shared_ptr<breakpoint_envelope> env, const char* fn)
{
	SetCursorContext(1, NULL);
	Main_OnCommand(40182, 0); // select all items
	Main_OnCommand(40006, 0); // remove selected items
	MediaTrack* dest_track = GetTrack(nullptr, 0);
	double timepos = 0.0;
	double seqlen = 30.0;
	int sanity = 0;
	while (timepos < seqlen)
	{
		create_item_result r = create_item_with_take_and_source(dest_track, fn);
		SetMediaItemPosition(r.item, timepos, false);
		double srclen = GetMediaSourceLength(r.src, nullptr);
		double normtime = 1.0 / seqlen * timepos;
		double directionfactor = skip_near_zero(-1.0 + 2.0*env->interpolate(normtime), 0.02);
		if (directionfactor < 0.0)
		{
			//readbg() << "item " << sanity << " should be reversed\n";
			SetMediaItemSelected(r.item, true);
			Main_OnCommand(41051, 0); // toggle reverse
			SetMediaItemSelected(r.item, false);
		}


		double absfactor = fabs(directionfactor);
		double itemlen = bound_value(0.02, srclen*absfactor, srclen);
		SetMediaItemLength(r.item, itemlen, false);
		SetMediaItemTakeInfo_Value(r.take, "D_PLAYRATE", 1.0 / absfactor);
		timepos += srclen*absfactor;
		++sanity;
		if (sanity > 1000)
		{
			readbg() << "too many items created!\n";
			break;
		}
	}
	UpdateArrange();
}

TestMRPPWindow::TestMRPPWindow(HWND parent, std::string title) : MRPWindow(parent, title)
{
	for (int i = 0; i < 8; ++i)
	{
		auto but = std::make_shared<WinButton>(this, std::to_string(i));
		but->GenericNotifyCallback = [i](GenericNotifications)
		{
			readbg() << "you pressed " << i << "\n";
		};
		add_control(but);
	}
	// Button 0 toggless enabled state of button 1
	m_controls[0]->GenericNotifyCallback = [this](GenericNotifications)
	{
		m_controls[1]->setEnabled(!m_controls[1]->isEnabled());
	};
	m_envcontrol1 = std::make_shared<EnvelopeControl>(this);

	// Button 3 toggless enabled state of envelope control
	m_controls[3]->GenericNotifyCallback = [this](GenericNotifications)
	{
		m_envcontrol1->setEnabled(!m_envcontrol1->isEnabled());
	};

	// Button 7 toggless visible state of button 0
	m_controls[7]->GenericNotifyCallback = [this](GenericNotifications)
	{
		m_controls[0]->setVisible(!m_controls[0]->isVisible());
	};
	auto env = std::make_shared<breakpoint_envelope>("foo", LICE_RGBA(255, 255, 255, 255));
	env->add_point({ 0.0, 0.5 , envbreakpoint::Power, 0.5 }, true);
	env->add_point({ 0.5, 0.0 , envbreakpoint::Power, 0.5 }, true);
	env->add_point({ 1.0, 0.5 }, true);
	m_envcontrol1->add_envelope(env);

	m_envcontrol1->GenericNotifyCallback = [this, env](GenericNotifications reason)
	{
		//if (reason == GenericNotifications::AfterManipulation)
		//	generate_items_sequence(env, m_edit1->getText().c_str());
	};

	add_control(m_envcontrol1);

	// Button 5 does some Xenakios silliness
	m_controls[5]->GenericNotifyCallback = [this, env](GenericNotifications)
	{
		generate_items_sequence(env, m_edit1->getText().c_str());
	};

	m_label1 = std::make_shared<WinLabel>(this, "This is a label");
	add_control(m_label1);

	m_edit1 = std::make_shared<WinLineEdit>(this, "C:/MusicAudio/pihla_ei/ei_mono_005.wav");
	add_control(m_edit1);
	m_edit1->TextCallback = [this](std::string txt)
	{
		m_label1->setText(txt);
	};
	// Button 6 launches bogus work in another thread to demo progress bar
	m_controls[6]->GenericNotifyCallback = [this](GenericNotifications)
	{
		m_progressbar1->setVisible(true);
		// we don't deal with multiple background tasks now, so disable the button to start the task
		m_controls[6]->setEnabled(false);
		static int rseed = 0;
		auto task = [this](int randseed)
		{
			std::mt19937 randgen(randseed);
			std::uniform_real_distribution<double> randdist(0.0, 1.0);
			double accum = 0.0;
			const int iterations = 50000000;
			double t0 = time_precise();
			for (int i = 0; i < iterations; ++i)
			{
				accum += randdist(randgen);
				//accum += randdist(randgen);
				// Production code should not do this at this granularity, because setProgressValue deals with
				// an atomic value. but this is just a demo...
				m_progressbar1->setProgressValue(1.0 / iterations*i);
			}
			double t1 = time_precise();
			auto finishtask = [=]()
			{
				m_edit1->setText(std::to_string(accum) + " elapsed time " + std::to_string(t1-t0));
				m_progressbar1->setProgressValue(0.0);
				m_progressbar1->setVisible(false);
				m_controls[6]->setEnabled(true);
			};
			execute_in_main_thread(finishtask);
		};
		m_future1 = std::async(std::launch::async, task, rseed);
		++rseed;
	};
	// Button 1 shows popup menu
	m_controls[1]->GenericNotifyCallback = [this, env](GenericNotifications)
	{
		PopupMenu popmenu(getWindowHandle());
		popmenu.add_menu_item("Menu entry 1", [](PopupMenu::CheckState) {});
		popmenu.add_menu_item("Menu entry 2", m_menuitem2state, [this](PopupMenu::CheckState cs)
		{
			m_menuitem2state = cs;
		});
		popmenu.add_menu_item("Menu entry 3", m_menuitem3state, [this](PopupMenu::CheckState cs) 
		{
			m_menuitem3state = cs;
		});
		PopupMenu submenu(getWindowHandle());
		submenu.add_menu_item("Submenu entry 1", [](PopupMenu::CheckState) { readbg() << "submenu entry 1\n"; });
		submenu.add_menu_item("Submenu entry 2", [](PopupMenu::CheckState) { readbg() << "submenu entry 2\n"; });
		PopupMenu subsubmenu(getWindowHandle());
		for (int i = 0; i < 8; ++i)
		{
			subsubmenu.add_menu_item(std::string("Subsubmenu entry ") + std::to_string(i + 1), [i](PopupMenu::CheckState) 
			{
				readbg() << "subsubmenu entry " << i + 1 << "\n";
			});
		}
		submenu.add_submenu("Going still deeper", subsubmenu);
		popmenu.add_submenu("More stuff", submenu);
		popmenu.execute(m_controls[1]->getXPosition(), m_controls[1]->getYPosition());
	};
	// Button 4 removes envelope points with value over 0.5
	m_controls[4]->GenericNotifyCallback = [this, env](GenericNotifications)
	{
		env->remove_points_conditionally([](const envbreakpoint& pt)
		{ return pt.get_y() > 0.5; });
		m_envcontrol1->repaint();
	};
	m_combo1 = std::make_shared<WinComboBox>(this);
	m_combo1->addItem("Apple", -9001);
	m_combo1->addItem("Pear", 666);
	m_combo1->addItem("Kiwi", 42);
	m_combo1->addItem("Banana", 100);
	m_combo1->SelectedChangedCallback = [this](int index)
	{
		int user_id = m_combo1->userIDfromIndex(index);
		readbg() << "combo index " << index << " userid " << user_id << "\n";
	};
	add_control(m_combo1);
	m_combo1->setSelectedUserID(42);

	m_combo2 = std::make_shared<WinComboBox>(this);
	m_combo2->addItem("Item 1", 100);
	m_combo2->addItem("Item 2", 101);
	m_combo2->addItem("Item 3", 102);
	m_combo2->addItem("Item 4", 103);
	m_combo2->SelectedChangedCallback = [this](int index)
	{
		int user_id = m_combo2->userIDfromIndex(index);
		readbg() << "combo index " << index << " userid " << user_id << "\n";
	};
	add_control(m_combo2);
	m_combo2->setSelectedIndex(0);

	m_slider1 = std::make_shared<ReaSlider>(this, 0.5);
	//m_slider1->setValueConverter(std::make_shared<FFTSizesValueConverter>());
	m_slider1->SliderValueCallback = [this](GenericNotifications, double x)
	{
		m_label1->setText(std::to_string(x));
		m_progressbar1->setProgressValue(x);
	};
	add_control(m_slider1);
	m_zoomscroll1 = std::make_shared<ZoomScrollBar>(this);
	add_control(m_zoomscroll1);
	m_zoomscroll1->RangeChangedCallback = [this](double t0, double t1)
	{
		m_envcontrol1->setViewTimeRange(t0, t1);
	};

	m_progressbar1 = std::make_shared<ProgressControl>(this);
	m_progressbar1->setVisible(false);
}

void TestMRPPWindow::resized()
{
	if (m_controls.size() == 0)
		return;
	auto sz = getSize();
	int w = sz.getWidth();
	int h = sz.getHeight();
	int gdivs = 16;
	MRP::Rectangle wg(0, 0, w, h);
	for (int i = 0; i < 8; ++i)
	{
		m_controls[i]->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 0, i, 1, i + 1));
	}
	m_slider1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 1, 0, 16, 1));
	m_envcontrol1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 1, 1, 16, 10));
	m_zoomscroll1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 1, 10, 16, 11));
	m_edit1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 0, 11, 16, 12));
	//m_combo1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 0, 15, 7, 16));
	//m_combo2->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 8, 15, 16, 16));
	m_progressbar1->setBounds(MRP::Rectangle::fromGridPositions(wg, gdivs, 0, 15, 16, 16));
}

void TestMRPPWindow::onRefreshTimer()
{
	
}

std::string TestMRPModalWindow::getText(int which)
{
	return m_line_edit_cached_text;
}

void TestMRPModalWindow::onModalClose()
{
	m_line_edit_cached_text = m_line_edit->getText();
}

void TestMRPModalWindow::init_modal_dialog()
{
	add_control(std::make_shared<WinButton>(this, "OK"));
	m_controls[0]->setBounds({ 5, 35, 50, 30 });
	m_controls[0]->GenericNotifyCallback = [this](GenericNotifications)
	{
		finishModal(Accepted);
	};
	add_control(std::make_shared<WinButton>(this, "Cancel"));
	m_controls[1]->setBounds({ 60, 35, 50, 30 });
	m_controls[1]->GenericNotifyCallback = [this](GenericNotifications)
	{
		finishModal(Rejected);
	};
	m_line_edit = std::make_shared<WinLineEdit>(this, "Sample text");
	m_line_edit->setBounds({ 55, 5, getSize().getWidth() - 100, 25 });
	add_control(m_line_edit);
	auto label = std::make_shared<WinLabel>(this, "Foobarbaz");
	add_control(label);
	label->setBounds({ 5, 5, 45, 20 });
}


HWND open_win_controls_window(HWND parent)
{
	static int counter = 1;
	// Test window deletes itself when it is closed, so we can keep this
	// raw pointer just here
	TestMRPPWindow* w = new TestMRPPWindow(parent, std::string("Testing window ") + std::to_string(counter));
	w->setDestroyOnClose(true);
	w->setPosition(20 + counter * 20, 60 + counter * 20);
	w->setSize(500, 300);
	++counter;
	return w->getWindowHandle();
}

TestMRPModalWindow::TestMRPModalWindow(HWND parent, std::string title) : MRPModalDialog(parent, title)
{

}

void show_modal_dialog(HWND parent)
{
	{
		TestMRPModalWindow dlg(parent,"Test Modal Dialog");
		MRPWindow::ModalResult r = dlg.runModally();
		if (r == MRPWindow::Accepted)
			readbg() << "Dialog was accepted : " << dlg.getText(0) << "\n";
		else readbg() << "Dialog was cancelled\n";
		//if (is_valid_mrp_window(&dlg) == true)
		//	readbg() << "MRPWindow still valid when it should not be\n";
	}
	//readbg() << g_mrpwindowsmap.size() << " entries in mrpwindowsmap\n";
}
