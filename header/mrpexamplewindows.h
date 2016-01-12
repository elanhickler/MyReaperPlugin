#pragma once

#include "mrpwindows.h"

#include <future>

class SimpleExampleWindow : public MRPWindow
{
public:
	SimpleExampleWindow(HWND parent, std::string title);
	void resized() override;
	void populate_listbox();
	void onRefreshTimer() override;
private:
	std::shared_ptr<WinLineEdit> m_edit1;
	std::shared_ptr<WinLineEdit> m_edit2;
	std::shared_ptr<WinButton> m_but1;
	std::shared_ptr<WinButton> m_but2;
	std::shared_ptr<WinButton> m_but3;
	std::shared_ptr<WinButton> m_but4;
	std::shared_ptr<WinListBox> m_listbox1;
	std::unordered_map<int, MediaItem*> m_itemmap;
	int m_last_project_change_count = 0;
};

HWND toggle_simple_example_window(HWND parent);

class SliderBankWindow : public MRPWindow
{
public:
	SliderBankWindow(HWND parent);
	void resized() override;
	virtual void on_slider_value_changed(int slidindex, double v);
private:
	struct slider_controls
	{
		std::shared_ptr<ReaSlider> m_slider;
		std::shared_ptr<WinLabel> m_label;
		std::shared_ptr<WinLineEdit> m_editbox;
	};
	std::vector<slider_controls> m_sliders;
};

class TestMRPModalWindow : public MRPModalDialog
{
public:
	TestMRPModalWindow(HWND parent, std::string title);
	void init_modal_dialog();
	void onModalClose() override;
	std::string getText(int which);
private:
	std::shared_ptr<WinLineEdit> m_line_edit;
	std::string m_line_edit_cached_text;
};

class TestMRPPWindow : public MRPWindow
{
public:
	TestMRPPWindow(HWND parent, std::string title = "Untitled");
	~TestMRPPWindow()
	{
		readbg() << "TestMRPWindow dtor\n";
	}
	void resized() override;
	void onRefreshTimer() override;
private:
	std::shared_ptr<WinComboBox> m_combo1;
	std::shared_ptr<WinComboBox> m_combo2;
	std::shared_ptr<ReaSlider> m_slider1;
	std::shared_ptr<EnvelopeControl> m_envcontrol1;
	std::shared_ptr<ZoomScrollBar> m_zoomscroll1;
	std::shared_ptr<WinLabel> m_label1;
	std::shared_ptr<WinLineEdit> m_edit1;
	std::shared_ptr<ProgressControl> m_progressbar1;
	std::shared_ptr<WinButton> m_button1;
	std::future<void> m_future1;
	PopupMenu::CheckState m_menuitem2state = PopupMenu::Unchecked;
	PopupMenu::CheckState m_menuitem3state = PopupMenu::Checked;
};

void show_modal_dialog(HWND parent);

HWND toggle_sliderbank_window(HWND parent);
HWND open_win_controls_window(HWND parent);