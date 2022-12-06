/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dynamic_slots.h"

#include <zephyr/drivers/mm/system_mm.h>
#include <zephyr/arch/xtensa/cache.h>

int dynamic_slots_init(const struct device *dev)
{
	uint32_t buf_size = MEMORY_WINDOW_SLOT_SIZE * MEMORY_WINDOW_SLOTS_COUNT;
	const struct mem_win_config *config = dev->config;

	uint32_t *virt_buf = (uint32_t __sparse_cache *)
		arch_xtensa_cached_ptr((void *)(config->mem_base));

	uint32_t err = sys_mm_drv_map_region(virt_buf+(CONFIG_MM_DRV_PAGE_SIZE/sizeof(uint32_t)), 0,
			buf_size, SYS_MM_MEM_CACHE_WB);

	if (err != 0) {
		return err;
	}

	memset(virt_buf, 0, (buf_size+CONFIG_MM_DRV_PAGE_SIZE));
	z_xtensa_cache_flush_inv(virt_buf, (buf_size+CONFIG_MM_DRV_PAGE_SIZE));

	return 0;
}

int dynamic_slots_map_slot(const struct device *dev, uint32_t resource_id,
		void *virtual_memory_address, uint32_t slot_id)
{
#if CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
	const struct mem_win_config *config = dev->config;

	uint32_t *buf = (uint32_t __sparse_cache *)
		arch_xtensa_cached_ptr((void *)(config->mem_base));
	uint32_t buf_size =
		CONFIG_MM_DRV_PAGE_SIZE + MEMORY_WINDOW_SLOT_SIZE*MEMORY_WINDOW_SLOTS_COUNT;

	z_xtensa_cache_flush_inv(buf, buf_size);

	struct memory_window_struct *memory_window =
			(struct memory_window_struct *)(z_soc_uncached_ptr(buf));

	size_t first_free = ARRAY_SIZE(memory_window->slot_descs);

	for (size_t idx = 0; idx < ARRAY_SIZE(memory_window->slot_descs); ++idx) {
		if (memory_window->slot_descs[idx].vma == (uint32_t)virtual_memory_address &&
		    memory_window->slot_descs[idx].resource_id == resource_id &&
		    memory_window->slot_descs[idx].slot_id == slot_id) {
			return 0;
		}
		if (first_free == ARRAY_SIZE(memory_window->slot_descs) &&
			memory_window->slot_descs[idx].vma == 0) {
			first_free = idx;
		}
	}

	if (first_free == ARRAY_SIZE(memory_window->slot_descs)) {
		return -ENOBUFS;
	}

	__ASSERT(debug_memory_window->slot_descs[first_free].type == 0,
			"Error: Dynamic slot allocation failed. Invalid slot type.");
	__ASSERT(debug_memory_window->slot_descs[first_free].resource_id == 0,
			"Error: Dynamic slot allocation failed. Invalid resource id.");

	uint8_t *slot_address = memory_window->slots[first_free];

	int err = sys_mm_drv_mirror_region(virtual_memory_address,
				CONFIG_MM_DRV_PAGE_SIZE, slot_address, 0);
	if (err != 0)	{
		return err;
	}

	memory_window->slot_descs[first_free].slot_id = slot_id;
	memory_window->slot_descs[first_free].vma = (uint32_t)virtual_memory_address;
	memory_window->slot_descs[first_free].resource_id = resource_id;

	z_xtensa_cache_flush_inv((buf), CONFIG_MM_DRV_PAGE_SIZE);

	return 0;
#else /* CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB */
	return -ENOSYS;
#endif /* CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB */
}

int dynamic_slots_unmap_slot(const struct device *dev, uint32_t resource_id,
			void *virtual_memory_address, uint32_t slot_id)
{
#if CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
	const struct mem_win_config *config = dev->config;

	uint32_t *buf = (uint32_t __sparse_cache *)
		arch_xtensa_cached_ptr((void *)(config->mem_base));

	struct memory_window_struct *memory_window =
			(struct memory_window_struct *)(z_soc_uncached_ptr(buf));

	for (size_t idx = 0; idx < ARRAY_SIZE(memory_window->slot_descs); ++idx) {
		if (memory_window->slot_descs[idx].resource_id != resource_id) {
			continue;
		}
		if (memory_window->slot_descs[idx].vma != (uint32_t)virtual_memory_address) {
			continue;
		}
		if (memory_window->slot_descs[idx].slot_id != slot_id) {
			continue;
		}

		uint8_t *base_address = (uint8_t *)(memory_window);
		uint8_t *slot_address = memory_window->slots[idx];

		int err = sys_mm_drv_mirror_region(base_address,
				CONFIG_MM_DRV_PAGE_SIZE, slot_address, 0);

		if (err != 0) {
			return err;
		}

		memory_window->slot_descs[idx].slot_id = SLOT_UNUSED;
		memory_window->slot_descs[idx].vma = 0;
		memory_window->slot_descs[idx].resource_id = 0;
		return 0;
	}
	return -EINVAL;
#else /* CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB */
	return -ENOSYS;
#endif /* CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB */
}
