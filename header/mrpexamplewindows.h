#pragma once

#include "mrpwindows.h"

class SimpleExampleWindow : public MRPWindow
{
public:
	SimpleExampleWindow(HWND parent, std::string title);
	void resized() override;
private:
	std::shared_ptr<WinLineEdit> m_edit1;
	std::shared_ptr<WinLineEdit> m_edit2;
	std::shared_ptr<WinButton> m_but1;
	std::shared_ptr<WinButton> m_but2;
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

HWND toggle_sliderbank_window(HWND parent);
