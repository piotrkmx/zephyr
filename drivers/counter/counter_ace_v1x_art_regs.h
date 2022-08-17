/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __COUNTER_ACE_V1X_ART_REGS__
#define __COUNTER_ACE_V1X_ART_REGS__

#if CONFIG_ACE_V1X_ART_COUNTER

struct DfTSCTRL {
	union {
		uint32_t full;
		struct {
			uint32_t    cdmas :  5;
			uint32_t     odts :  1;
			uint32_t     lwcs :  1;
			uint32_t    hhtse :  1;
			uint32_t    rsvd9 :  2;
			uint32_t    clnks :  2;
			uint32_t    dmats :  2;
			uint32_t   rsvd29 : 16;
			uint32_t    ionte :  1;
			uint32_t      ntk :  1;
		} part;
	} u;
};

union DfDWCCS {
	uint64_t full;
	struct {
		uint64_t      dwc : 64;
	} part;
};

union DfARTCS {
	uint64_t full;
	struct {
		uint32_t	high : 32;
		uint32_t	low : 32;
	} part;
};

#define ACE_ART_COUNTER_ID DT_NODELABEL(ace_art_counter)

#define ACE_ARTCS	(DT_REG_ADDR(ACE_ART_COUNTER_ID))

#endif

#endif /*__COUNTER_ACE_V1X_ART_REGS__*/