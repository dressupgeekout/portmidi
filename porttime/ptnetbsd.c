/* ptnetbsd.c -- portable timer implementation for linux */

#include <stdint.h>

#include "porttime.h"

PtError
Pt_Start(int resolution, PtCallback *callback, void *userData)
{
	return ptNoError;
}

PtError
Pt_Stop(void)
{
	return ptNoError;
}

int
Pt_Started(void)
{
	return 0;
}

PtTimestamp
Pt_Time(void)
{
	return 0; /* XXX */
}

void
Pt_Sleep(int32_t duration)
{
	;
}
