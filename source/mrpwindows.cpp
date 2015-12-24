#include "mrpwindows.h"
#include <unordered_map>

// Oh, the horror
#ifndef WIN32
#include "WDL/WDL/swell/swell-dlggen.h"
#define IDD_EMPTYDIALOG 666
#ifndef SWELL_DLG_SCALE_AUTOGEN
#define SWELL_DLG_SCALE_AUTOGEN 1.7
#endif
#ifndef SWELL_DLG_FLAGS_AUTOGEN
#define SWELL_DLG_FLAGS_AUTOGEN SWELL_DLG_WS_FLIPPED|SWELL_DLG_WS_NOAUTOSIZE
#endif

#ifndef SET_IDD_EMPTYDIALOG_SCALE
#define SET_IDD_EMPTYDIALOG_SCALE SWELL_DLG_SCALE_AUTOGEN
#endif
#ifndef SET_IDD_EMPTYDIALOG_STYLE
#define SET_IDD_EMPTYDIALOG_STYLE SWELL_DLG_FLAGS_AUTOGEN|SWELL_DLG_WS_RESIZABLE|SWELL_DLG_WS_OPAQUE
#endif
SWELL_DEFINE_DIALOG_RESOURCE_BEGIN(IDD_EMPTYDIALOG,SET_IDD_EMPTYDIALOG_STYLE,"Dialog",309,179,SET_IDD_EMPTYDIALOG_SCALE)
BEGIN
END
SWELL_DEFINE_DIALOG_RESOURCE_END(IDD_EMPTYDIALOG)

#define IDD_EMPTYDIALOG2 667
#ifndef SWELL_DLG_SCALE_AUTOGEN
#define SWELL_DLG_SCALE_AUTOGEN 1.7
#endif
#ifndef SWELL_DLG_FLAGS_AUTOGEN
#define SWELL_DLG_FLAGS_AUTOGEN SWELL_DLG_WS_FLIPPED|SWELL_DLG_WS_NOAUTOSIZE
#endif

#ifndef SET_IDD_EMPTYDIALOG_SCALE
#define SET_IDD_EMPTYDIALOG_SCALE SWELL_DLG_SCALE_AUTOGEN
#endif
#ifndef SET_IDD_EMPTYDIALOG2_STYLE
#define SET_IDD_EMPTYDIALOG2_STYLE SWELL_DLG_FLAGS_AUTOGEN|SWELL_DLG_WS_OPAQUE
#endif
SWELL_DEFINE_DIALOG_RESOURCE_BEGIN(IDD_EMPTYDIALOG2,SET_IDD_EMPTYDIALOG2_STYLE,"Dialog",309,179,SET_IDD_EMPTYDIALOG_SCALE)
BEGIN
END
SWELL_DEFINE_DIALOG_RESOURCE_END(IDD_EMPTYDIALOG2)


#endif

extern HINSTANCE g_hInst;


#ifdef WIN32
struct MyDLGTEMPLATE : DLGTEMPLATE
{
	WORD ext[3];
	MyDLGTEMPLATE()
	{
		memset(this, 0, sizeof(*this));
	}
};
#endif

HWND open_win_controls_window(HWND parent)
{
	static int counter = 1;
	// Test window deletes itself when it is closed, so we can keep this
	// raw pointer just here
	TestMRPPWindow* w = new TestMRPPWindow(parent, std::string("Test window ") + std::to_string(counter));
	w->setDestroyOnClose(true);
	w->setPosition(20 + counter * 20, 60 + counter * 20);
	w->setSize(500, 300);
	++counter;
	return w->getWindowHandle();
}

std::unordered_map<HWND, MRPWindow*> g_mrpwindowsmap;
extern HWND g_parent;

MRPWindow::MRPWindow(HWND parent, std::string title)
{
#ifdef WIN32
	MyDLGTEMPLATE t;
	t.style = DS_SETFONT | DS_FIXEDSYS | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
	t.cx = 200;
	t.cy = 100;
	t.dwExtendedStyle = WS_EX_TOOLWINDOW;
	m_hwnd = CreateDialogIndirectParam(g_hInst, &t, parent, (DLGPROC)dlgproc, (LPARAM)this);
#else
	m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
		parent, dlgproc, (LPARAM)this);
