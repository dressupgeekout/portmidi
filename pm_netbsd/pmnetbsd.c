/* pmnetbsd.c -- PortMidi os-dependent code */


#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/midiio.h>

#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"

typedef struct netbsd_info_struct {
	char *charlotte; /* TESTING */
} netbsd_info_node, *netbsd_info_type;

static netbsd_info_type create_netbsd_info(void);

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

static netbsd_info_type
create_netbsd_info(void)
{
	netbsd_info_type info = (netbsd_info_type)pm_alloc(sizeof(netbsd_info_type));

	if (!info) {
		return NULL;
	}

	info->charlotte = malloc(128);
	if (!info->charlotte) {
		return NULL;
	}

	return info;
}

static PmError
midi_abort(PmInternal *midi)
{
	netbsd_info_type info = (netbsd_info_type)midi->api_info;
	free(info->charlotte);
	return pmNoError;
}

static PmError
midi_in_close(PmInternal *midi)
{
	netbsd_info_type info = (netbsd_info_type)midi->api_info;
	free(info->charlotte);
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
	netbsd_info_type info = create_netbsd_info();
	midi->api_info = info;
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
	/* XXX Iterate over the MIDI input devices -- foreach, register the device w/
	 * PortMidi */

	/* Iterate over the MIDI output devices -- foreach, register the device w/
	 * PortMidi -- for now assume there's only 1 */
	const char *interf = "NetBSD";
	const char *name = "NAME";
	int is_input = 0;
	int is_virtual = 0;
	int fd = open("/dev/midi1", O_WRONLY);
	pm_add_device((char *)interf, name, is_input, is_virtual, (void *)(intptr_t)fd, &pm_out_dictionary);

	pm_initialized = TRUE;
}

void
pm_term(void)
{
}
