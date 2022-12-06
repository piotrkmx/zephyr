/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

#include <memory_window/dynamic_slots.h>
#include <telemetry/telemetry_intel.h>
#include <zephyr/logging/log_backend_adsp_mtrace.h>

#define DEBUG_MEMORY_WINDOW_INIT_PRIORITY 38

int debug_memory_window_init(const struct device *d)
{
	ARG_UNUSED(d);
	int err = 0;

	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(mem_window2));

	if (!device_is_ready(dev)) {
		return -ENODEV;
	}

	err = dynamic_slots_init(dev);
	if (err != 0) {
		return err;
	}

#if CONFIG_TELEMETRY_INTEL
	telemetry_init();
	err = dynamic_slots_map_slot(dev, 0, telemetry_get_buffer().data, SLOT_TELEMETRY);
	if (err != 0) {
		return err;
	}
#endif

#if CONFIG_LOG_BACKEND_ADSP_MTRACE
	mtrace_init();
	err = dynamic_slots_map_slot(dev, 0, mtrace_get_buffer().data, SLOT_DEBUG_LOG);
	if (err != 0) {
		return err;
	}
#endif

	return 0;
}

SYS_INIT(debug_memory_window_init, EARLY, DEBUG_MEMORY_WINDOW_INIT_PRIORITY);
