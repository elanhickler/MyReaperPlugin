#pragma once

#ifdef _WIN32
#include <windows.h>
#include "WDL/WDL/win32_utf8.h"
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include "utilfuncs.h"

class IValueConverter;
class MRPWindow;

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
	WinControl(MRPWindow* parent);
	virtual ~WinControl();
	
	bool isVisible();
	virtual void setVisible(bool b);
	
	bool isEnabled();
	virtual void setEnabled(bool b);

	int getXPosition() const;
	int getYPosition() const;
	int getWidth() const;
	int getHeight() const;
	MRP::Rectangle getBounds() const;
	virtual void setTopLeftPosition(int x, int y);
	virtual void setBounds(MRP::Rectangle geom);
	virtual void setSize(int w, int h);
	
	
	void setObjectName(std::string name);
	const std::string& getObjectName() const { return m_object_name; }
	
	std::function<void(GenericNotifications)> GenericNotifyCallback;

	// These are mainly for use with functions exported to be used with
	// ReaScript. However, perhaps sometimes a nice hacky use case can be found 
	// in C++ code too...
	virtual double getFloatingPointProperty(int which) { return 0.0; }
	virtual void setFloatingPointProperty(int which, double v) {}
	virtual int getIntegerProperty(int which) { return 0; }
	virtual void setIntegerProperty(int which, int v) {}
	virtual std::string getStringProperty(int which) { return ""; }
	virtual void setStringProperty(int which, std::string v) {}
	virtual void sendStringCommand(const std::string& message) {}

	virtual void onRefreshTimer() {}
	
	virtual bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return false; }
	// Use this responsibly.
	HWND getWindowHandle() const { return m_hwnd; }
protected:
	HWND m_hwnd = NULL;
	MRPWindow* m_parent = nullptr;
	int m_control_id = 0;
	std::string m_object_name;
	bool m_is_enabled = true;
};

class WinButton : public WinControl
{
public:
	WinButton(MRPWindow* parent, std::string text);
	void setText(std::string text);
	std::string getText();
	void setStringProperty(int which, std::string text) override;
	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

class WinLabel : public WinControl
{
public:
	WinLabel(MRPWindow* parent, std::string text, bool alignright=false);
	void setText(std::string text);
	std::string getText();
	
};

class WinLineEdit : public WinControl
{
public:
	WinLineEdit(MRPWindow* parent, std::string inittext);
	void setText(std::string text);
	std::string getText();
	std::function<void(std::string)> TextCallback;
	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

class WinComboBox : public WinControl
{
public:
	WinComboBox(MRPWindow* parent);
	void addItem(std::string text, int user_id);
	int numItems();
	int getSelectedIndex();
	int getSelectedUserID();
	void setSelectedIndex(int index);
	void setSelectedUserID(int id);
	int userIDfromIndex(int index);
	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	std::function<void(int)> SelectedChangedCallback;
private:
	
};

// Just a single column listbox. Probably best to do a separate subclass for multicolumn listview as
// that's pretty complicated to deal with...
class WinListBox : public WinControl
{
public:
	WinListBox(MRPWindow* parent);
	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void addItem(std::string text, int user_id);
	void clearItems();
	void removeItem(int index);
	int userIDfromIndex(int index);
	std::function<void(int)> SelectedChangedCallback;
	int numItems();
	std::string getItemText(int index);
	int getSelectedIndex();
	void setSelectedIndex(int index);
};

class ReaSlider : public WinControl
{
public:
	ReaSlider(MRPWindow* parent, double initpos = 0.5);
	bool handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	std::function<void(GenericNotifications,double)> SliderValueCallback;
	double getValue();
	void setValue(double pos);
	void setTickMarkPositionFromValue(double pos);
	void setValueConverter(std::shared_ptr<IValueConverter> c);
	double getFloatingPointProperty(int which) override;
	void setFloatingPointProperty(int which, double v) override;
private:
	std::shared_ptr<IValueConverter> m_val_converter;
};

int get_wincontrol_leak_count();