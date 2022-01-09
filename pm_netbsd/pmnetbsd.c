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
#include "pmnetbsd.h"

PmDeviceID pm_default_input_device_id = -1;
PmDeviceID pm_default_output_device_id = -1;

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

pm_fns_node pm_in_dictionary = {
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

PmDeviceID
Pm_GetDefaultInputDeviceID()
{
    Pm_Initialize();
    return pm_default_input_device_id;
}

PmDeviceID
Pm_GetDefaultOutputDeviceID()
{
    Pm_Initialize();
    return pm_default_output_device_id;
}

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

	/*
	 * We're going to use drvctl(4) to determine which MIDI devices are
	 * connected.
	 */
	int drvctl = open("/dev/drvctl", O_RDWR);

	if (drvctl < 0) {
		fprintf(stderr, "ERROR opening /dev/drvctl: %s\n", strerror(errno));
		return;
	}

	/* Figure out how many children we actually have, first. */
	struct devlistargs args = {
		.l_devname = "",
		.l_childname = NULL,
		.l_children = 0
	};

	ioctl(drvctl, DRVLISTDEV, &args);

	/*
	 * Now we know how big to allocate our buffer of childnames. Then we can find
	 * the list of children for real.
	 *
	 * XXX we assume all MIDI devices are actually USB MIDI devices :( Really
	 * what I want is a list of all /dev/rmidi* which are currently attached.
	 */
	size_t n_children = args.l_children;
	args.l_childname = malloc(n_children * sizeof(args.l_childname[0]));
	snprintf(args.l_devname, sizeof(args.l_devname), "%s", "umidi0");

	if (ioctl(drvctl, DRVLISTDEV, &args) < 0) {
		fprintf(stderr, "ERROR in DRVLISTDEV: %s\n", strerror(errno));
		close(drvctl);
		return;
	}

	/*
	 * Register each device we've found w/ PortMidi.
	 */

	char real_path[PATH_MAX];
	int in_fd;
	int out_fd;

	/* XXX Assume all devices are ok for in AND out. */
	// XXX the PortMidi `name` should be like what you find from USB HID
	/* The first device we see is the 'default'. */

	/* Input devices */
	for (int i = 0; i < args.l_children; i++) {
		memset(real_path, 0, sizeof(real_path));
		snprintf(real_path, sizeof(real_path), "/dev/r%s", args.l_childname[i]);
		in_fd = open(real_path, O_RDONLY); // XXX check for error
		PmDeviceID id = pm_add_device("NetBSD", args.l_childname[i], 1, 0, (void *)(intptr_t)in_fd, &pm_in_dictionary);

		if (pm_default_input_device_id == -1) {
			pm_default_input_device_id = id;
		}
	}

	/* Output devices */
	for (int i = 0; i < args.l_children; i++) {
		memset(real_path, 0, sizeof(real_path));
		snprintf(real_path, sizeof(real_path), "/dev/r%s", args.l_childname[i]);
		out_fd = open(real_path, O_WRONLY); // XXX check for error
		PmDeviceID id = pm_add_device("NetBSD", args.l_childname[i], 0, 0, (void *)(intptr_t)out_fd, &pm_out_dictionary);

		if (pm_default_output_device_id == -1) {
			pm_default_output_device_id = id;
		}
	}

	free(args.l_childname);
	close(drvctl);
	pm_initialized = TRUE;
}

void
pm_term(void)
{
}
