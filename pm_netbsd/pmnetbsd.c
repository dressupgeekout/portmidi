/* pmnetbsd.c -- PortMidi os-dependent code */

#include <stdlib.h>
#include <sys/midiio.h>

#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"

static PmError midi_abort(PmInternal *midi);
static PmError midi_in_close(PmInternal *midi);
static PmError midi_in_open(PmInternal *midi, void *driverInfo);
static PmError midi_out_close(PmInternal *midi);
static PmError midi_out_open(PmInternal *midi, void *driverInfo);
static unsigned int midi_check_host_error(PmInternal *midi);

pm_fns_node pm_netbsd_in_dictionary = {
	.write_short = none_write_short,
	.begin_sysex = none_sysex,
	.end_sysex = none_sysex,
	.write_byte = none_write_byte,
	.write_realtime = none_write_short,
	.synchronize = none_synchronize,
	.open = midi_in_open,
	.abort = midi_abort,
	.close = midi_in_close,
	.poll = none_poll,
	.check_host_error = midi_check_host_error
};

pm_fns_node pm_out_dictionary = {
	.write_short = none_write_short,
	.begin_sysex = none_sysex,
	.end_sysex = none_sysex,
	.write_byte = none_write_byte,
	.write_realtime = none_write_short,
	.synchronize = none_synchronize,
	.open = midi_out_open,
	.abort = midi_abort,
	.close = midi_in_close,
	.poll = none_poll,
	.check_host_error = midi_check_host_error
};

/* ********** */

static PmError
midi_abort(PmInternal *midi)
{
	return pmNoError;
}

static PmError
midi_in_close(PmInternal *midi)
{
	return pmNoError;
}

static PmError
midi_in_open(PmInternal *midi, void *driverInfo)
{
	return pmNoError;
}

static PmError
midi_out_close(PmInternal *midi)
{
	return pmNoError;
}

static PmError
midi_out_open(PmInternal *midi, void *driverInfo)
{
	return pmNoError;
}

static unsigned int
midi_check_host_error(PmInternal *midi)
{
	return FALSE;
}

/* ********** */

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
