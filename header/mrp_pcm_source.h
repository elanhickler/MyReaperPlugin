#pragma once

#include "WDL/WDL/lice/lice.h"
#include "reaper_plugin/reaper_plugin_functions.h"

#include <memory>


class MRP_AudioDSP;

class MRP_PCMSource : public PCM_source
{
public:
	MRP_PCMSource(std::shared_ptr<MRP_AudioDSP> dsp) : m_dsp(dsp) {}
	
	// Inherited via PCM_source
	PCM_source * Duplicate() override;

	bool IsAvailable() override;

	const char * GetType() override;

	bool SetFileName(const char * newfn) override;

	int GetNumChannels() override;

	double GetSampleRate() override;

	double GetLength() override;

	int PropertiesWindow(HWND hwndParent) override;

	void GetSamples(PCM_source_transfer_t * block) override;

	void GetPeakInfo(PCM_source_peaktransfer_t * block) override;

	void SaveState(ProjectStateContext * ctx) override;

	int LoadState(const char * firstline, ProjectStateContext * ctx) override;

	void Peaks_Clear(bool deleteFile) override;

	int PeaksBuild_Begin() override;

	int PeaksBuild_Run() override;

	void PeaksBuild_Finish() override;

	int Extended(int call, void *parm1, void *parm2, void *parm3) override;
private:
	std::shared_ptr<MRP_AudioDSP> m_dsp;
};