#endif
	//m_hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG),
	//	parent, dlgproc, (LPARAM)this);
	g_mrpwindowsmap[m_hwnd] = this;
	SetWindowText(m_hwnd, title.c_str());
	SetWindowPos(m_hwnd, NULL, 20, 60, 100, 100, SWP_NOACTIVATE | SWP_NOZORDER);
	ShowWindow(m_hwnd, SW_SHOW);
}

MRPWindow::~MRPWindow()
{
	readbg() << "MRPWindow dtor\n";
	m_controls.clear();
	if (m_hwnd != NULL)
	{
		DestroyWindow(m_hwnd);
		g_mrpwindowsmap.erase(m_hwnd);
	}
}

std::pair<int, int> MRPWindow::getSize()
{
	if (m_hwnd == NULL)
		return{ 0,0 };
	RECT r;
	GetClientRect(m_hwnd, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	return{ w,h };
}

void MRPWindow::setPosition(int x, int y)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}
}

void MRPWindow::setSize(int w, int h)
{
	if (m_hwnd != NULL)
	{
		SetWindowPos(m_hwnd, NULL, 0, 0, w, h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
}

MRPWindow::ModalResult MRPWindow::runModally(HWND parent)
{
	m_is_modal = true;
#ifdef WIN32
	MyDLGTEMPLATE t;
	t.style = DS_SETFONT | DS_FIXEDSYS | WS_CAPTION | WS_SYSMENU;
	t.cx = 200;
	t.cy = 100;
	DialogBoxIndirectParam(g_hInst, &t, parent, dlgproc, (LPARAM)this);
#else
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_EMPTYDIALOG), parent, dlgproc, (LPARAM)this);
#endif
	return m_modal_result;
}

void show_modal_dialog(HWND parent)
{
	{
		TestMRPModalWindow dlg;
		MRPWindow::ModalResult r = dlg.runModally(parent);
		if (r == MRPWindow::Accepted)
			readbg() << "Dialog was accepted : " << dlg.m_line_edit->getText();
		else readbg() << "Dialog was cancelled\n";
	}
	readbg() << g_mrpwindowsmap.size() << " entries in mrpwindowsmap\n";
}

void MRPWindow::closeRequested()
{
	readbg() << "close requested...\n";
	if (m_hwnd != NULL && m_destroy_on_close == false)
	{
		readbg() << "only hiding this window...\n";
		ShowWindow(m_hwnd, SW_HIDE);
		
		return;
	}

	if (m_destroy_on_close == true)
	{
		delete this;
		readbg() << "window map has " << g_mrpwindowsmap.size() << " entries\n";
	}
}


INT_PTR MRPWindow::dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_INITDIALOG)
	{
		MRPWindow* mrpw = (MRPWindow*)lp;
		if (mrpw != nullptr)
		{
			mrpw->m_hwnd = hwnd;
			g_mrpwindowsmap[hwnd] = mrpw;
			if (mrpw->m_is_modal == true)
				mrpw->init_modal_dialog();
		}
		return TRUE;
	}
	if (msg == WM_COMMAND || msg == WM_HSCROLL || msg == WM_VSCROLL)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			for (auto& e : mptr->m_controls)
				if (e->handleMessage(hwnd, msg, wp, lp) == true)
					return TRUE;
		}
	}
	if (msg == WM_SIZE)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			mptr->resized();
			InvalidateRect(hwnd, NULL, TRUE);
			return TRUE;
		}
	}
	if (msg == WM_CLOSE)
	{
		MRPWindow* mptr = get_from_map(g_mrpwindowsmap, hwnd);
		if (mptr != nullptr)
		{
			if (mptr->m_is_modal == false)
			{
				mptr->closeRequested();
			}
			else
			{
				mptr->m_modal_result = MRPWindow::Rejected;
				EndDialog(hwnd, 2);
			}
			return TRUE;
		}
	}
	return FALSE;
}

