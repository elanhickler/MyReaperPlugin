#include "mrp_pcm_source.h"

PCM_source * MRP_PCMSource::Duplicate()
{
	return nullptr;
}

bool MRP_PCMSource::IsAvailable()
{
	return false;
}

const char * MRP_PCMSource::GetType()
{
	return "MRP_PCMSOURCE";
}

bool MRP_PCMSource::SetFileName(const char * newfn)
{
	return false;
}

int MRP_PCMSource::GetNumChannels()
{
	return 0;
}

double MRP_PCMSource::GetSampleRate()
{
	return 0.0;
}

double MRP_PCMSource::GetLength()
{
	return 0.0;
}

int MRP_PCMSource::PropertiesWindow(HWND hwndParent)
{
	return 0;
}

void MRP_PCMSource::GetSamples(PCM_source_transfer_t * block)
{
}

void MRP_PCMSource::GetPeakInfo(PCM_source_peaktransfer_t * block)
{
}

void MRP_PCMSource::SaveState(ProjectStateContext * ctx)
{
}

int MRP_PCMSource::LoadState(const char * firstline, ProjectStateContext * ctx)
{
	return 0;
}

void MRP_PCMSource::Peaks_Clear(bool deleteFile)
{
}

int MRP_PCMSource::PeaksBuild_Begin()
{
	return 0;
}

int MRP_PCMSource::PeaksBuild_Run()
{
	return 0;
}

void MRP_PCMSource::PeaksBuild_Finish()
{
}

int MRP_PCMSource::Extended(int call, void *parm1, void *parm2, void *parm3)
{
	return 0;
}
