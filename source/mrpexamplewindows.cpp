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
