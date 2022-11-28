/*
 * Copyright (c) 2022 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_SOC_INTEL_ADSP_MEMORY_H_
#define ZEPHYR_SOC_INTEL_ADSP_MEMORY_H_


#include <zephyr/devicetree.h>
#include <adsp-vectors.h>
#include <mem_window.h>



#define L2_SRAM_BASE (DT_REG_ADDR(DT_NODELABEL(sram0)))
#define L2_SRAM_SIZE (DT_REG_SIZE(DT_NODELABEL(sram0)))

#define LP_SRAM_BASE (DT_REG_ADDR(DT_NODELABEL(sram1)))
#define LP_SRAM_SIZE (DT_REG_SIZE(DT_NODELABEL(sram1)))

#define ROM_JUMP_ADDR (LP_SRAM_BASE + 0x10)

/* Linker-usable RAM region */
#define RAM_BASE (L2_SRAM_BASE + CONFIG_HP_SRAM_RESERVE + VECTOR_TBL_SIZE)
#define RAM_SIZE (L2_SRAM_SIZE - CONFIG_HP_SRAM_RESERVE - VECTOR_TBL_SIZE)

/* The rimage tool produces two blob addresses we need to find: one is
 * our bootloader code block which starts at its entry point, the
 * other is the "manifest" containing the HP-SRAM data to unpack,
 * which appears 24k earlier in the DMA'd file, and thus in IMR
 * memory.  There's no ability to change this offset, it's a magic
 * number from rimage we simply need to honor.
 */

#define IMR_BOOT_LDR_DATA_BASE          (0xA1048000+0x1000)
#define IMR_BOOT_LDR_MANIFEST_BASE      0xA1042000
#define IMR_BOOT_LDR_TEXT_ENTRY_BASE (IMR_BOOT_LDR_MANIFEST_BASE + 0x6000)

#define ADSP_L1_CACHE_PREFCTL_VALUE 0x1038

/* L1 init */
#define ADSP_L1CC_ADDR                       (0x1FE80080)
#define ADSP_CxL1CCAP_ADDR                   (ADSP_L1CC_ADDR + 0x0000)
#define ADSP_CxL1CCFG_ADDR                   (ADSP_L1CC_ADDR + 0x0004)
#define ADSP_CxL1PCFG_ADDR                   (ADSP_L1CC_ADDR + 0x0008)

#if (!defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__))

#define ADSP_CxL1CCAP_REG   (*(volatile uint32_t *)(ADSP_CxL1CCAP_ADDR))
#define ADSP_CxL1CCFG_REG   (*(volatile uint32_t *)(ADSP_CxL1CCFG_ADDR))
#define ADSP_CxL1PCFG_REG   (*(volatile uint32_t *)(ADSP_CxL1PCFG_ADDR))

#endif  /* (!defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)) */

/* The number of set associative cache way supported on L1 Data Cache */
#define ADSP_CxL1CCAP_DCMWC ((ADSP_CxL1CCAP_REG >> 16) & 7)
/* The number of set associative cache way supported on L1 Instruction Cache */
#define ADSP_CxL1CCAP_ICMWC ((ADSP_CxL1CCAP_REG >> 20) & 7)

#ifndef _LINKER
/* L2 Local Memory Management */

/* These registers are for the L2 memory control and status. */

enum errorCode {
    ADSP_SUCCESS = 0,
    ADSP_NOT_FOUND = 9,
    ADSP_INVALID_ALIGNMENT = 22,
    ADSP_MM_VMA_NOT_LOCATED_IN_L2HP = 500,
    ADSP_MM_PROCAL_OVERRIDE_NOT_SUPPORTED = 501,
    ADSP_MM_INVALID_SIZE_FOR_DMA_ALLOCATION = 502,
    ADSP_MM_INVALID_ATTRIBUTE = 503,

    ADSP_MM_L2HP_INIT_ERROR = 510,
    ADSP_MM_L2HP_INVALID_ALIGNMENT = 520,
    ADSP_MM_L2LP_INVALID_ALIGNMENT = 521,

    ADSP_MM_INVALID_REQUEST_L2HP = 522,
    ADSP_MM_INVALID_REQUEST_L2LP = 523,
    ADSP_MM_INVALID_REQUEST_L2RF = 524,
    ADSP_MM_INVALID_REQUEST_DDR = 525,
    ADSP_MM_INVALID_REQUEST_EXTMEM = 526,
    ADSP_MM_INVALID_REQUEST_L1SRAM = 527,
    ADSP_MM_INVALID_REQUEST_SMEM = 528,

    ADSP_MM_DEADLOCK_ON_SWAP = 530,
    ADSP_MM_ADDRESS_NO_ALIGNED_ON_SWAP = 531,
    ADSP_MM_SIZE_NO_ALIGNED_ON_SWAP = 532,
    ADSP_MM_UNEXPECTED_CRITICAL_SECTION = 533,

