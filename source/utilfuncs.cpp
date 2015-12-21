#include "utilfuncs.h"
#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

int readbgbuf::overflow(int c)
{
	if (c != traits_type::eof()) {
		char ch[2] = { traits_type::to_char_type(c), 0 };
		ShowConsoleMsg(ch);
	}
	return c;
}

std::streamsize readbgbuf::xsputn(const char *buffer, std::streamsize n)
{
	std::string buf(buffer, buffer + n);
	ShowConsoleMsg(buf.c_str());
	return n;
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
