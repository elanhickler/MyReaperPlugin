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
