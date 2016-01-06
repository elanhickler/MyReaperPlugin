#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

readbg::~readbg()
{
	ShowConsoleMsg(m_buf.c_str());
}

char g_d_format_str[16] = "%.2f\0";

void set_readbg_decimals(int decims)
{
	if (decims < 0 || decims>8)
		return;
	// might wanna check if the secure version of sprintf can be used...
	sprintf(g_d_format_str, "%%.%df", decims);
}

std::string is_source_audio(PCM_source* src)
{
	if (src == nullptr)
		return "Source null";
	if (src->IsAvailable() == false)
		return "Source offline";
	if (strcmp(src->GetType(), "MIDI") == 0)
		return "Source is MIDI";
	if (src->GetSampleRate() < 1.0)
		return "Source sample rate less than 1 Hz";
	if (src->GetNumChannels() < 1)
		return "Source has no audio channels";
	if (src->GetLength() <= 0.0)
		return "Source length equal or less than zero seconds";
	return std::string();
}

create_item_result create_item_with_take_and_source(MediaTrack * track, const char * fn)
{
	create_item_result result;
	result.item = AddMediaItemToTrack(track);
	result.take = AddTakeToMediaItem(result.item);
	result.src = PCM_Source_CreateFromFile(fn);
	SetMediaItemTake_Source(result.take, result.src);
	return result;
}

UINT_PTR g_main_thread_exec_timer = 0;
std::mutex g_main_thread_exec_mutex;
std::vector<std::function<void(void)>> g_main_thread_exec_tasks;

void CALLBACK mtetimerproc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (idEvent == g_main_thread_exec_timer)
	{
		g_main_thread_exec_mutex.lock();
		for (auto& e : g_main_thread_exec_tasks)
			e();
		g_main_thread_exec_tasks.clear();
		g_main_thread_exec_mutex.unlock();
	}
}

void start_or_stop_main_thread_executor(bool stop)
{
	if (stop == false)
	{
		g_main_thread_exec_timer = SetTimer(0, 1000, 10, mtetimerproc);
	}
	else
	{
		if (g_main_thread_exec_timer != 0)
		{
			g_main_thread_exec_mutex.lock();
			g_main_thread_exec_tasks.clear();
			KillTimer(0,g_main_thread_exec_timer);
			g_main_thread_exec_mutex.unlock();
		}
	}
}

void execute_in_main_thread(std::function<void(void)> f)
{
	g_main_thread_exec_mutex.lock();
	g_main_thread_exec_tasks.push_back(f);
	g_main_thread_exec_mutex.unlock();
}
