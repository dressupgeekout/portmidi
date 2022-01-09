/* pmnetbsd.c -- PortMidi os-dependent code */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/drvctlio.h>
#include <sys/ioctl.h>
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
	pm_initialized = FALSE;
	/* XXX Iterate over the MIDI input devices -- foreach, register the device w/
	 * PortMidi */

	/* Iterate over the MIDI output devices -- foreach, register the device w/
	 * PortMidi -- for now assume there's only 1 */
	const char *interf = "NetBSD";
	const char *name = "NAME"; /* but the name comes from USB */
	int is_input = 0;
	int is_virtual = 0;

	/*
	 * Try to use drvctl to determine which MIDI devices are connected
	 */
	int drvctl = open("/dev/drvctl", O_RDWR);

	if (drvctl < 0) {
		fprintf(stderr, "ERROR opening /dev/drvctl: %s\n", strerror(errno));
		return;
	}

	/* Figure out how many children we actually have */
	struct devlistargs args = {
		.l_devname = "",
		.l_childname = NULL,
		.l_children = 0
	};

	ioctl(drvctl, DRVLISTDEV, &args);

	/*
	 * Now we know how big to allocate our buffer of childnames.
	 *
	 * XXX we assume all MIDI devices are actually USB MIDI devices :( Really
	 * what I want is a list of all /dev/rmidi* which are currently attached.
	 */
	size_t n_children = args.l_children;
	args.l_childname = malloc(n_children * sizeof(args.l_childname[0]));
	snprintf(args.l_devname, sizeof(args.l_devname), "%s", "umidi0");

	/* OK now do it for real */
	if (ioctl(drvctl, DRVLISTDEV, &args) < 0) {
		fprintf(stderr, "ERROR in DRVLISTDEV: %s\n", strerror(errno));
		close(drvctl);
		return;
	}

	char real_path[PATH_MAX];
	int fd;

	for (int i = 0; i < args.l_children; i++) {
		snprintf(real_path, sizeof(real_path), "/dev/r%s", args.l_childname[i]);
		fd = open(real_path, O_WRONLY);
		pm_add_device((char *)interf, name, is_input, is_virtual, (void *)(intptr_t)fd, &pm_out_dictionary);
	}

	close(drvctl);
	pm_initialized = TRUE;
}

void
pm_term(void)
{
}
