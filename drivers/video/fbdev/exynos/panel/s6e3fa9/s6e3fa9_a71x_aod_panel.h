/*
 * linux/drivers/video/fbdev/exynos/panel/s6e3fa9/s6e3fa9_a71x_aod_panel.h
 *
 * Header file for AOD Driver
 *
 * Copyright (c) 2016 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __S6E3FA9_A71X_AOD_PANEL_H__
#define __S6E3FA9_A71X_AOD_PANEL_H__

#include "s6e3fa9_aod.h"
#include "s6e3fa9_aod_panel.h"

#include "s6e3fa9_a71x_self_mask_img.h"

/* A71X */
static DEFINE_STATIC_PACKET(s6e3fa9_a71x_self_mask_img_pkt, DSI_PKT_TYPE_WR_SR, A71X_SELF_MASK_IMG, 0);

static void *s6e3fa9_a71x_self_mask_img_cmdtbl[] = {
	&KEYINFO(s6e3fa9_aod_l2_key_enable),
	&PKTINFO(s6e3fa9_self_mask_sd_path),
	&PKTINFO(s6e3fa9_a71x_self_mask_img_pkt),
	&PKTINFO(s6e3fa9_reset_sd_path),
	&KEYINFO(s6e3fa9_aod_l2_key_disable),
};

static struct seqinfo s6e3fa9_a71x_aod_seqtbl[MAX_AOD_SEQ] = {
	[SELF_MASK_IMG_SEQ] = SEQINFO_INIT("self_mask_img", s6e3fa9_a71x_self_mask_img_cmdtbl),
	[SELF_MASK_ENA_SEQ] = SEQINFO_INIT("self_mask_ena", s6e3fa9_self_mask_ena_cmdtbl),
	[SELF_MASK_DIS_SEQ] = SEQINFO_INIT("self_mask_dis", s6e3fa9_self_mask_dis_cmdtbl),
	[SELF_MASK_CHECKSUM_SEQ] = SEQINFO_INIT("self_mask_checksum", s6e3fa9_self_mask_checksum_cmdtbl),
	[SELF_MASK_GC_ON_SEQ] = SEQINFO_INIT("green_circle_on", s6e3fa9_green_circle_on_cmdtbl),
	[SELF_MASK_GC_OFF_SEQ] = SEQINFO_INIT("green_circle_off", s6e3fa9_green_circle_off_cmdtbl),

	[ENTER_AOD_SEQ] = SEQINFO_INIT("enter_aod", s6e3fa9_aod_enter_aod_cmdtbl),
	[EXIT_AOD_SEQ] = SEQINFO_INIT("exit_aod", s6e3fa9_aod_exit_aod_cmdtbl),
};

static struct aod_tune s6e3fa9_a71x_aod = {
	.name = "s6e3fa9_a71x_aod",
	.nr_seqtbl = ARRAY_SIZE(s6e3fa9_a71x_aod_seqtbl),
	.seqtbl = s6e3fa9_a71x_aod_seqtbl,
	.nr_maptbl = ARRAY_SIZE(s6e3fa9_aod_maptbl),
	.maptbl = s6e3fa9_aod_maptbl,
	.self_mask_en = true,
};
#endif //__S6E3FA9_A71X_AOD_PANEL_H__