class FFTSizesValueConverter : public IValueConverter
{
public:
	FFTSizesValueConverter() {}
	const std::array<int, 8> c_fft_sizes{ 128,256,512,1024,2048,4096,8192,16384 };
	// Inherited via IValueConverter
	virtual double fromNormalizedToValue(double x) override
	{
		return c_fft_sizes[x*(c_fft_sizes.size()-1)];
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
		SetMediaItemTakeInfo_Value(r.take, "D_PLAYRATE", 1.0/absfactor);
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
		auto but = std::make_shared<WinButton>(m_hwnd, std::to_string(i));
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
	m_envcontrol1 = std::make_shared<EnvelopeControl>(m_hwnd);
	
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
	env->add_point({ 0.0, 0.5 }, true);
	env->add_point({ 1.0, 0.5 }, true);
	m_envcontrol1->add_envelope(env);
	
	m_envcontrol1->GenericNotifyCallback = [this,env](GenericNotifications reason)
	{
		if (reason==GenericNotifications::AfterManipulation)
			generate_items_sequence(env,m_edit1->getText().c_str());
	};
	
	add_control(m_envcontrol1);
	
	// Button 5 does some Xenakios silliness
	m_controls[5]->GenericNotifyCallback = [this,env](GenericNotifications)
	{
		generate_items_sequence(env, m_edit1->getText().c_str());
	};

	m_label1 = std::make_shared<WinLabel>(m_hwnd, "This is a label");
	add_control(m_label1);
	
	m_edit1 = std::make_shared<WinLineEdit>(m_hwnd, "C:/MusicAudio/pihla_ei/ei_mono_005.wav");
	add_control(m_edit1);
	m_edit1->TextCallback = [this](std::string txt)
	{
		m_label1->setText(txt);
	};
	// Button 6 reverses text of line edit :-)
	m_controls[6]->GenericNotifyCallback = [this](GenericNotifications)
	{
		std::string txt = m_edit1->getText();
		std::reverse(txt.begin(), txt.end());
		m_edit1->setText(txt);
	};
	// Button 4 removes envelope points with value over 0.5
	m_controls[4]->GenericNotifyCallback = [this, env](GenericNotifications)
	{
		env->remove_points_conditionally([](const envbreakpoint& pt)
		{ return pt.get_y() > 0.5; });
		m_envcontrol1->repaint();
	};
	m_combo1 = std::make_shared<WinComboBox>(m_hwnd);
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

	m_combo2 = std::make_shared<WinComboBox>(m_hwnd);
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
	
	m_slider1 = std::make_shared<ReaSlider>(m_hwnd, 0.5);
	m_slider1->setValueConverter(std::make_shared<FFTSizesValueConverter>());
	m_slider1->SliderValueCallback = [this](double x)
	{
		m_label1->setText(std::to_string(x));
	};
	add_control(m_slider1);
}

void TestMRPPWindow::resized()
{
	if (m_controls.size() == 0)
		return;
	auto sz = getSize();
	int w = sz.first;
	int h = sz.second;
	int ch = (double)(sz.second-5) / m_controls.size();
	// layout buttons to left side of window
	for (int i = 0; i < 8; ++i)
	{
		m_controls[i]->setBounds(5, 5 + i*25, 40, 20);
	}
	
	m_combo1->setBounds(5, h - 30, w / 2 - 10, 25);
	m_combo2->setBounds(w / 2, h - 30, w / 2 - 5, 25);
	m_edit1->setBounds(5, h - 60, w-10, 20);
	m_slider1->setBounds(50, 5, w - 55, 20);
	m_envcontrol1->setBounds(50, 30, w - 55, h - 120);
	
	int envcontbot = m_envcontrol1->getYPosition() + m_envcontrol1->getHeight();
	m_label1->setBounds(50, envcontbot + 3, w - 55, 20);
}

void TestMRPModalWindow::init_modal_dialog()
{
	add_control(std::make_shared<WinButton>(m_hwnd, "OK"));
	m_controls[0]->setBounds(5, 35, 50, 30);
	m_controls[0]->GenericNotifyCallback = [this](GenericNotifications)
	{
		finishModal(Accepted);
	};
	add_control(std::make_shared<WinButton>(m_hwnd, "Cancel"));
	m_controls[1]->setBounds(60, 35, 50, 30);
	m_controls[1]->GenericNotifyCallback = [this](GenericNotifications)
	{
		finishModal(Rejected);
	};
	m_line_edit = std::make_shared<WinLineEdit>(m_hwnd,"Sample text");
	m_line_edit->setBounds(55, 5, getSize().first-100, 25);
	add_control(m_line_edit);
	auto label = std::make_shared<WinLabel>(m_hwnd,"Foobarbaz");
	add_control(label);
	label->setBounds(5, 5, 45, 20);
}

