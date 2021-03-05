/*
 * copyright (c) 2019 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Core file for Samsung EXYNOS TSMUX driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef EXYNOS9630_CMU_MFC_H
#define EXYNOS9630_CMU_MFC_H

/* CMU_MFC_SFR is for debugging */

#define TSMUX_CMU_MFC_DESC(_sfr_name, _base_pa, _offset) {	\
	.sfr_name = _sfr_name,	\
	.base_pa = _base_pa,	\
	.offset = _offset,	\
}

struct tsmux_cmu_mfc_sfr {
	char *sfr_name;
	phys_addr_t base_pa;
	u32 offset;
};

struct tsmux_cmu_mfc_sfr tsmux_cmu_mfc_sfr_list[] = {
	TSMUX_CMU_MFC_DESC("CLK_CON_DIV_DIV_CLK_MFC_BUSP", 0x12E00000, 0x1800),
	TSMUX_CMU_MFC_DESC("PLL_CON0_MUX_CLKCMU_MFC_WFD_USER", 0x12E00000, 0x610),
	TSMUX_CMU_MFC_DESC("PLL_CON0_MUX_CLKCMU_MFC_MFC_USER", 0x12E00000, 0x600),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_WFD_QCH", 0x12E00000, 0x7060),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_VGEN_MFC_QCH", 0x12E00000, 0x705c),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_SYSREG_MFC_QCH", 0x12E00000, 0x7058),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_SYSMMU_MFCD1_QCH_S2", 0x12E00000, 0x7054),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_SYSMMU_MFCD1_QCH_S1", 0x12E00000, 0x7050),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_SYSMMU_MFCD0_QCH_S2", 0x12E00000, 0x704c),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_SYSMMU_MFCD0_QCH_S1", 0x12E00000, 0x7048),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_RSTNSYNC_CLK_MFC_BUSD_WFD_SW_RESET_QCH", 0x12E00000, 0x7044),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_RSTNSYNC_CLK_MFC_BUSD_MFC_SW_RESET_QCH", 0x12E00000, 0x7040),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_RSTNSYNC_CLK_MFC_BUSD_LH_ATB_MFC_SI_SW_RESET_QCH", 0x12E00000, 0x703c),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_RSTNSYNC_CLK_MFC_BUSD_LH_ATB_MFC_MI_SW_RESET_QCH", 0x12E00000, 0x7038),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_PPMU_WFD_QCH", 0x12E00000, 0x7034),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_PPMU_MFCD1_QCH", 0x12E00000, 0x7030),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_PPMU_MFCD0_QCH", 0x12E00000, 0x702c),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_MFC_QCH", 0x12E00000, 0x7028),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_MFC_CMU_MFC_QCH", 0x12E00000, 0x7024),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_LH_ATB_MFC_QCH_SI", 0x12E00000, 0x7020),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_LH_ATB_MFC_QCH_MI", 0x12E00000, 0x701c),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_LHS_AXI_D1_MFC_QCH", 0x12E00000, 0x7018),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_LHS_AXI_D0_MFC_QCH", 0x12E00000, 0x7014),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_LHM_AXI_P_MFC_QCH", 0x12E00000, 0x7010),
	TSMUX_CMU_MFC_DESC("DBG_NFO_QCH_CON_D_TZPC_MFC_QCH", 0x12E00000, 0x700c),
	TSMUX_CMU_MFC_DESC("QCH_CON_WFD_QCH", 0x12E00000, 0x3060),
	TSMUX_CMU_MFC_DESC("QCH_CON_VGEN_MFC_QCH", 0x12E00000, 0x305c),
	TSMUX_CMU_MFC_DESC("QCH_CON_SYSREG_MFC_QCH", 0x12E00000, 0x3058),
	TSMUX_CMU_MFC_DESC("QCH_CON_SYSMMU_MFCD1_QCH_S2", 0x12E00000, 0x3054),
	TSMUX_CMU_MFC_DESC("QCH_CON_SYSMMU_MFCD1_QCH_S1", 0x12E00000, 0x3050),
	TSMUX_CMU_MFC_DESC("QCH_CON_SYSMMU_MFCD0_QCH_S2", 0x12E00000, 0x304c),
	TSMUX_CMU_MFC_DESC("QCH_CON_SYSMMU_MFCD0_QCH_S1", 0x12E00000, 0x3048),
	TSMUX_CMU_MFC_DESC("QCH_CON_RSTNSYNC_CLK_MFC_BUSD_WFD_SW_RESET_QCH", 0x12E00000, 0x3044),
	TSMUX_CMU_MFC_DESC("QCH_CON_RSTNSYNC_CLK_MFC_BUSD_MFC_SW_RESET_QCH", 0x12E00000, 0x3040),
	TSMUX_CMU_MFC_DESC("QCH_CON_RSTNSYNC_CLK_MFC_BUSD_LH_ATB_MFC_SI_SW_RESET_QCH", 0x12E00000, 0x303c),
	TSMUX_CMU_MFC_DESC("QCH_CON_RSTNSYNC_CLK_MFC_BUSD_LH_ATB_MFC_MI_SW_RESET_QCH", 0x12E00000, 0x3038),
	TSMUX_CMU_MFC_DESC("QCH_CON_PPMU_WFD_QCH", 0x12E00000, 0x3034),
	TSMUX_CMU_MFC_DESC("QCH_CON_PPMU_MFCD1_QCH", 0x12E00000, 0x3030),
	TSMUX_CMU_MFC_DESC("QCH_CON_PPMU_MFCD0_QCH", 0x12E00000, 0x302c),
	TSMUX_CMU_MFC_DESC("QCH_CON_MFC_QCH", 0x12E00000, 0x3028),
	TSMUX_CMU_MFC_DESC("QCH_CON_MFC_CMU_MFC_QCH", 0x12E00000, 0x3024),
	TSMUX_CMU_MFC_DESC("QCH_CON_LH_ATB_MFC_QCH_SI", 0x12E00000, 0x3020),
	TSMUX_CMU_MFC_DESC("QCH_CON_LH_ATB_MFC_QCH_MI", 0x12E00000, 0x301c),
	TSMUX_CMU_MFC_DESC("QCH_CON_LHS_AXI_D1_MFC_QCH", 0x12E00000, 0x3018),
	TSMUX_CMU_MFC_DESC("QCH_CON_LHS_AXI_D0_MFC_QCH", 0x12E00000, 0x3014),
	TSMUX_CMU_MFC_DESC("QCH_CON_LHM_AXI_P_MFC_QCH", 0x12E00000, 0x3010),
	TSMUX_CMU_MFC_DESC("QCH_CON_D_TZPC_MFC_QCH", 0x12E00000, 0x300c),
	TSMUX_CMU_MFC_DESC("MFC_CMU_MFC_CONTROLLER_OPTION", 0x12e00000, 0x0800),
	TSMUX_CMU_MFC_DESC("SYSREG_MFC_BUS_COMPONENT_DRCG_EN", 0x12e20000, 0x0104),
};

u32 tsmux_cmu_mfc_sfr_list_size = ARRAY_SIZE(tsmux_cmu_mfc_sfr_list);

#endif
