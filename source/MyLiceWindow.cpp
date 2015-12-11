#include "MyLiceWindow.h"

void TestControl::paint(LICE_IBitmap * bm)
{
	LICE_FillRect(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(0, 0, 0, 255));
	LICE_Line(bm, 0, 0, bm->getWidth(), bm->getHeight(), LICE_RGBA(255, 255, 255, 255));
}
