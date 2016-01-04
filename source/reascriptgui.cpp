
#include "reascriptgui.h"
#include "mylicecontrols.h"
#include <regex>

extern HWND g_parent;

std::unordered_set<ReaScriptWindow*> g_reascriptwindows;

ReaScriptWindow::ReaScriptWindow(std::string title) : MRPWindow(g_parent,title)
{
	g_reascriptwindows.insert(this);
	// deliberately allocate tons of memory to see object destruction works
	m_leak_test.resize(100000000);
}

ReaScriptWindow::~ReaScriptWindow()
{
	g_reascriptwindows.erase(this);
}

std::shared_ptr<LiceControl> create_licecontrol(ReaScriptWindow* w, std::string classname)
{
	if (classname == "BreakpointEnvelope")
	{
		auto points = std::make_shared<breakpoint_envelope>("Untitled envelope",LICE_RGBA(0,255,0,255));
		points->add_point({ 0.0,0.5 }, false);
		points->add_point({ 1.0,0.5 }, false);
		points->sort_points();
		auto control = std::make_shared<EnvelopeControl>(w);
		control->add_envelope(points);
		// By default, don't set the control dirty on envelope point drag
		control->setIntegerProperty(2, 0);
		return control;
	}
	if (classname == "ZoomScrollBar")
	{
		auto zsb = std::make_shared<ZoomScrollBar>(w);
		return zsb;
	}
	return nullptr;
}

bool ReaScriptWindow::addControlFromName(std::string cname, std::string objectname)
{
	if (cname == "Button")
	{
		auto c = std::make_shared<WinButton>(this, objectname);
		c->setObjectName(objectname);
		c->GenericNotifyCallback = [this, objectname](GenericNotifications)
		{
			m_dirty_controls.insert(objectname);
		};
		c->setBounds({ 5, 5, 50, 25 });
		add_control(c);
		return true;
	}
	else if (cname == "Slider")
	{
		auto c = std::make_shared<ReaSlider>(this);
		c->setObjectName(objectname);
		c->SliderValueCallback = [this, objectname](GenericNotifications, double)
		{
			m_dirty_controls.insert(objectname);
		};
		c->setBounds({ 5, 5, 50, 25 });
		add_control(c);
		return true;
	}
	else 
	{
		auto c = create_licecontrol(this, cname);
		if (c != nullptr)
		{
			c->setObjectName(objectname);
			c->GenericNotifyCallback = [this, objectname](GenericNotifications)
			{
				m_dirty_controls.insert(objectname);
			};
			c->setBounds({ 5, 5, 50, 25 });
			add_control(c);
			return true;
		}
	}
	return false;
}

void ReaScriptWindow::setControlBounds(std::string name, int x, int y, int w, int h)
{
	WinControl* c = control_from_name(name);
	if (c != nullptr)
	{
		c->setBounds({ x, y, w, h });
	}
}

#ifdef WIN32
#define USE_REG_EXP_CONTROL_SEARCH
#endif

WinControl* ReaScriptWindow::control_from_name(const std::string& name)
{
#ifdef USE_REG_EXP_CONTROL_SEARCH
	std::regex re(name, std::regex_constants::ECMAScript | std::regex_constants::icase);
	for (auto& e : m_controls)
	{
		if (std::regex_match(e->getObjectName(), re) == true)
			return e.get();
	}
	return nullptr;
#else
	for (auto& e : m_controls)
		if (e->getObjectName() == name)
			return e.get();
	return nullptr;
#endif
}

bool ReaScriptWindow::isControlDirty(std::string name)
{
	return m_dirty_controls.count(name)==1;
}

void ReaScriptWindow::clearDirtyControls()
{
	m_dirty_controls.clear();
}

double ReaScriptWindow::getControlValueDouble(const std::string& obname, int which)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		return c->getFloatingPointProperty(which);
	}
	return 0.0;
}

int ReaScriptWindow::getControlValueInt(const std::string& obname, int which)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		return c->getIntegerProperty(which);
	}
	return 0;
}

void ReaScriptWindow::setControlValueString(const std::string& obname, int which, std::string text)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		c->setStringProperty(which, text);
	}
}

void ReaScriptWindow::setControlValueDouble(const std::string& obname, int which, double v)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		c->setFloatingPointProperty(which, v);
	}
}

void ReaScriptWindow::setControlValueInt(const std::string& obname, int which, int v)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		c->setIntegerProperty(which, v);
	}
}

void ReaScriptWindow::sendCommandString(const std::string & obname, const std::string & cmd)
{
	WinControl* c = control_from_name(obname);
	if (c != nullptr)
	{
		c->sendStringCommand(cmd);
	}
}

bool is_valid_reascriptwindow(ReaScriptWindow* w)
{
	return g_reascriptwindows.count(w) == 1;
}

