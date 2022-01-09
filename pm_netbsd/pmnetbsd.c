/* pmnetbsd.c -- PortMidi os-dependent code */

#include <stdlib.h>

#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"

void *
pm_alloc(size_t s)
{
	return malloc(s);
}

void
pm_free(void *ptr)
{
	free(ptr);
}

void
pm_init(void)
{
	pm_initialized = TRUE;
}

void
pm_term(void)
{
}
