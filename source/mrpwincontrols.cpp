#include "mrpwincontrols.h"

WinControl::WinControl(HWND parent)
{

}

WinControl::~WinControl()
{

}

WinButton::WinButton(HWND parent, std::string text) :
	WinControl(parent)
{

}

void WinButton::setText(std::string text)
{

}