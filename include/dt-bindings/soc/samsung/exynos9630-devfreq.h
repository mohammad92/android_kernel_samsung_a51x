/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Device Tree binding constants for Exynos9630 devfreq
 */

#ifndef _DT_BINDINGS_EXYNOS_9630_DEVFREQ_H
#define _DT_BINDINGS_EXYNOS_9630_DEVFREQ_H
/* DEVFREQ TYPE LIST */
#define DEVFREQ_MIF		0
#define DEVFREQ_INT		1
#define DEVFREQ_DISP		2
#define DEVFREQ_CAM		3
#define DEVFREQ_INTCAM		4
#define DEVFREQ_AUD		5
#define DEVFREQ_MFC		6
#define DEVFREQ_NPU		7
#define DEVFREQ_DSP		8
#define DEVFREQ_TNR		9
#define DEVFREQ_DNC		10
#define DEVFREQ_TYPE_END	11

/* ESS FLAG LIST */
#define ESS_FLAG_INT	2
#define ESS_FLAG_MIF	3
#define ESS_FLAG_CAM	4
#define ESS_FLAG_DISP	5
#define ESS_FLAG_INTCAM	6
#define ESS_FLAG_AUD	7
#define ESS_FLAG_MFC	8
#define ESS_FLAG_NPU	9
#define ESS_FLAG_DSP	10
#define ESS_FLAG_TNR	11
#define ESS_FLAG_DNC	12
#define ESS_FLAG_G3D	13 /* G3D doesn't use DEVFREQ, but this value is added here for the consistency */

/* DEVFREQ GOV TYPE */
#define SIMPLE_INTERACTIVE 0

/* PM QOS related variable */
#define	DT_PM_QOS_RESERVED			0
#define	DT_PM_QOS_CPU_DMA_LATENCY		1
#define	DT_PM_QOS_NETWORK_LATENCY		2
#define	DT_PM_QOS_CLUSTER0_FREQ_MIN		3
#define	DT_PM_QOS_CLUSTER0_FREQ_MAX		4
#define	DT_PM_QOS_CLUSTER1_FREQ_MIN		5
#define	DT_PM_QOS_CLUSTER1_FREQ_MAX		6
#define	DT_PM_QOS_CPU_ONLINE_MIN		7
#define	DT_PM_QOS_CPU_ONLINE_MAX		8
#define	DT_PM_QOS_DEVICE_THROUGHPUT		9
#define	DT_PM_QOS_INTCAM_THROUGHPUT		10
#define	DT_PM_QOS_DEVICE_THROUGHPUT_MAX		11
#define	DT_PM_QOS_INTCAM_THROUGHPUT_MAX		12
#define	DT_PM_QOS_BUS_THROUGHPUT		13
#define	DT_PM_QOS_BUS_THROUGHPUT_MAX		14
#define	DT_PM_QOS_NETWORK_THROUGHPUT		15
#define	DT_PM_QOS_MEMORY_BANDWIDTH		16
#define	DT_PM_QOS_DISPLAY_THROUGHPUT		17
#define	DT_PM_QOS_DISPLAY_THROUGHPUT_MAX	18
#define	DT_PM_QOS_CAM_THROUGHPUT		19
#define	DT_PM_QOS_AUD_THROUGHPUT		20
#define	DT_PM_QOS_CAM_THROUGHPUT_MAX		21
#define	DT_PM_QOS_AUD_THROUGHPUT_MAX		22
#define	DT_PM_QOS_MFC_THROUGHPUT		23
#define	DT_PM_QOS_MFC_THROUGHPUT_MAX		24
#define	DT_PM_QOS_NPU_THROUGHPUT		25
#define	DT_PM_QOS_NPU_THROUGHPUT_MAX		26
#define	DT_PM_QOS_DSP_THROUGHPUT		27
#define	DT_PM_QOS_DSP_THROUGHPUT_MAX		28
#define	DT_PM_QOS_TNR_THROUGHPUT		29
#define	DT_PM_QOS_TNR_THROUGHPUT_MAX		30
#define	DT_PM_QOS_DNC_THROUGHPUT		31
#define	DT_PM_QOS_DNC_THROUGHPUT_MAX		32

#endif
