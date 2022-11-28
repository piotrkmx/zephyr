/* Copyright(c) 2022 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/xtensa/cache.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <errno.h>
#include <soc.h>

#include <mem_window.h>
#include <adsp_debug_window.h>

int boot_complete(const struct device *d)
{
	ARG_UNUSED(d);
	uint32_t *win;
	const struct mem_win_config *config;
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(mem_window0));

	if (!device_is_ready(dev)) {
		volatile static int stop5xx = 1;
		while (stop5xx);
		return -ENODEV;
	}
	config = dev->config;

	uint32_t mem_base = config->mem_base;
	win = z_soc_uncached_ptr((__sparse_force void __sparse_cache *)config->mem_base);
	/* Software protocol: "firmware entered" has the value 5 */
	win[0] = 5;

	const struct device *dev_2 = DEVICE_DT_GET(DT_NODELABEL(mem_window2));

	if (!device_is_ready(dev_2)) {
		volatile static int stop7xx = 1;
		while (stop7xx);
		return -ENODEV;
	}

	debug_memory_window_init(dev_2);

	return 0;
}

SYS_INIT(boot_complete, EARLY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
