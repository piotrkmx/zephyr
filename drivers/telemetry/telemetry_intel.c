/*
 * Copyright (c) 2022 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "telemetry_intel.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/mm/system_mm.h>

__aligned(CONFIG_MM_DRV_PAGE_SIZE)uint8_t telemetry_data_buffer[CONFIG_MM_DRV_PAGE_SIZE];
struct byte_array shared_telemetry_buffer;

void telemetry_init(void)
{
	shared_telemetry_buffer.data = z_soc_cached_ptr(&telemetry_data_buffer);
	shared_telemetry_buffer.size = CONFIG_MM_DRV_PAGE_SIZE;

	memset(shared_telemetry_buffer.data, 0, shared_telemetry_buffer.size);
	z_xtensa_cache_flush_inv(shared_telemetry_buffer.data, shared_telemetry_buffer.size);

	struct TelemetryWndData* telemetry_data = (struct TelemetryWndData*)(shared_telemetry_buffer.data);
	telemetry_data->separator_1 = 'T';
	telemetry_data->separator_2 = 'E';
	telemetry_data->separator_3 = 'L';
	telemetry_data->separator_4 = 'E';
	telemetry_data->separator_5 = 'M';
	telemetry_data->separator_6 = 'E';
	telemetry_data->separator_7 = 'T';
	telemetry_data->separator_8 = 'R';
	telemetry_data->separator_9 = 'Y';
	z_xtensa_cache_flush_inv(shared_telemetry_buffer.data, shared_telemetry_buffer.size);

}

struct byte_array telemetry_get_buffer(void)
{
	return shared_telemetry_buffer;
}

struct TelemetryWndData *telemetry_get_data(void)
{
	struct TelemetryWndData *telemetry_data =
			(struct TelemetryWndData *)(shared_telemetry_buffer.data);
	return telemetry_data;
}
