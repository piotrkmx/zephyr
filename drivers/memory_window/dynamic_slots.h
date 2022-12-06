/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _DYNAMIC_SLOTS_H_
#define _DYNAMIC_SLOTS_H_

#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>

#include <mem_window.h>

#define MEMORY_WINDOW_SLOT_SIZE     CONFIG_MM_DRV_PAGE_SIZE
#define MEMORY_WINDOW_SLOT_INVALID_RESOURCE_ID    0xffffffff
#define MEMORY_WINDOW_SLOTS_COUNT   7

enum slot_ids {
	/*! Unused slot. */
	SLOT_UNUSED       = 0x00000000,
	/*! CRT: Critical & Warning log buffer. */
	SLOT_CRITICAL_LOG = 0x54524300,
	/*! LOG: Log buffer, must be enabled by IPC. Last byte set to Core ID. */
	SLOT_DEBUG_LOG    = 0x474f4c00,
	/*! GDB: Communication with GDB. */
	SLOT_GDB_STUB     = 0x42444700,
	/*! TEL: Data for telemetry. */
	SLOT_TELEMETRY    = 0x4c455400,
	SLOT_BROKEN       = 0x44414544
};

struct slot_desc {
	uint32_t resource_id;
	uint32_t slot_id;
	uint32_t vma;
} __packed __aligned(4);

struct memory_window_struct {
	struct slot_desc slot_descs[MEMORY_WINDOW_SLOTS_COUNT];
	uint8_t rsvd[MEMORY_WINDOW_SLOT_SIZE -
	    MEMORY_WINDOW_SLOTS_COUNT * sizeof(struct slot_desc)];
	uint8_t slots[MEMORY_WINDOW_SLOTS_COUNT][MEMORY_WINDOW_SLOT_SIZE];
} __packed __aligned(4);

int dynamic_slots_init(const struct device *dev);
int dynamic_slots_map_slot(const struct device *dev, uint32_t resource_id,
		void *virtual_memory_address, uint32_t slot_id);
int dynamic_slots_unmap_slot(const struct device *dev, uint32_t resource_id,
		void *virtual_memory_address, uint32_t slot_id);

#endif /* _DYNAMIC_SLOTS_H_ */
