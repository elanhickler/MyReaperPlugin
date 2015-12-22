#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include <string>
#include <functional>

enum class GenericNotifications
{
	Unknown,
	Clicked,
	Something,
	SomethingA,
	SomethingB,
	SomethingC,
	SomethingD,
	Time,
	Value,
	TimeRange,
	ValueRange,
	ViewTimeRange,
	ViewValueRange,
	ObjectMoved,
	ObjectAdded,
	ObjectRemoved,
	ObjectSize,
	ObjectLength,
	ObjectProperty,
	ObjectCount,
	PositionChanged,
	Contents,
	Text,
	Number,
	Rotation,
	Scroll,
	Color,
	Image,
	Audio,
	PlaybackPosition,
	PlaybackState,
	BeforeManipulation,
	DuringManipulation,
	AfterManipulation,
	BeforeTask,
	DuringTask,
	AfterTask
};

class WinControl
{
public:
	WinControl(HWND parent);
	virtual ~WinControl();
	virtual void setBounds(int x, int y, int w, int h);

	virtual bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return false; }

protected:
	HWND m_hwnd = NULL;
	HWND m_parent = NULL;
	int m_control_id = 0;
};

class WinButton : public WinControl
{
public:
	WinButton(HWND parent, std::string text);
	void setText(std::string text);
	std::string getText();
	std::function<void(GenericNotifications)> GenericNotification;

	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

class WinLabel : public WinControl
{
public:
	WinLabel(HWND parent, std::string text);
	void setText(std::string text);
	std::string getText();
	
};
