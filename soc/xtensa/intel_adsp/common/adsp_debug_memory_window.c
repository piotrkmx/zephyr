// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2022 Intel Corporation. All rights reserved.
//
// Author: Piotr Kmiecik <piotrx.kmiecik@intel.com>

#include <zephyr/arch/xtensa/cache.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/mm/system_mm.h>
#include <zephyr/kernel.h>
#include <soc.h>
#include <adsp_memory.h>
#include <adsp_debug_window.h>
#include <errno.h>


#include <mem_window.h>

// z_xtensa_cache_flush

#define NELEMENTS(arr) (sizeof(arr) / sizeof(arr[0]))
static struct device *debug_memory_window_device;

__attribute__((optimize("-O0")))
static int debug_memory_window_get_buffer (void* buffer_desc)
{
	uint32_t *buf = 0;

	//(uint32_t *)(z_soc_uncached_ptr((void *)(WIN2_MBASE + WIN2_OFFSET)));
	volatile static int stop2xx1 = 1;
	while (stop2xx1);

	const struct mem_win_config *config = debug_memory_window_device->config;
	buf = z_soc_uncached_ptr((__sparse_force void __sparse_cache *)config->mem_base);

	if (buf)
	{
		((byte_array_t*)buffer_desc)->data = (uint32_t*)buf;
		((byte_array_t*)buffer_desc)->size = sizeof(struct adsp_debug_window);
		return ADSP_SUCCESS;
	}

	return ADSP_MM_HW_BUFFER_NOT_ALLOCATED;
}

__attribute__((optimize("-O0")))
int debug_memory_window_init(const struct device *d)
{
    int err = 0;
    debug_memory_window_device = d;

	volatile static int stop2xx = 1;
	while (stop2xx);

    byte_array_t debug_memory_window;
    err = debug_memory_window_get_buffer(&debug_memory_window);

    if (err!=0)
        return err;

    err = sys_mm_drv_map_region(debug_memory_window.data, 0,
				 ADSP_DW_PAGE_SIZE, SYS_MM_MEM_CACHE_WB);

    memset(debug_memory_window.data, 0, ADSP_DW_PAGE_SIZE);
    z_xtensa_cache_flush_inv(debug_memory_window.data,ADSP_DW_PAGE_SIZE);

    if (err !=0)
        return err;

#if CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
    err = sys_mm_drv_mirror_region(debug_memory_window.data,
            debug_memory_window.size - ADSP_DW_PAGE_SIZE, debug_memory_window.data + ADSP_DW_PAGE_SIZE, 0);
    if (err !=0)
        return err;
#endif // CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB

    return 0;
}

__attribute__((optimize("-O0")))
int debug_memory_window_map_slot(uint32_t resource_id, void* virtual_memory_address,
				uint32_t slot_id)
{
#if CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB

	uint32_t err = 0;
	byte_array_t buf;

    err = debug_memory_window_get_buffer(&buf);
	z_xtensa_cache_flush_inv(&buf, sizeof(struct adsp_debug_window));
	struct adsp_debug_window *debug_memory_window;
    debug_memory_window = (struct adsp_debug_window*)(z_soc_uncached_ptr(buf.data));

    size_t first_free = NELEMENTS(debug_memory_window->descs);

    for (size_t idx = 0; idx < NELEMENTS(debug_memory_window->descs); ++idx)
    {
        if (debug_memory_window->descs[idx].vma == (uint32_t)virtual_memory_address &&
            debug_memory_window->descs[idx].resource_id == resource_id &&
            debug_memory_window->descs[idx].type == slot_id)
        {
            return ADSP_SUCCESS;
        }
        if (first_free == NELEMENTS(debug_memory_window->descs) &&
            debug_memory_window->descs[idx].vma == 0)
        {
            first_free = idx;
        }
    }

    if (first_free == NELEMENTS(debug_memory_window->descs))
        return ADSP_NOT_FOUND;

    //__assert(debug_memory_window->descs[first_free].type == 0);
    //__assert(debug_memory_window->descs[first_free].resource_id == 0);

    uint8_t* slot_address = debug_memory_window->slots[first_free];

    int ec = sys_mm_drv_mirror_region(virtual_memory_address, ADSP_DW_PAGE_SIZE, slot_address, 0);
    if (ec != ADSP_SUCCESS)
    {
		return ADSP_MM_L2HP_CANNOT_MIRROR;
    }

    debug_memory_window->descs[first_free].type = slot_id;
    debug_memory_window->descs[first_free].vma = (uint32_t)virtual_memory_address;
    debug_memory_window->descs[first_free].resource_id = resource_id;

    return ADSP_SUCCESS;
#else // CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
    return ADSP_SUCCESS;
#endif // CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
}

int debug_memory_window_unmap_slot(uint32_t resource_id, void* virtual_memory_address,
            uint32_t slot_id)
{
#if 0
/*
    RETURN_EC_ON_FAIL(IS_ALIGNED(virtual_memory_address, ADSP_DW_PAGE_SIZE),
        ADSP_INVALID_ALIGNMENT);
*/
	assert(get_prid() == MASTER_CORE_ID);

#if CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
    struct adsp_debug_window* const debug_memory_window = SRAM_TO_SRAM_ALIAS(s_debug_memory_window);

    // search the same page_index mapped for memory window address
    for (size_t idx = 0; idx < NELEMENTS(debug_memory_window->descs); ++idx)
    {
        if (debug_memory_window->descs[idx].resource_id != resource_id) continue;
        if (debug_memory_window->descs[idx].vma != (uint32_t)virtual_memory_address) continue;
        if (debug_memory_window->descs[idx].type != slot_id) continue;

        uint8_t* base_address = (uint8_t*)(s_debug_memory_window);
        uint8_t* slot_address = s_debug_memory_window->slots[idx];
        // reset VMA
        int ec = sys_mm_drv_mirror_region(base_address, ADSP_DW_PAGE_SIZE, slot_address, 0);
        if (ec != ADSP_SUCCESS)
            return ADSP_MM_L2HP_CANNOT_MIRROR;

        debug_memory_window->descs[idx].slot_id = SLOT_UNUSED;
        debug_memory_window->descs[idx].vma = 0;
        debug_memory_window->descs[idx].resource_id = 0;
        return ADSP_SUCCESS;
    }
    return ADSP_NOT_FOUND;
#else // CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
    return ADSP_SUCCESS;
#endif // CONFIG_MM_DRV_INTEL_ADSP_MTL_TLB
#endif // 0
    return 0;
}