    ADSP_MM_NOT_LOCATED_IN_L2RF = 560,          ///< Given address is not located in expected memory region (L2RF)
    ADSP_MM_NOT_LOCATED_IN_LPSRAM = 561,        ///< Given address is not located in expected memory region (LPSRAM)

    ADSP_MM_INVALID_L2HP_CONFIG = 562, ///< Given configuration is not valid.
    ADSP_MM_INVALID_L2LP_CONFIG = 563, ///< Given configuration is not valid.
    ADSP_MM_INVALID_L2RF_CONFIG = 564, ///< Given configuration is not valid.
    ADSP_MM_INVALID_EXTMEM_CONFIG = 565, ///< Given configuration is not valid.
    ADSP_MM_INVALID_DDR_CONFIG = 566, ///< Given configuration is not valid.
    ADSP_MM_INVALID_L1SRAM_CONFIG = 567, ///< Given configuration is not valid.

    ADSP_MM_L2HP_NOT_MAPPED = 568, ///< Cannot map memory.
    ADSP_MM_L2LP_NOT_MAPPED = 569, ///< Cannot map memory.
    ADSP_MM_L2RF_NOT_MAPPED = 570, ///< Cannot map memory.
    ADSP_MM_EXTMEM_NOT_MAPPED = 571, ///< Cannot map memory.
    ADSP_MM_DDR_NOT_MAPPED = 572, ///< Cannot map memory.
    ADSP_MM_L1SRAM_NOT_MAPPED = 573, ///< Cannot map memory.

    ADSP_MM_CANNOT_REGISTER_MP = 574, ///< Cannot register memory pool.
    ADSP_MM_CANNOT_ALLOCATE_MP = 575, ///< Cannot allocate memory pool.

    ADSP_MM_CANNOT_ALLOCATE_L2HP = 576, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_ALLOCATE_L2LP = 577, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_ALLOCATE_L2RF = 578, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_ALLOCATE_EXTMEM = 579, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_ALLOCATE_DDR = 580, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_ALLOCATE_L1SRAM = 581, ///< Cannot allocate memory.

    ADSP_MM_CANNOT_FREE_L2HP = 582, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_L2LP = 583, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_L2RF = 584, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_EXTMEM = 585, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_DDR = 586, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_L1SRAM = 587, ///< Cannot allocate memory.

    ADSP_MM_L2HP_CANNOT_MIRROR = 588,  ///< Cannot mirror memory.

    ADSP_MM_HW_BUFFER_TOO_SMALL = 590,
    ADSP_MM_HW_BUFFER_NOT_ALLOCATED = 591,

    ADSP_MM_INVALID_SMEM_CONFIG = 592, ///< Given configuration is not valid.
    ADSP_MM_SMEM_NOT_MAPPED = 593, ///< Cannot map memory.
    ADSP_MM_CANNOT_ALLOCATE_SMEM = 594, ///< Cannot allocate memory.
    ADSP_MM_CANNOT_FREE_SMEM = 595, ///< Cannot free memory.
    ADSP_MM_SMEM_INIT_ERROR = 596
};

#define DFL2MM_REG 0x71d00

struct ace_l2mm {
	uint32_t l2mcap;
	uint32_t l2mpat;
	uint32_t l2mecap;
	uint32_t l2mecs;
	uint32_t l2hsbpmptr;
	uint32_t l2usbpmptr;
	uint32_t l2usbmrpptr;
	uint32_t l2ucmrpptr;
	uint32_t l2ucmrpdptr;
};

#define ACE_L2MM ((volatile struct ace_l2mm *)DFL2MM_REG)

/* DfL2MCAP */
struct ace_l2mcap {
	uint32_t l2hss  : 8;
	uint32_t l2uss  : 4;
	uint32_t l2hsbs : 4;
	uint32_t l2hs2s : 8;
	uint32_t l2usbs : 5;
	uint32_t l2se   : 1;
	uint32_t el2se  : 1;
	uint32_t rsvd32 : 1;
};

#define ACE_L2MCAP ((volatile struct ace_l2mcap *)DFL2MM_REG)

static ALWAYS_INLINE uint32_t ace_hpsram_get_bank_count(void)
{
	return ACE_L2MCAP->l2hss;
}

static ALWAYS_INLINE uint32_t ace_lpsram_get_bank_count(void)
{
	return ACE_L2MCAP->l2uss;
}

struct ace_hpsram_regs {
	/** @brief power gating control */
	uint8_t HSxPGCTL;
	/** @brief retention mode control */
	uint8_t HSxRMCTL;
	uint8_t reserved[2];
	/** @brief power gating status */
	uint8_t HSxPGISTS;
	uint8_t reserved1[3];
};
#endif

/* These registers are for the L2 HP SRAM bank power management control and status.*/
#define L2HSBPM_REG					0x17A800
#define L2HSBPM_REG_SIZE			0x0008

#define HPSRAM_REGS(block_idx)		((volatile struct ace_hpsram_regs *const) \
	(L2HSBPM_REG + L2HSBPM_REG_SIZE * (block_idx)))

#endif /* ZEPHYR_SOC_INTEL_ADSP_MEMORY_H_ */
