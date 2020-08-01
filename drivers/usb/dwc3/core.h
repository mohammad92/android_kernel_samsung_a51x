// SPDX-License-Identifier: GPL-2.0
/*
 * core.h - DesignWare USB3 DRD Core Header
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 */

#ifndef __DRIVERS_USB_DWC3_CORE_H
#define __DRIVERS_USB_DWC3_CORE_H

#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/debugfs.h>
#include <linux/completion.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <linux/ulpi/interface.h>

#include <linux/phy/phy.h>

#if defined(CONFIG_BATTERY_SAMSUNG_LEGO_STYLE)
#include "../../battery/common/sec_charging_common.h"
#else
#include "../../battery_v2/include/sec_charging_common.h"
#endif

#define DWC3_MSG_MAX	500

/* Global constants */
#define DWC3_PULL_UP_TIMEOUT	500	/* ms */
#define DWC3_BOUNCE_SIZE	1024	/* size of a superspeed bulk */
#define DWC3_EP0_SETUP_SIZE	512
#define DWC3_ENDPOINTS_NUM	32
#define DWC3_XHCI_RESOURCES_NUM	2

#define DWC3_SCRATCHBUF_SIZE	4096	/* each buffer is assumed to be 4KiB */
#define DWC3_EVENT_BUFFERS_SIZE	4096
#define DWC3_EVENT_TYPE_MASK	0xfe

#define DWC3_EVENT_TYPE_DEV	0
#define DWC3_EVENT_TYPE_CARKIT	3
#define DWC3_EVENT_TYPE_I2C	4

#define DWC3_DEVICE_EVENT_DISCONNECT		0
#define DWC3_DEVICE_EVENT_RESET			1
#define DWC3_DEVICE_EVENT_CONNECT_DONE		2
#define DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE	3
#define DWC3_DEVICE_EVENT_WAKEUP		4
#define DWC3_DEVICE_EVENT_HIBER_REQ		5
#define DWC3_DEVICE_EVENT_EOPF			6
#define DWC3_DEVICE_EVENT_SOF			7
#define DWC3_DEVICE_EVENT_ERRATIC_ERROR		9
#define DWC3_DEVICE_EVENT_CMD_CMPL		10
#define DWC3_DEVICE_EVENT_OVERFLOW		11

/* Controller's role while using the OTG block */
#define DWC3_OTG_ROLE_IDLE	0
#define DWC3_OTG_ROLE_HOST	1
#define DWC3_OTG_ROLE_DEVICE	2

#define DWC3_GEVNTCOUNT_MASK	0xfffc
#define DWC3_GEVNTCOUNT_EHB	BIT(31)
#define DWC3_GSNPSID_MASK	0xffff0000
#define DWC3_GSNPSREV_MASK	0xffff

/* DWC3 registers memory space boundries */
#define DWC3_XHCI_REGS_START		0x0
#define DWC3_XHCI_REGS_END		0x7fff
#define DWC3_GLOBALS_REGS_START		0xc100
#define DWC3_GLOBALS_REGS_END		0xc6ff
#define DWC3_DEVICE_REGS_START		0xc700
#define DWC3_DEVICE_REGS_END		0xcbff
#define DWC3_OTG_REGS_START		0xcc00
#define DWC3_OTG_REGS_END		0xccff

/* Global Registers */
#define DWC3_GSBUSCFG0		0xc100
#define DWC3_GSBUSCFG1		0xc104
#define DWC3_GTXTHRCFG		0xc108
#define DWC3_GRXTHRCFG		0xc10c
#define DWC3_GCTL		0xc110
#define DWC3_GEVTEN		0xc114
#define DWC3_GSTS		0xc118
#define DWC3_GUCTL1		0xc11c
#define DWC3_GSNPSID		0xc120
#define DWC3_GGPIO		0xc124
#define DWC3_GUID		0xc128
#define DWC3_GUCTL		0xc12c
#define DWC3_GBUSERRADDR0	0xc130
#define DWC3_GBUSERRADDR1	0xc134
#define DWC3_GPRTBIMAP0		0xc138
#define DWC3_GPRTBIMAP1		0xc13c
#define DWC3_GHWPARAMS0		0xc140
#define DWC3_GHWPARAMS1		0xc144
#define DWC3_GHWPARAMS2		0xc148
#define DWC3_GHWPARAMS3		0xc14c
#define DWC3_GHWPARAMS4		0xc150
#define DWC3_GHWPARAMS5		0xc154
#define DWC3_GHWPARAMS6		0xc158
#define DWC3_GHWPARAMS7		0xc15c
#define DWC3_GDBGFIFOSPACE	0xc160
#define DWC3_GDBGLTSSM		0xc164
#define DWC3_GDBGBMU		0xc16c
#define DWC3_GDBGLSPMUX		0xc170
#define DWC3_GDBGLSP		0xc174
#define DWC3_GDBGEPINFO0	0xc178
#define DWC3_GDBGEPINFO1	0xc17c
#define DWC3_GPRTBIMAP_HS0	0xc180
#define DWC3_GPRTBIMAP_HS1	0xc184
#define DWC3_GPRTBIMAP_FS0	0xc188
#define DWC3_GPRTBIMAP_FS1	0xc18c
#define DWC3_GUCTL2		0xc19c

#define DWC3_VER_NUMBER		0xc1a0
#define DWC3_VER_TYPE		0xc1a4

#define DWC3_GUSB2PHYCFG(n)	(0xc200 + ((n) * 0x04))
#define DWC3_GUSB2I2CCTL(n)	(0xc240 + ((n) * 0x04))

#define DWC3_GUSB2PHYACC(n)	(0xc280 + ((n) * 0x04))

#define DWC3_GUSB3PIPECTL(n)	(0xc2c0 + ((n) * 0x04))

#define DWC3_GTXFIFOSIZ(n)	(0xc300 + ((n) * 0x04))
#define DWC3_GRXFIFOSIZ(n)	(0xc380 + ((n) * 0x04))

#define DWC3_GEVNTADRLO(n)	(0xc400 + ((n) * 0x10))
#define DWC3_GEVNTADRHI(n)	(0xc404 + ((n) * 0x10))
#define DWC3_GEVNTSIZ(n)	(0xc408 + ((n) * 0x10))
#define DWC3_GEVNTCOUNT(n)	(0xc40c + ((n) * 0x10))

#define DWC3_GHWPARAMS8		0xc600
#define DWC3_GFLADJ		0xc630

/* Device Registers */
#define DWC3_DCFG		0xc700
#define DWC3_DCTL		0xc704
#define DWC3_DEVTEN		0xc708
#define DWC3_DSTS		0xc70c
#define DWC3_DGCMDPAR		0xc710
#define DWC3_DGCMD		0xc714
#define DWC3_DALEPENA		0xc720

#define DWC3_DEP_BASE(n)	(0xc800 + ((n) * 0x10))
#define DWC3_DEPCMDPAR2		0x00
#define DWC3_DEPCMDPAR1		0x04
#define DWC3_DEPCMDPAR0		0x08
#define DWC3_DEPCMD		0x0c

#define DWC3_DEV_IMOD(n)	(0xca00 + ((n) * 0x4))

/* OTG Registers */
#define DWC3_OCFG		0xcc00
#define DWC3_OCTL		0xcc04
#define DWC3_OEVT		0xcc08
#define DWC3_OEVTEN		0xcc0C
#define DWC3_OSTS		0xcc10

#define DWC3_GSBUSCFG0_INCR256BRSTENA	(1 << 7)
#define DWC3_GSBUSCFG0_INCR128BRSTENA	(1 << 6)
#define DWC3_GSBUSCFG0_INCR64BRSTENA	(1 << 5)
#define DWC3_GSBUSCFG0_INCR32BRSTENA	(1 << 4)
#define DWC3_GSBUSCFG0_INCR16BRSTENA	(1 << 3)
#define DWC3_GSBUSCFG0_INCR8BRSTENA	(1 << 2)
#define DWC3_GSBUSCFG0_INCR4BRSTENA	(1 << 1)
#define DWC3_GSBUSCFG0_INCRBRSTENA	(1 << 0)

/* LINK Registers */
#define DWC3_LU3LFPSRXTIM	0xd010
#define DWC3_LSKIPFREQ		0xd020
#define DWC3_LLUCTL		0xd024
#define DWC3_BU31RHBDBG		0xd800

#define DWC3_GSBUSCFG0_INCRBRST_MASK	0xff

/* Bit fields */
/* Link Register - LLUCTL */
#define DWC3_PENDING_HP_TIMER_US(n)	((n) << 16)
#define DWC3_LLUCTL_LTSSM_TIMER_OVRRD	BIT(23)
#define DWC3_EN_US_HP_TIMER		BIT(15)
#define DWC3_LLUCTL_PIPE_RESET		BIT(7)
#define DWC3_FORCE_GEN1			BIT(10)
#define DWC3_LLUCTL_TX_TS1_CNT(n)	((n) << 0)
#define DWC3_LLUCTL_TX_TS1_CNT_MASK	DWC3_LLUCTL_TX_TS1_CNT(0x1f)

/* Link Register - LSKIPFREQ */
#define DWC3_PM_ENTRY_TIMER_US(n)	((n) << 20)
#define DWC3_PM_ENTRY_TIMER_US_MASK	DWC3_PM_ENTRY_TIMER_US(0xf)
#define DWC3_PM_LC_TIMER_US(n)		((n) << 24)
#define DWC3_PM_LC_TIMER_US_MASK	DWC3_PM_LC_TIMER_US(0x7)
#define DWC3_EN_PM_TIMER_US		BIT(27)

/* Debug Regster */
#define  DWC3_BU31RHBDBG_TOUTCTL	(0x1 << 3)

#define DWC3_GSBUSCFG0_INCRBRSTEN	(1 << 0)
#define DWC3_GSBUSCFG0_INCR4BRSTEN	(1 << 1)
#define DWC3_GSBUSCFG0_INCR8BRSTEN	(1 << 2)
#define DWC3_GSBUSCFG0_INCR16BRSTEN	(1 << 3)
#define DWC3_GSBUSCFG0_INCR32BRSTEN	(1 << 4)
#define DWC3_GSBUSCFG0_INCR64BRSTEN	(1 << 5)
#define DWC3_GSBUSCFG0_INCR128BRSTEN	(1 << 6)
#define DWC3_GSBUSCFG0_INCR256BRSTEN	(1 << 7)
#define DWC3_GSBUSCFG0_DESWRREQINFO	(2 << 16)
#define DWC3_GSBUSCFG0_DATWRREQINFO	(2 << 20)
#define DWC3_GSBUSCFG0_DESRDREQINFO	(2 << 24)
#define DWC3_GSBUSCFG0_DATRDREQINFO	(2 << 28)
#define DWC3_GDBGFIFOSPACE_NUM(n)	((n) & 0x1f)
#define DWC3_GDBGFIFOSPACE_TYPE(n)	(((n) << 5) & 0x1e0)
#define DWC3_GDBGFIFOSPACE_SPACE_AVAILABLE(n) (((n) >> 16) & 0xffff)

#define DWC3_GSBUSCFG1_BREQLIMIT(n)	((n) << 8)
#define DWC3_GSBUSCFG1_BREQLIMIT_SHIFT	8
#define DWC3_GSBUSCFG1_BREQLIMIT_MASK	(0xf << 8)
#define DWC3_GSBUSCFG1_EN1KPAGE		(1 << 12)

#define DWC3_GSBUSCFG1_EXTPLIMIT(n)	((n) << 13)
#define DWC3_GSBUSCFG1_EXTPLIMIT_SHIFT	13
#define DWC3_GSBUSCFG1_EXTPLIMIT_MASK	(0xf << 13)

#define DWC3_GRXTHRCFG_USBRXPKTCNTSEL		(1 << 29)
#define DWC3_GRXTHRCFG_USBRXPKTCNT_MASK		(0xf << 24)
#define DWC3_GRXTHRCFG_USBRXPKTCNT_SHIFT	24
#define DWC3_GRXTHRCFG_USBRXPKTCNT(n)		((n) << 24)
#define DWC3_GRXTHRCFG_USBMAXRXBURSTSIZE_MASK	(0x1f << 19)
#define DWC3_GRXTHRCFG_USBMAXRXBURSTSIZE_SHIFT	19
#define DWC3_GRXTHRCFG_USBMAXRXBURSTSIZE(n)	((n) << 19)

#define DWC3_TXFIFOQ		0
#define DWC3_RXFIFOQ		1
#define DWC3_TXREQQ		2
#define DWC3_RXREQQ		3
#define DWC3_RXINFOQ		4
#define DWC3_PSTATQ		5
#define DWC3_DESCFETCHQ		6
#define DWC3_EVENTQ		7
#define DWC3_AUXEVENTQ		8

/* Global RX Threshold Configuration Register */
#define DWC3_GRXTHRCFG_MAXRXBURSTSIZE(n) (((n) & 0x1f) << 19)
#define DWC3_GRXTHRCFG_RXPKTCNT(n) (((n) & 0xf) << 24)
#define DWC3_GRXTHRCFG_PKTCNTSEL BIT(29)

/* Global RX Threshold Configuration Register for DWC_usb31 only */
#define DWC31_GRXTHRCFG_MAXRXBURSTSIZE(n)	(((n) & 0x1f) << 16)
#define DWC31_GRXTHRCFG_RXPKTCNT(n)		(((n) & 0x1f) << 21)
#define DWC31_GRXTHRCFG_PKTCNTSEL		BIT(26)
#define DWC31_RXTHRNUMPKTSEL_HS_PRD		BIT(15)
#define DWC31_RXTHRNUMPKT_HS_PRD(n)		(((n) & 0x3) << 13)
#define DWC31_RXTHRNUMPKTSEL_PRD		BIT(10)
#define DWC31_RXTHRNUMPKT_PRD(n)		(((n) & 0x1f) << 5)
#define DWC31_MAXRXBURSTSIZE_PRD(n)		((n) & 0x1f)

/* Global TX Threshold Configuration Register for DWC_usb31 only */
#define DWC31_GTXTHRCFG_MAXTXBURSTSIZE(n)	(((n) & 0x1f) << 16)
#define DWC31_GTXTHRCFG_TXPKTCNT(n)		(((n) & 0x1f) << 21)
#define DWC31_GTXTHRCFG_PKTCNTSEL		BIT(26)
#define DWC31_TXTHRNUMPKTSEL_HS_PRD		BIT(15)
#define DWC31_TXTHRNUMPKT_HS_PRD(n)		(((n) & 0x3) << 13)
#define DWC31_TXTHRNUMPKTSEL_PRD		BIT(10)
#define DWC31_TXTHRNUMPKT_PRD(n)		(((n) & 0x1f) << 5)
#define DWC31_MAXTXBURSTSIZE_PRD(n)		((n) & 0x1f)

/* Global Configuration Register */
#define DWC3_GCTL_PWRDNSCALE(n)	((n) << 19)
#define DWC3_GCTL_PWRDNSCALE_MASK      DWC3_GCTL_PWRDNSCALE(0x1fff)
#define DWC3_GCTL_U2RSTECN	BIT(16)
#define DWC3_GCTL_RAMCLKSEL(x)	(((x) & DWC3_GCTL_CLK_MASK) << 6)
#define DWC3_GCTL_CLK_BUS	(0)
#define DWC3_GCTL_CLK_PIPE	(1)
#define DWC3_GCTL_CLK_PIPEHALF	(2)
#define DWC3_GCTL_CLK_MASK	(3)

#define DWC3_GCTL_PRTCAP(n)	(((n) & (3 << 12)) >> 12)
#define DWC3_GCTL_PRTCAPDIR(n)	((n) << 12)
#define DWC3_GCTL_PRTCAP_HOST	1
#define DWC3_GCTL_PRTCAP_DEVICE	2
#define DWC3_GCTL_PRTCAP_OTG	3

#define DWC3_GCTL_CORESOFTRESET		BIT(11)
#define DWC3_GCTL_SOFITPSYNC		BIT(10)
#define DWC3_GCTL_SCALEDOWN(n)		((n) << 4)
#define DWC3_GCTL_SCALEDOWN_MASK	DWC3_GCTL_SCALEDOWN(3)
#define DWC3_GCTL_DISSCRAMBLE		BIT(3)
#define DWC3_GCTL_U2EXIT_LFPS		BIT(2)
#define DWC3_GCTL_GBLHIBERNATIONEN	BIT(1)
#define DWC3_GCTL_DSBLCLKGTNG		BIT(0)
#define DWC3_GUCTL_USBHSTINAUTORETRYEN	(1 << 14)

#define DWC3_GUCTL_HSTINAUTORETRY	BIT(14)

#define DWC3_GUCTL_REFCLKPER(n)		((n) << 22)
#define DWC3_GUCTL_REFCLKPER_MASK	(DWC3_GUCTL_REFCLKPER(0x3FF))
#define DWC3_GUCTL_NOEXTRDL		(1 << 21)
#define DWC3_GUCTL_USBHSTINAUTORETRYEN	(1 << 14)
#define DWC3_GUCTL_SPRSCTRLTRANSEN	(1 << 17)
#define DWC3_GUCTL_DTOUT(n)		(n)
#define DWC3_GUCTL_DTOUT_MASK		(0x7ff)

/* Global User Control 1 Register */
#define DWC3_GUCTL1_TX_IPGAP_LINECHECK_DIS	BIT(28)
#define DWC3_GUCTL1_DEV_L1_EXIT_BY_HW	BIT(24)
#define DWC3_GUCTL1_IP_GAP_ADD_ON(n)		((n) << 21)
#define DWC3_GUCTL1_IP_GAP_ADD_ON_MASK	(DWC3_GUCTL1_IP_GAP_ADD_ON(0x7))

/* Global USB2 PHY Configuration Register */
#define DWC3_GUSB2PHYCFG_PHYSOFTRST	BIT(31)
#define DWC3_GUSB2PHYCFG_U2_FREECLK_EXISTS	BIT(30)
#define DWC3_GUSB2PHYCFG_SUSPHY		BIT(6)
#define DWC3_GUSB2PHYCFG_ULPI_UTMI	BIT(4)
#define DWC3_GUSB2PHYCFG_ENBLSLPM	BIT(8)
#define DWC3_GUSB2PHYCFG_PHYIF(n)	(n << 3)
#define DWC3_GUSB2PHYCFG_PHYIF_MASK	DWC3_GUSB2PHYCFG_PHYIF(1)
#define DWC3_GUSB2PHYCFG_USBTRDTIM(n)	(n << 10)
#define DWC3_GUSB2PHYCFG_USBTRDTIM_MASK	DWC3_GUSB2PHYCFG_USBTRDTIM(0xf)
#define USBTRDTIM_UTMI_8_BIT		9
#define USBTRDTIM_UTMI_16_BIT		5
#define UTMI_PHYIF_16_BIT		1
#define UTMI_PHYIF_8_BIT		0

/* Global USB2 PHY Vendor Control Register */
#define DWC3_GUSB2PHYACC_NEWREGREQ	BIT(25)
#define DWC3_GUSB2PHYACC_BUSY		BIT(23)
#define DWC3_GUSB2PHYACC_WRITE		BIT(22)
#define DWC3_GUSB2PHYACC_ADDR(n)	(n << 16)
#define DWC3_GUSB2PHYACC_EXTEND_ADDR(n)	(n << 8)
#define DWC3_GUSB2PHYACC_DATA(n)	(n & 0xff)

/* Global USB3 PIPE Control Register */
#define DWC3_GUSB3PIPECTL_PHYSOFTRST	BIT(31)
#define DWC3_GUSB3PIPECTL_U2SSINP3OK	BIT(29)
#define DWC3_GUSB3PIPECTL_DISRXDETINP3	BIT(28)
#define DWC3_GUSB3PIPECTL_UX_EXIT_PX	BIT(27)
#define DWC3_GUSB3PIPECTL_U1U2EXITFAIL_TO_RECOV	BIT(25)
#define DWC3_GUSB3PIPECTL_REQP1P2P3	BIT(24)
#define DWC3_GUSB3PIPECTL_DEP1P2P3(n)	((n) << 19)
#define DWC3_GUSB3PIPECTL_DEP1P2P3_MASK	DWC3_GUSB3PIPECTL_DEP1P2P3(7)
#define DWC3_GUSB3PIPECTL_DEP1P2P3_EN	DWC3_GUSB3PIPECTL_DEP1P2P3(1)
#define DWC3_GUSB3PIPECTL_DEPOCHANGE	BIT(18)
#define DWC3_GUSB3PIPECTL_SUSPHY	BIT(17)
#define DWC3_GUSB3PIPECTL_LFPSFILT	BIT(9)
#define DWC3_GUSB3PIPECTL_RX_DETOPOLL	BIT(8)
#define DWC3_GUSB3PIPECTL_TX_DEEPH_MASK	DWC3_GUSB3PIPECTL_TX_DEEPH(3)
#define DWC3_GUSB3PIPECTL_TX_DEEPH(n)	((n) << 1)
#define DWC3_ELASTIC_BUFFER_MODE	BIT(0)

/* Global TX Fifo Size Register */
#define DWC31_GTXFIFOSIZ_TXFRAMNUM	BIT(15)		/* DWC_usb31 only */
#define DWC31_GTXFIFOSIZ_TXFDEF(n)	((n) & 0x7fff)	/* DWC_usb31 only */
#define DWC3_GTXFIFOSIZ_TXFDEF(n)	((n) & 0xffff)
#define DWC3_GTXFIFOSIZ_TXFSTADDR(n)	((n) & 0xffff0000)

/* Global Event Size Registers */
#define DWC3_GEVNTSIZ_INTMASK		BIT(31)
#define DWC3_GEVNTSIZ_SIZE(n)		((n) & 0xffff)

/* Global HWPARAMS0 Register */
#define DWC3_GHWPARAMS0_MODE(n)		((n) & 0x3)
#define DWC3_GHWPARAMS0_MODE_GADGET	0
#define DWC3_GHWPARAMS0_MODE_HOST	1
#define DWC3_GHWPARAMS0_MODE_DRD	2
#define DWC3_GHWPARAMS0_MBUS_TYPE(n)	(((n) >> 3) & 0x7)
#define DWC3_GHWPARAMS0_SBUS_TYPE(n)	(((n) >> 6) & 0x3)
#define DWC3_GHWPARAMS0_MDWIDTH(n)	(((n) >> 8) & 0xff)
#define DWC3_GHWPARAMS0_SDWIDTH(n)	(((n) >> 16) & 0xff)
#define DWC3_GHWPARAMS0_AWIDTH(n)	(((n) >> 24) & 0xff)

/* Global HWPARAMS1 Register */
#define DWC3_GHWPARAMS1_EN_PWROPT(n)	(((n) & (3 << 24)) >> 24)
#define DWC3_GHWPARAMS1_EN_PWROPT_NO	0
#define DWC3_GHWPARAMS1_EN_PWROPT_CLK	1
#define DWC3_GHWPARAMS1_EN_PWROPT_HIB	2
#define DWC3_GHWPARAMS1_PWROPT(n)	((n) << 24)
#define DWC3_GHWPARAMS1_PWROPT_MASK	DWC3_GHWPARAMS1_PWROPT(3)

/* Global HWPARAMS3 Register */
#define DWC3_GHWPARAMS3_SSPHY_IFC(n)		((n) & 3)
#define DWC3_GHWPARAMS3_SSPHY_IFC_DIS		0
#define DWC3_GHWPARAMS3_SSPHY_IFC_GEN1		1
#define DWC3_GHWPARAMS3_SSPHY_IFC_GEN2		2 /* DWC_usb31 only */
#define DWC3_GHWPARAMS3_HSPHY_IFC(n)		(((n) & (3 << 2)) >> 2)
#define DWC3_GHWPARAMS3_HSPHY_IFC_DIS		0
#define DWC3_GHWPARAMS3_HSPHY_IFC_UTMI		1
#define DWC3_GHWPARAMS3_HSPHY_IFC_ULPI		2
#define DWC3_GHWPARAMS3_HSPHY_IFC_UTMI_ULPI	3
#define DWC3_GHWPARAMS3_FSPHY_IFC(n)		(((n) & (3 << 4)) >> 4)
#define DWC3_GHWPARAMS3_FSPHY_IFC_DIS		0
#define DWC3_GHWPARAMS3_FSPHY_IFC_ENA		1

/* Global HWPARAMS4 Register */
#define DWC3_GHWPARAMS4_HIBER_SCRATCHBUFS(n)	(((n) & (0x0f << 13)) >> 13)
#define DWC3_MAX_HIBER_SCRATCHBUFS		15

/* Global HWPARAMS6 Register */
#define DWC3_GHWPARAMS6_BCSUPPORT		BIT(14)
#define DWC3_GHWPARAMS6_OTG3SUPPORT		BIT(13)
#define DWC3_GHWPARAMS6_ADPSUPPORT		BIT(12)
#define DWC3_GHWPARAMS6_HNPSUPPORT		BIT(11)
#define DWC3_GHWPARAMS6_SRPSUPPORT		BIT(10)
#define DWC3_GHWPARAMS6_EN_FPGA			BIT(7)

/* Global HWPARAMS7 Register */
#define DWC3_GHWPARAMS7_RAM1_DEPTH(n)	((n) & 0xffff)
#define DWC3_GHWPARAMS7_RAM2_DEPTH(n)	(((n) >> 16) & 0xffff)

/* Global Frame Length Adjustment Register */
#define DWC3_GFLADJ_REFCLK_240MHZDECR_PLS1	BIT(31)
#define DWC3_GFLADJ_REFCLK_240MHZ_DECR(n)	((n) << 24)
#define DWC3_GFLADJ_REFCLK_240MHZ_DECR_MASK	DWC3_GFLADJ_REFCLK_240MHZ_DECR(0x7F)
#define DWC3_GFLADJ_REFCLK_LPM_SEL		BIT(23)
#define DWC3_GFLADJ_REFCLK_FLADJ(n)		((n) << 8)
#define DWC3_GFLADJ_REFCLK_FLADJ_MASK		DWC3_GFLADJ_REFCLK_FLADJ(0x3FFF)
#define DWC3_GFLADJ_30MHZ_SDBND_SEL		BIT(7)
#define DWC3_GFLADJ_30MHZ_MASK			0x3f

/* Global User Control Register 2 */
#define DWC3_GUCTL2_RST_ACTBITLATER		BIT(14)

/* Device Configuration Register */
#define DWC3_DCFG_DEVADDR(addr)	((addr) << 3)
#define DWC3_DCFG_DEVADDR_MASK	DWC3_DCFG_DEVADDR(0x7f)

#define DWC3_DCFG_SPEED_MASK	(7 << 0)
#define DWC3_DCFG_SUPERSPEED_PLUS (5 << 0)  /* DWC_usb31 only */
#define DWC3_DCFG_SUPERSPEED	(4 << 0)
#define DWC3_DCFG_HIGHSPEED	(0 << 0)
#define DWC3_DCFG_FULLSPEED	BIT(0)
#define DWC3_DCFG_LOWSPEED	(2 << 0)
#define DWC3_DCFG_FULLSPEED1	(3 << 0) // Check later!

#define DWC3_DCFG_NUMP_SHIFT	17
#define DWC3_DCFG_NUMP(n)	(((n) >> DWC3_DCFG_NUMP_SHIFT) & 0x1f)
#define DWC3_DCFG_NUMP_MASK	(0x1f << DWC3_DCFG_NUMP_SHIFT)
#define DWC3_DCFG_LPM_CAP	BIT(22)

/* Device Control Register */
#define DWC3_DCTL_RUN_STOP	BIT(31)
#define DWC3_DCTL_CSFTRST	BIT(30)
#define DWC3_DCTL_LSFTRST	BIT(29)

#define DWC3_DCTL_HIRD_THRES_MASK	(0x1f << 24)
#define DWC3_DCTL_HIRD_THRES(n)	((n) << 24)

#define DWC3_DCTL_APPL1RES	BIT(23)

/* These apply for core versions 1.87a and earlier */
#define DWC3_DCTL_TRGTULST_MASK		(0x0f << 17)
#define DWC3_DCTL_TRGTULST(n)		((n) << 17)
#define DWC3_DCTL_TRGTULST_U2		(DWC3_DCTL_TRGTULST(2))
#define DWC3_DCTL_TRGTULST_U3		(DWC3_DCTL_TRGTULST(3))
#define DWC3_DCTL_TRGTULST_SS_DIS	(DWC3_DCTL_TRGTULST(4))
#define DWC3_DCTL_TRGTULST_RX_DET	(DWC3_DCTL_TRGTULST(5))
#define DWC3_DCTL_TRGTULST_SS_INACT	(DWC3_DCTL_TRGTULST(6))

/* These apply for core versions 1.94a and later */
#define DWC3_DCTL_LPM_ERRATA_MASK	DWC3_DCTL_LPM_ERRATA(0xf)
#define DWC3_DCTL_LPM_ERRATA(n)		((n) << 20)

#define DWC3_DCTL_KEEP_CONNECT		BIT(19)
#define DWC3_DCTL_L1_HIBER_EN		BIT(18)
#define DWC3_DCTL_CRS			BIT(17)
#define DWC3_DCTL_CSS			BIT(16)

#define DWC3_DCTL_INITU2ENA		BIT(12)
#define DWC3_DCTL_ACCEPTU2ENA		BIT(11)
#define DWC3_DCTL_INITU1ENA		BIT(10)
#define DWC3_DCTL_ACCEPTU1ENA		BIT(9)
#define DWC3_DCTL_TSTCTRL_MASK		(0xf << 1)

#define DWC3_DCTL_REMOTE_WAKE_UP	(0x8 << 5)
#define DWC3_DCTL_ULSTCHNGREQ_MASK	(0x0f << 5)
#define DWC3_DCTL_ULSTCHNGREQ(n) (((n) << 5) & DWC3_DCTL_ULSTCHNGREQ_MASK)

#define DWC3_DCTL_ULSTCHNG_NO_ACTION	(DWC3_DCTL_ULSTCHNGREQ(0))
#define DWC3_DCTL_ULSTCHNG_SS_DISABLED	(DWC3_DCTL_ULSTCHNGREQ(4))
#define DWC3_DCTL_ULSTCHNG_RX_DETECT	(DWC3_DCTL_ULSTCHNGREQ(5))
#define DWC3_DCTL_ULSTCHNG_SS_INACTIVE	(DWC3_DCTL_ULSTCHNGREQ(6))
#define DWC3_DCTL_ULSTCHNG_RECOVERY	(DWC3_DCTL_ULSTCHNGREQ(8))
#define DWC3_DCTL_ULSTCHNG_COMPLIANCE	(DWC3_DCTL_ULSTCHNGREQ(10))
#define DWC3_DCTL_ULSTCHNG_LOOPBACK	(DWC3_DCTL_ULSTCHNGREQ(11))

/* Device Event Enable Register */
#define DWC3_DEVTEN_VNDRDEVTSTRCVEDEN	BIT(12)
#define DWC3_DEVTEN_EVNTOVERFLOWEN	BIT(11)
#define DWC3_DEVTEN_CMDCMPLTEN		BIT(10)
#define DWC3_DEVTEN_ERRTICERREN		BIT(9)
#define DWC3_DEVTEN_SOFEN		BIT(7)
#define DWC3_DEVTEN_EOPFEN		BIT(6)
#define DWC3_DEVTEN_U3L2_SUSPEN	BIT(6)
#define DWC3_DEVTEN_HIBERNATIONREQEVTEN	BIT(5)
#define DWC3_DEVTEN_WKUPEVTEN		BIT(4)
#define DWC3_DEVTEN_ULSTCNGEN		BIT(3)
#define DWC3_DEVTEN_CONNECTDONEEN	BIT(2)
#define DWC3_DEVTEN_USBRSTEN		BIT(1)
#define DWC3_DEVTEN_DISCONNEVTEN	BIT(0)

/* Device Status Register */
#define DWC3_DSTS_DCNRD			BIT(29)

/* This applies for core versions 1.87a and earlier */
#define DWC3_DSTS_PWRUPREQ		BIT(24)

/* These apply for core versions 1.94a and later */
#define DWC3_DSTS_RSS			BIT(25)
#define DWC3_DSTS_SSS			BIT(24)

#define DWC3_DSTS_COREIDLE		BIT(23)
#define DWC3_DSTS_DEVCTRLHLT		BIT(22)

#define DWC3_DSTS_USBLNKST_MASK		(0x0f << 18)
#define DWC3_DSTS_USBLNKST(n)		(((n) & DWC3_DSTS_USBLNKST_MASK) >> 18)

#define DWC3_DSTS_RXFIFOEMPTY		BIT(17)

#define DWC3_DSTS_SOFFN_MASK		(0x3fff << 3)
#define DWC3_DSTS_SOFFN(n)		(((n) & DWC3_DSTS_SOFFN_MASK) >> 3)

#define DWC3_DSTS_CONNECTSPD		(7 << 0)

#define DWC3_DSTS_SUPERSPEED_PLUS	(5 << 0) /* DWC_usb31 only */
#define DWC3_DSTS_SUPERSPEED		(4 << 0)
#define DWC3_DSTS_HIGHSPEED		(0 << 0)
#define DWC3_DSTS_FULLSPEED		BIT(0)
#define DWC3_DSTS_LOWSPEED		(2 << 0)
#define DWC3_DSTS_FULLSPEED1		(3 << 0)


/* Notification Type. identified the type of the device notification
   (8.5.6 Device Notification (DEV_NOTIFICATION) Transaction Packet) */
#define RESERVED				0x0
#define FUNCTION_WAKE				0x1
#define LATENCY_TOLERANCE_MESSAGE		0x2
#define BUS_INTERVAL_ADJUSTMENT_MESSAGE		0x3
#define HOST_ROLE_REQUEST			0x4
#define SUBLINK_SPEED				0x5

/* Device Generic Command Register */
#define DWC3_DGCMD_SET_LMP		0x01
#define DWC3_DGCMD_SET_PERIODIC_PAR	0x02
#define DWC3_DGCMD_XMIT_FUNCTION	0x03

/* These apply for core versions 1.94a and later */
#define DWC3_DGCMD_SET_SCRATCHPAD_ADDR_LO	0x04
#define DWC3_DGCMD_SET_SCRATCHPAD_ADDR_HI	0x05

/* Function Wake Noti about U3 */
#define DWC3_DGCMD_TRANSMIT_DEV_NOTI	0x07

#define DWC3_DGCMD_SELECTED_FIFO_FLUSH	0x09
#define DWC3_DGCMD_ALL_FIFO_FLUSH	0x0a
#define DWC3_DGCMD_SET_ENDPOINT_NRDY	0x0c
#define DWC3_DGCMD_RUN_SOC_BUS_LOOPBACK	0x10

#define DWC3_DGCMD_STATUS(n)		(((n) >> 12) & 0x0F)
#define DWC3_DGCMD_CMDACT		BIT(10)
#define DWC3_DGCMD_CMDIOC		BIT(8)

/* Device Generic Command Parameter Register */
#define DWC3_DGCMDPAR_FORCE_LINKPM_ACCEPT	BIT(0)
#define DWC3_DGCMDPAR_FIFO_NUM(n)		((n) << 0)
#define DWC3_DGCMDPAR_RX_FIFO			(0 << 5)
#define DWC3_DGCMDPAR_TX_FIFO			BIT(5)
#define DWC3_DGCMDPAR_LOOPBACK_DIS		(0 << 0)
#define DWC3_DGCMDPAR_LOOPBACK_ENA		BIT(0)

/* Device Endpoint Command Register */
#define DWC3_DEPCMD_PARAM_SHIFT		16
#define DWC3_DEPCMD_PARAM(x)		((x) << DWC3_DEPCMD_PARAM_SHIFT)
#define DWC3_DEPCMD_GET_RSC_IDX(x)	(((x) >> DWC3_DEPCMD_PARAM_SHIFT) & 0x7f)
#define DWC3_DEPCMD_STATUS(x)		(((x) >> 12) & 0x0F)
#define DWC3_DEPCMD_HIPRI_FORCERM	BIT(11)
#define DWC3_DEPCMD_CLEARPENDIN		BIT(11)
#define DWC3_DEPCMD_CMDACT		BIT(10)
#define DWC3_DEPCMD_CMDIOC		BIT(8)

#define DWC3_DEPCMD_DEPSTARTCFG		(0x09 << 0)
#define DWC3_DEPCMD_ENDTRANSFER		(0x08 << 0)
#define DWC3_DEPCMD_UPDATETRANSFER	(0x07 << 0)
#define DWC3_DEPCMD_STARTTRANSFER	(0x06 << 0)
#define DWC3_DEPCMD_CLEARSTALL		(0x05 << 0)
#define DWC3_DEPCMD_SETSTALL		(0x04 << 0)
/* This applies for core versions 1.90a and earlier */
#define DWC3_DEPCMD_GETSEQNUMBER	(0x03 << 0)
/* This applies for core versions 1.94a and later */
#define DWC3_DEPCMD_GETEPSTATE		(0x03 << 0)
#define DWC3_DEPCMD_SETTRANSFRESOURCE	(0x02 << 0)
#define DWC3_DEPCMD_SETEPCONFIG		(0x01 << 0)

#define DWC3_DEPCMD_CMD(x)		((x) & 0xf)

/* The EP number goes 0..31 so ep0 is always out and ep1 is always in */
#define DWC3_DALEPENA_EP(n)		BIT(n)

#define DWC3_DEPCMD_TYPE_CONTROL	0
#define DWC3_DEPCMD_TYPE_ISOC		1
#define DWC3_DEPCMD_TYPE_BULK		2
#define DWC3_DEPCMD_TYPE_INTR		3

/* OTG Control Register */
#define DWC3_OTG_OCTL_PERIMODE         (1 << 6)

/* OTG Events Register */
#define DWC3_OEVT_DEVICEMODE			(1 << 31)
#define DWC3_OEVT_CLEAR_ALL			(~DWC3_OEVT_DEVICEMODE)
#define DWC3_OEVTEN_OTGCONIDSTSCHNGEVNT		(1 << 24)
#define DWC3_OEVTEN_OTGBDEVVBUSCHNGEVNT		(1 << 8)

/* OTG Status Register */
#define DWC3_OTG_OSTS_BSESVALID		(1 << 2)
#define DWC3_OTG_OSTS_CONIDSTS		(1 << 0)

#define DWC3_DEV_IMOD_COUNT_SHIFT	16
#define DWC3_DEV_IMOD_COUNT_MASK	(0xffff << 16)
#define DWC3_DEV_IMOD_INTERVAL_SHIFT	0
#define DWC3_DEV_IMOD_INTERVAL_MASK	(0xffff << 0)

#define DWC3_LINK_STATE_INFO_LIMIT 15
#define DWC3_LINK_STATE_LAST_INFO_MEM 8

/* OTG Configuration Register */
#define DWC3_OCFG_DISPWRCUTTOFF		BIT(5)
#define DWC3_OCFG_HIBDISMASK		BIT(4)
#define DWC3_OCFG_SFTRSTMASK		BIT(3)
#define DWC3_OCFG_OTGVERSION		BIT(2)
#define DWC3_OCFG_HNPCAP		BIT(1)
#define DWC3_OCFG_SRPCAP		BIT(0)

/* OTG CTL Register */
#define DWC3_OCTL_OTG3GOERR		BIT(7)
#define DWC3_OCTL_PERIMODE		BIT(6)
#define DWC3_OCTL_PRTPWRCTL		BIT(5)
#define DWC3_OCTL_HNPREQ		BIT(4)
#define DWC3_OCTL_SESREQ		BIT(3)
#define DWC3_OCTL_TERMSELIDPULSE	BIT(2)
#define DWC3_OCTL_DEVSETHNPEN		BIT(1)
#define DWC3_OCTL_HSTSETHNPEN		BIT(0)

/* OTG Event Register */
#define DWC3_OEVT_DEVRUNSTPSET		BIT(26)
#define DWC3_OEVT_HIBENTRY		BIT(25)
#define DWC3_OEVT_CONIDSTSCHNG		BIT(24)
#define DWC3_OEVT_HRRCONFNOTIF		BIT(23)
#define DWC3_OEVT_HRRINITNOTIF		BIT(22)
#define DWC3_OEVT_ADEVIDLE		BIT(21)
#define DWC3_OEVT_ADEVBHOSTEND		BIT(20)
#define DWC3_OEVT_ADEVHOST		BIT(19)
#define DWC3_OEVT_ADEVHNPCHNG		BIT(18)
#define DWC3_OEVT_ADEVSRPDET		BIT(17)
#define DWC3_OEVT_ADEVSESSENDDET	BIT(16)
#define DWC3_OEVT_BDEVBHOSTEND		BIT(11)
#define DWC3_OEVT_BDEVHNPCHNG		BIT(10)
#define DWC3_OEVT_BDEVSESSVLDDET	BIT(9)
#define DWC3_OEVT_BDEVVBUSCHNG		BIT(8)
#define DWC3_OEVT_BSESSVLD		BIT(3)
#define DWC3_OEVT_HSTNEGSTS		BIT(2)
#define DWC3_OEVT_SESREQSTS		BIT(1)
#define DWC3_OEVT_ERROR			BIT(0)

/* OTG Event Enable Register */
#define DWC3_OEVTEN_XHCIRUNSTPSETEN	BIT(27)
#define DWC3_OEVTEN_DEVRUNSTPSETEN	BIT(26)
#define DWC3_OEVTEN_HIBENTRYEN		BIT(25)
#define DWC3_OEVTEN_CONIDSTSCHNGEN	BIT(24)
#define DWC3_OEVTEN_HRRCONFNOTIFEN	BIT(23)
#define DWC3_OEVTEN_HRRINITNOTIFEN	BIT(22)
#define DWC3_OEVTEN_ADEVIDLEEN		BIT(21)
#define DWC3_OEVTEN_ADEVBHOSTENDEN	BIT(20)
#define DWC3_OEVTEN_ADEVHOSTEN		BIT(19)
#define DWC3_OEVTEN_ADEVHNPCHNGEN	BIT(18)
#define DWC3_OEVTEN_ADEVSRPDETEN	BIT(17)
#define DWC3_OEVTEN_ADEVSESSENDDETEN	BIT(16)
#define DWC3_OEVTEN_BDEVBHOSTENDEN	BIT(11)
#define DWC3_OEVTEN_BDEVHNPCHNGEN	BIT(10)
#define DWC3_OEVTEN_BDEVSESSVLDDETEN	BIT(9)
#define DWC3_OEVTEN_BDEVVBUSCHNGEN	BIT(8)

/* OTG Status Register */
#define DWC3_OSTS_DEVRUNSTP		BIT(13)
#define DWC3_OSTS_XHCIRUNSTP		BIT(12)
#define DWC3_OSTS_PERIPHERALSTATE	BIT(4)
#define DWC3_OSTS_XHCIPRTPOWER		BIT(3)
#define DWC3_OSTS_BSESVLD		BIT(2)
#define DWC3_OSTS_VBUSVLD		BIT(1)
#define DWC3_OSTS_CONIDSTS		BIT(0)

/* Structures */

struct dwc3_trb;

/**
 * struct dwc3_event_buffer - Software event buffer representation
 * @buf: _THE_ buffer
 * @cache: The buffer cache used in the threaded interrupt
 * @length: size of this buffer
 * @lpos: event offset
 * @count: cache of last read event count register
 * @flags: flags related to this event buffer
 * @dma: dma_addr_t
 * @dwc: pointer to DWC controller
 */
struct dwc3_event_buffer {
	void			*buf;
	void			*cache;
	unsigned		length;
	unsigned int		lpos;
	unsigned int		count;
	unsigned int		flags;

#define DWC3_EVENT_PENDING	BIT(0)

	dma_addr_t		dma;

	struct dwc3		*dwc;
};

#define DWC3_EP_FLAG_STALLED	BIT(0)
#define DWC3_EP_FLAG_WEDGED	BIT(1)

#define DWC3_EP_DIRECTION_TX	true
#define DWC3_EP_DIRECTION_RX	false

#define DWC3_TRB_NUM		256
#define DWC3_TRB_MASK		(DWC3_TRB_NUM - 1)

/**
 * struct dwc3_ep - device side endpoint representation
 * @endpoint: usb endpoint
 * @cancelled_list: list of cancelled requests for this endpoint
 * @pending_list: list of pending requests for this endpoint
 * @started_list: list of started requests on this endpoint
 * @lock: spinlock for endpoint request queue traversal
 * @regs: pointer to first endpoint register
 * @trb_pool: array of transaction buffers
 * @trb_pool_dma: dma address of @trb_pool
 * @trb_enqueue: enqueue 'pointer' into TRB array
 * @trb_dequeue: dequeue 'pointer' into TRB array
 * @dwc: pointer to DWC controller
 * @saved_state: ep state saved during hibernation
 * @flags: endpoint flags (wedged, stalled, ...)
 * @number: endpoint number (1 - 15)
 * @type: set to bmAttributes & USB_ENDPOINT_XFERTYPE_MASK
 * @resource_index: Resource transfer index
 * @frame_number: set to the frame number we want this transfer to start (ISOC)
 * @interval: the interval on which the ISOC transfer is started
 * @name: a human readable name e.g. ep1out-bulk
 * @direction: true for TX, false for RX
 * @stream_capable: true when streams are enabled
 */
struct dwc3_ep {
	struct usb_ep		endpoint;
	struct list_head	cancelled_list;
	struct list_head	pending_list;
	struct list_head	started_list;

	spinlock_t		lock;
	void __iomem		*regs;

	struct dwc3_trb		*trb_pool;
	dma_addr_t		trb_pool_dma;
	u32			free_slot;
	u32			busy_slot;
	struct dwc3		*dwc;

	u32			saved_state;
	unsigned		flags;
#define DWC3_EP_ENABLED		BIT(0)
#define DWC3_EP_STALL		BIT(1)
#define DWC3_EP_WEDGE		BIT(2)
#define DWC3_EP_TRANSFER_STARTED BIT(3)
#define DWC3_EP_PENDING_REQUEST	BIT(5)
#define DWC3_EP_END_TRANSFER_PENDING	BIT(7)

	/* This last one is specific to EP0 */
#define DWC3_EP0_DIR_IN		BIT(31)

	/*
	 * IMPORTANT: we *know* we have 256 TRBs in our @trb_pool, so we will
	 * use a u8 type here. If anybody decides to increase number of TRBs to
	 * anything larger than 256 - I can't see why people would want to do
	 * this though - then this type needs to be changed.
	 *
	 * By using u8 types we ensure that our % operator when incrementing
	 * enqueue and dequeue get optimized away by the compiler.
	 */
	u8			trb_enqueue;
	u8			trb_dequeue;

	u8			number;
	u8			type;
	u8			resource_index;
	u32			frame_number;
	u32			interval;

	char			name[20];

	unsigned		direction:1;
	unsigned		stream_capable:1;
};

enum dwc3_phy {
	DWC3_PHY_UNKNOWN = 0,
	DWC3_PHY_USB3,
	DWC3_PHY_USB2,
};

enum dwc3_ep0_next {
	DWC3_EP0_UNKNOWN = 0,
	DWC3_EP0_COMPLETE,
	DWC3_EP0_NRDY_DATA,
	DWC3_EP0_NRDY_STATUS,
};

enum dwc3_ep0_state {
	EP0_UNCONNECTED		= 0,
	EP0_SETUP_PHASE,
	EP0_DATA_PHASE,
	EP0_STATUS_PHASE,
};

enum dwc3_link_state {
	/* In SuperSpeed */
	DWC3_LINK_STATE_U0		= 0x00, /* in HS, means ON */
	DWC3_LINK_STATE_U1		= 0x01,
	DWC3_LINK_STATE_U2		= 0x02, /* in HS, means SLEEP */
	DWC3_LINK_STATE_U3		= 0x03, /* in HS, means SUSPEND */
	DWC3_LINK_STATE_SS_DIS		= 0x04,
	DWC3_LINK_STATE_RX_DET		= 0x05, /* in HS, means Early Suspend */
	DWC3_LINK_STATE_SS_INACT	= 0x06,
	DWC3_LINK_STATE_POLL		= 0x07,
	DWC3_LINK_STATE_RECOV		= 0x08,
	DWC3_LINK_STATE_HRESET		= 0x09,
	DWC3_LINK_STATE_CMPLY		= 0x0a,
	DWC3_LINK_STATE_LPBK		= 0x0b,
	DWC3_LINK_STATE_RESET		= 0x0e,
	DWC3_LINK_STATE_RESUME		= 0x0f,
	DWC3_LINK_STATE_MASK		= 0x0f,
};

enum {
	RELEASE	= 0,
	NOTIFY	= 1,
};

/* TRB Length, PCM and Status */
#define DWC3_TRB_SIZE_MASK	(0x00ffffff)
#define DWC3_TRB_SIZE_LENGTH(n)	((n) & DWC3_TRB_SIZE_MASK)
#define DWC3_TRB_SIZE_PCM1(n)	(((n) & 0x03) << 24)
#define DWC3_TRB_SIZE_TRBSTS(n)	(((n) & (0x0f << 28)) >> 28)

#define DWC3_TRBSTS_OK			0
#define DWC3_TRBSTS_MISSED_ISOC		1
#define DWC3_TRBSTS_SETUP_PENDING	2
#define DWC3_TRB_STS_XFER_IN_PROG	4

/* TRB Control */
#define DWC3_TRB_CTRL_HWO		BIT(0)
#define DWC3_TRB_CTRL_LST		BIT(1)
#define DWC3_TRB_CTRL_CHN		BIT(2)
#define DWC3_TRB_CTRL_CSP		BIT(3)
#define DWC3_TRB_CTRL_TRBCTL(n)		(((n) & 0x3f) << 4)
#define DWC3_TRB_CTRL_ISP_IMI		BIT(10)
#define DWC3_TRB_CTRL_IOC		BIT(11)
#define DWC3_TRB_CTRL_SID_SOFN(n)	(((n) & 0xffff) << 14)

#define DWC3_TRBCTL_TYPE(n)		((n) & (0x3f << 4))
#define DWC3_TRBCTL_NORMAL		DWC3_TRB_CTRL_TRBCTL(1)
#define DWC3_TRBCTL_CONTROL_SETUP	DWC3_TRB_CTRL_TRBCTL(2)
#define DWC3_TRBCTL_CONTROL_STATUS2	DWC3_TRB_CTRL_TRBCTL(3)
#define DWC3_TRBCTL_CONTROL_STATUS3	DWC3_TRB_CTRL_TRBCTL(4)
#define DWC3_TRBCTL_CONTROL_DATA	DWC3_TRB_CTRL_TRBCTL(5)
#define DWC3_TRBCTL_ISOCHRONOUS_FIRST	DWC3_TRB_CTRL_TRBCTL(6)
#define DWC3_TRBCTL_ISOCHRONOUS		DWC3_TRB_CTRL_TRBCTL(7)
#define DWC3_TRBCTL_LINK_TRB		DWC3_TRB_CTRL_TRBCTL(8)

/**
 * struct dwc3_trb - transfer request block (hw format)
 * @bpl: DW0-3
 * @bph: DW4-7
 * @size: DW8-B
 * @ctrl: DWC-F
 */
struct dwc3_trb {
	u32		bpl;
	u32		bph;
	u32		size;
	u32		ctrl;
} __packed;

/**
 * struct dwc3_hwparams - copy of HWPARAMS registers
 * @hwparams0: GHWPARAMS0
 * @hwparams1: GHWPARAMS1
 * @hwparams2: GHWPARAMS2
 * @hwparams3: GHWPARAMS3
 * @hwparams4: GHWPARAMS4
 * @hwparams5: GHWPARAMS5
 * @hwparams6: GHWPARAMS6
 * @hwparams7: GHWPARAMS7
 * @hwparams8: GHWPARAMS8
 */
struct dwc3_hwparams {
	u32	hwparams0;
	u32	hwparams1;
	u32	hwparams2;
	u32	hwparams3;
	u32	hwparams4;
	u32	hwparams5;
	u32	hwparams6;
	u32	hwparams7;
	u32	hwparams8;
};

/* HWPARAMS0 */
#define DWC3_MODE(n)		((n) & 0x7)

#define DWC3_MDWIDTH(n)		(((n) & 0xff00) >> 8)

/* HWPARAMS1 */
#define DWC3_NUM_INT(n)		(((n) & (0x3f << 15)) >> 15)

/* HWPARAMS3 */
#define DWC3_NUM_IN_EPS_MASK	(0x1f << 18)
#define DWC3_NUM_EPS_MASK	(0x3f << 12)
#define DWC3_NUM_EPS(p)		(((p)->hwparams3 &		\
			(DWC3_NUM_EPS_MASK)) >> 12)
#define DWC3_NUM_IN_EPS(p)	(((p)->hwparams3 &		\
			(DWC3_NUM_IN_EPS_MASK)) >> 18)

/* HWPARAMS7 */
#define DWC3_RAM1_DEPTH(n)	((n) & 0xffff)

/**
 * struct dwc3_request - representation of a transfer request
 * @request: struct usb_request to be transferred
 * @list: a list_head used for request queueing
 * @dep: struct dwc3_ep owning this request
 * @sg: pointer to first incomplete sg
 * @start_sg: pointer to the sg which should be queued next
 * @num_pending_sgs: counter to pending sgs
 * @num_queued_sgs: counter to the number of sgs which already got queued
 * @remaining: amount of data remaining
 * @status: internal dwc3 request status tracking
 * @epnum: endpoint number to which this request refers
 * @trb: pointer to struct dwc3_trb
 * @trb_dma: DMA address of @trb
 * @num_trbs: number of TRBs used by this request
 * @needs_extra_trb: true when request needs one extra TRB (either due to ZLP
 *	or unaligned OUT)
 * @direction: IN or OUT direction flag
 * @mapped: true when request has been dma-mapped
 * @started: request is started
 */
struct dwc3_request {
	struct usb_request	request;
	struct list_head	list;
	struct dwc3_ep		*dep;
	struct scatterlist	*sg;
	struct scatterlist	*start_sg;

	unsigned		num_pending_sgs;
	unsigned int		num_queued_sgs;
	unsigned		remaining;

	unsigned int		status;
#define DWC3_REQUEST_STATUS_QUEUED	0
#define DWC3_REQUEST_STATUS_STARTED	1
#define DWC3_REQUEST_STATUS_CANCELLED	2
#define DWC3_REQUEST_STATUS_COMPLETED	3
#define DWC3_REQUEST_STATUS_UNKNOWN	-1

	u8			epnum;
	struct dwc3_trb		*trb;
	dma_addr_t		trb_dma;

	unsigned		num_trbs;

	unsigned		needs_extra_trb:1;
	unsigned		direction:1;
	unsigned		mapped:1;
	unsigned		started:1;
};

#define MAX_RETRY_CNT 5

/*
 * struct dwc3_scratchpad_array - hibernation scratchpad array
 * (format defined by hw)
 */
struct dwc3_scratchpad_array {
	__le64	dma_adr[DWC3_MAX_HIBER_SCRATCHBUFS];
};

/**
 * struct dwc3 - representation of our controller
 * @drd_work: workqueue used for role swapping
 * @ep0_trb: trb which is used for the ctrl_req
 * @bounce: address of bounce buffer
 * @scratchbuf: address of scratch buffer
 * @setup_buf: used while precessing STD USB requests
 * @ep0_trb_addr: dma address of @ep0_trb
 * @bounce_addr: dma address of @bounce
 * @ep0_usb_req: dummy req used while handling STD USB requests
 * @scratch_addr: dma address of scratchbuf
 * @ep0_in_setup: one control transfer is completed and enter setup phase
 * @lock: for synchronizing
 * @dev: pointer to our struct device
 * @sysdev: pointer to the DMA-capable device
 * @xhci: pointer to our xHCI child
 * @xhci_resources: struct resources for our @xhci child
 * @ev_buf: struct dwc3_event_buffer pointer
 * @eps: endpoint array
  * @dotg: pointer to OTG
 * @gadget: device side representation of the peripheral controller
 * @gadget_driver: pointer to the gadget driver
 * @clks: array of clocks
 * @num_clks: number of clocks
 * @reset: reset control
 * @regs: base address for our registers
 * @regs_size: address space size
 * @fladj: frame length adjustment
 * @irq_gadget: peripheral controller's IRQ number
 * @otg_irq: IRQ number for OTG IRQs
 * @current_otg_role: current role of operation while using the OTG block
 * @desired_otg_role: desired role of operation while using the OTG block
 * @otg_restart_host: flag that OTG controller needs to restart host
 * @nr_scratch: number of scratch buffers
 * @u1u2: only used on revisions <1.83a for workaround
 * @maximum_speed: maximum speed requested (mainly for testing purposes)
 * @revision: revision register contents
 * @dr_mode: requested mode of operation
 * @current_dr_role: current role of operation when in dual-role mode
 * @desired_dr_role: desired role of operation when in dual-role mode
 * @edev: extcon handle
 * @edev_nb: extcon notifier
 * @hsphy_mode: UTMI phy mode, one of following:
 *		- USBPHY_INTERFACE_MODE_UTMI
 *		- USBPHY_INTERFACE_MODE_UTMIW
 * @usb2_phy: pointer to USB2 PHY
 * @usb3_phy: pointer to USB3 PHY
 * @usb2_generic_phy: pointer to USB2 PHY
 * @usb3_generic_phy: pointer to USB3 PHY
 * @ulpi: pointer to ulpi interface
 * @u2sel: parameter from Set SEL request.
 * @u2pel: parameter from Set SEL request.
 * @u1sel: parameter from Set SEL request.
 * @u1pel: parameter from Set SEL request.
 * @num_eps: number of endpoints
 * @ep0_next_event: hold the next expected event
 * @ep0state: state of endpoint zero
 * @link_state: link state
 * @speed: device speed (super, high, full, low)
 * @hwparams: copy of hwparams registers
 * @root: debugfs root folder pointer
 * @regset: debugfs pointer to regdump file
 * @test_mode: true when we're entering a USB test mode
 * @test_mode_nr: test feature selector
 * @lpm_nyet_threshold: LPM NYET response threshold
 * @hird_threshold: HIRD threshold
 * @rx_thr_num_pkt_prd: periodic ESS receive packet count
 * @rx_max_burst_prd: max periodic ESS receive burst size
 * @tx_thr_num_pkt_prd: periodic ESS transmit packet count
 * @tx_max_burst_prd: max periodic ESS transmit burst size
 * @hsphy_interface: "utmi" or "ulpi"
 * @vbus_session: Indicates if the gadget was powered by the otg driver
 * @softconnect: Indicates if pullup was issued by the usb_gadget_driver
 * @disconnect: signals that Disconnection interrupt happend
 * @suspend_clk_freq: frequency of suspend clock
 * @connected: true when we're connected to a host, false otherwise
 * @delayed_status: true when gadget driver asks for delayed status
 * @ep0_bounced: true when we used bounce buffer
 * @ep0_expect_in: true when we expect a DATA IN transfer
 * @has_hibernation: true when dwc3 was configured with Hibernation
 * @sysdev_is_parent: true when dwc3 device has a parent driver
 * @has_lpm_erratum: true when core was configured with LPM Erratum. Note that
 *			there's now way for software to detect this in runtime.
 * @is_utmi_l1_suspend: the core asserts output signal
 * 	0	- utmi_sleep_n
 * 	1	- utmi_l1_suspend_n
 * @is_fpga: true when we are using the FPGA board
 * @pending_events: true when we have pending IRQs to be handled
 * @pullups_connected: true when Run/Stop bit is set
 * @setup_packet_pending: true when there's a Setup Packet in FIFO. Workaround
 * @three_stage_setup: set if we perform a three phase setup
 * @usb3_lpm_capable: set if hadrware supports Link Power Management
 * @disable_scramble_quirk: set if we enable the disable scramble quirk
 * @u2exit_lfps_quirk: set if we enable u2exit lfps quirk
 * @u2ss_inp3_quirk: set if we enable P3 OK for U2/SS Inactive quirk
 * @req_p1p2p3_quirk: set if we enable request p1p2p3 quirk
 * @del_p1p2p3_quirk: set if we enable delay p1p2p3 quirk
 * @u1u2_exitfail_to_recov_quirk: set if we enable u1u2 exitfail to recov quirk
 * @ux_exit_in_px_quirk: set in MK, LT
 * @elastic_buf_mode_quirk: set in MK
 * @del_phy_power_chg_quirk: set if we enable delay phy power change quirk
 * @lfps_filter_quirk: set if we enable LFPS filter quirk
 * @rx_detect_poll_quirk: set if we enable rx_detect to polling lfps quirk
 * @dis_u3_susphy_quirk: set if we disable usb3 suspend phy
 * @dis_u2_susphy_quirk: set if we disable usb2 suspend phy
 * @dis_enblslpm_quirk: set if we clear enblslpm in GUSB2PHYCFG,
 *                      disabling the suspend signal to the PHY.
 * @dis_rxdet_inp3_quirk: set if we disable Rx.Detect in P3
 * @dis_u2_freeclk_exists_quirk : set if we clear u2_freeclk_exists
 *			in GUSB2PHYCFG, specify that USB2 PHY doesn't
 *			provide a free-running PHY clock.
 * @dis_del_phy_power_chg_quirk: set if we disable delay phy power
 *			change quirk.
 * @dis_tx_ipgap_linecheck_quirk: set if we disable u2mac linestate
 *			check during HS transmit.
 * @tx_de_emphasis_quirk: set if we enable Tx de-emphasis quirk
 * @tx_de_emphasis: Tx de-emphasis value
 * 	0	- -6dB de-emphasis
 * 	1	- -3.5dB de-emphasis
 * 	2	- No de-emphasis
 * 	3	- Reserved
 * @dis_metastability_quirk: set to disable metastability quirk.
 * @imod_interval: set the interrupt moderation interval in 250ns
 *                 increments or 0 to disable.
 * @adj_sof_accuracy: set to adjust sof accuracy
 * @no_extra_delay: No Extra Delay Between SOF and the first packet.
 * @usb_host_device_timeout: timeout value to determine how long the host waits
 *                  for response from Enhanced SuperSpeed device.
 */
struct dwc3 {
	struct work_struct	drd_work;
	struct dwc3_trb		*ep0_trb;
	void			*bounce;
	void			*scratchbuf;
	u8			*setup_buf;
	dma_addr_t		ep0_trb_addr;
	dma_addr_t		bounce_addr;
	dma_addr_t		scratch_addr;
	struct dwc3_request	ep0_usb_req;
	struct completion	ep0_in_setup;

	/* device lock */
	spinlock_t		lock;

	struct device		*dev;
	struct device		*sysdev;

	struct platform_device	*xhci;
	struct resource		xhci_resources[DWC3_XHCI_RESOURCES_NUM];

	struct dwc3_event_buffer *ev_buf;
	struct dwc3_ep		*eps[DWC3_ENDPOINTS_NUM];
	struct dwc3_otg		*dotg;

	struct usb_gadget	gadget;
	struct usb_gadget_driver *gadget_driver;

	struct clk_bulk_data	*clks;
	int			num_clks;

	struct reset_control	*reset;

	struct usb_phy		*usb2_phy;
	struct usb_phy		*usb3_phy;

	struct phy		*usb2_generic_phy;
	struct phy		*usb3_generic_phy;

	bool			phys_ready;

	struct ulpi		*ulpi;
	bool			ulpi_ready;

	void __iomem		*regs;
	size_t			regs_size;

	enum usb_dr_mode	dr_mode;
	u32			current_dr_role;
	u32			desired_dr_role;
	struct extcon_dev	*edev;
	struct notifier_block	edev_nb;
	enum usb_phy_interface	hsphy_mode;

#ifdef CONFIG_USB_CONFIGFS_F_MBIM
	int			retry_cnt;
	struct work_struct		work;
#endif

	u32			fladj;
	u32			irq_gadget;
	u32			otg_irq;
	u32			current_otg_role;
	u32			desired_otg_role;
	bool			otg_restart_host;
	u32			nr_scratch;
	u32			u1u2;
	u32			maximum_speed;

	/*
	 * All 3.1 IP version constants are greater than the 3.0 IP
	 * version constants. This works for most version checks in
	 * dwc3. However, in the future, this may not apply as
	 * features may be developed on newer versions of the 3.0 IP
	 * that are not in the 3.1 IP.
	 */
	u32			revision;

#define DWC3_REVISION_173A	0x5533173a
#define DWC3_REVISION_175A	0x5533175a
#define DWC3_REVISION_180A	0x5533180a
#define DWC3_REVISION_183A	0x5533183a
#define DWC3_REVISION_185A	0x5533185a
#define DWC3_REVISION_187A	0x5533187a
#define DWC3_REVISION_188A	0x5533188a
#define DWC3_REVISION_190A	0x5533190a
#define DWC3_REVISION_194A	0x5533194a
#define DWC3_REVISION_200A	0x5533200a
#define DWC3_REVISION_202A	0x5533202a
#define DWC3_REVISION_210A	0x5533210a
#define DWC3_REVISION_220A	0x5533220a
#define DWC3_REVISION_230A	0x5533230a
#define DWC3_REVISION_240A	0x5533240a
#define DWC3_REVISION_250A	0x5533250a
#define DWC3_REVISION_260A	0x5533260a
#define DWC3_REVISION_270A	0x5533270a
#define DWC3_REVISION_280A	0x5533280a
#define DWC3_REVISION_290A	0x5533290a
#define DWC3_REVISION_300A	0x5533300a
#define DWC3_REVISION_310A	0x5533310a

/*
 * NOTICE: we're using bit 31 as a "is usb 3.1" flag. This is really
 * just so dwc31 revisions are always larger than dwc3.
 */
#define DWC3_REVISION_IS_DWC31		0x80000000
#define DWC3_USB31_REVISION_110A	(0x3131302a | DWC3_REVISION_IS_DWC31)
#define DWC3_USB31_REVISION_120A	(0x3132302a | DWC3_REVISION_IS_DWC31)

	enum dwc3_ep0_next	ep0_next_event;
	enum dwc3_ep0_state	ep0state;
	enum dwc3_link_state	link_state;
	u16 max_cnt_link_info;
	unsigned int linkstate_record[DWC3_LINK_STATE_LAST_INFO_MEM];
	u8 linkstate_ai;

	u16			u2sel;
	u16			u2pel;
	u8			u1sel;
	u8			u1pel;

	u8			speed;
	u8			remote_wakeup_set;
	u8			usb_remote_wakeup;

	u8			num_eps;

	struct dwc3_hwparams	hwparams;
	struct dentry		*root;
	struct debugfs_regset32	*regset;

	u8			test_mode;
	u8			test_mode_nr;
	u8			lpm_nyet_threshold;
	u8			hird_threshold;
	u8			rx_thr_num_pkt_prd;
	u8			rx_max_burst_prd;
	u8			tx_thr_num_pkt_prd;
	u8			tx_max_burst_prd;

	const char		*hsphy_interface;
	bool			vbus_session;
	bool			softconnect;

	struct completion	disconnect;

	/**
	 * Frequency of suspend clock.
	 * Suspend clock is a clock source of 16KHz clock for a small part
	 * of the USB3 core that operates when the SS PHY is in its lowest
	 * power (P3) state.
	 */
	u32			suspend_clk_freq;
	unsigned		connected:1;
	unsigned		delayed_status:1;
	unsigned		ep0_bounced:1;
	unsigned		ep0_expect_in:1;
	unsigned		has_hibernation:1;
	unsigned		sysdev_is_parent:1;
	unsigned		has_lpm_erratum:1;
	unsigned		is_utmi_l1_suspend:1;
	unsigned		is_fpga:1;
	unsigned		pending_events:1;
	unsigned		pullups_connected:1;
	unsigned		setup_packet_pending:1;
	unsigned		three_stage_setup:1;
	unsigned		usb3_lpm_capable:1;

	unsigned		disable_scramble_quirk:1;
	unsigned		u2exit_lfps_quirk:1;
	unsigned		u2ss_inp3_quirk:1;
	unsigned		req_p1p2p3_quirk:1;
	unsigned        	del_p1p2p3_quirk:1;
	unsigned 		u1u2_exitfail_to_recov_quirk:1;
	unsigned		ux_exit_in_px_quirk:1;
	unsigned		elastic_buf_mode_quirk:1;
	unsigned		del_phy_power_chg_quirk:1;
	unsigned		lfps_filter_quirk:1;
	unsigned		rx_detect_poll_quirk:1;
	unsigned		dis_u3_susphy_quirk:1;
	unsigned		dis_u2_susphy_quirk:1;
	unsigned		dis_enblslpm_quirk:1;
	unsigned		dis_rxdet_inp3_quirk:1;
	unsigned		dis_u2_freeclk_exists_quirk:1;
	unsigned		dis_del_phy_power_chg_quirk:1;
	unsigned		dis_tx_ipgap_linecheck_quirk:1;

	unsigned		tx_de_emphasis_quirk:1;
	unsigned		tx_de_emphasis:2;

	unsigned		dis_metastability_quirk:1;

	u16			imod_interval;

	unsigned		adj_sof_accuracy:1;

	unsigned		sparse_transfer_control:1;
	unsigned		is_not_vbus_pad:1;
	unsigned		start_config_issued:1;

	unsigned		no_extra_delay:1; 
	unsigned		force_gen1:1; /* Force GEN1 */
	u32			usb_host_device_timeout;

	struct workqueue_struct	*int_qos_lock_wq;
	struct work_struct	int_qos_work;
	int level_val;
	struct work_struct      set_vbus_current_work;
	int			vbus_current; /* 100mA,  500mA,  900mA */
#ifdef CONFIG_USB_CONFIGFS_F_MBIM
	struct delayed_work		dwc3_reset_delayed_work;
#endif

#ifdef CONFIG_USB_SS_REMOTE_WAKEUP
	u8			remote_wakeup_fail;
	int 			interface_number;
#endif
	struct delayed_work usb_event_work;
	ktime_t rst_time_before;
	ktime_t rst_time_first;
	int rst_err_cnt;
	bool rst_err_noti;
	bool event_state;
};

#define ERR_RESET_CNT	3
#define INCRX_BURST_MODE 0
#define INCRX_UNDEF_LENGTH_BURST_MODE 1

#define work_to_dwc(w)		(container_of((w), struct dwc3, drd_work))

/* -------------------------------------------------------------------------- */

struct dwc3_event_type {
	u32	is_devspec:1;
	u32	type:7;
	u32	reserved8_31:24;
} __packed;

#define DWC3_DEPEVT_XFERCOMPLETE	0x01
#define DWC3_DEPEVT_XFERINPROGRESS	0x02
#define DWC3_DEPEVT_XFERNOTREADY	0x03
#define DWC3_DEPEVT_RXTXFIFOEVT		0x04
#define DWC3_DEPEVT_STREAMEVT		0x06
#define DWC3_DEPEVT_EPCMDCMPLT		0x07

/**
 * struct dwc3_event_depvt - Device Endpoint Events
 * @one_bit: indicates this is an endpoint event (not used)
 * @endpoint_number: number of the endpoint
 * @endpoint_event: The event we have:
 *	0x00	- Reserved
 *	0x01	- XferComplete
 *	0x02	- XferInProgress
 *	0x03	- XferNotReady
 *	0x04	- RxTxFifoEvt (IN->Underrun, OUT->Overrun)
 *	0x05	- Reserved
 *	0x06	- StreamEvt
 *	0x07	- EPCmdCmplt
 * @reserved11_10: Reserved, don't use.
 * @status: Indicates the status of the event. Refer to databook for
 *	more information.
 * @parameters: Parameters of the current event. Refer to databook for
 *	more information.
 */
struct dwc3_event_depevt {
	u32	one_bit:1;
	u32	endpoint_number:5;
	u32	endpoint_event:4;
	u32	reserved11_10:2;
	u32	status:4;

/* Within XferNotReady */
#define DEPEVT_STATUS_TRANSFER_ACTIVE	BIT(3)

/* Within XferComplete or XferInProgress */
#define DEPEVT_STATUS_BUSERR	BIT(0)
#define DEPEVT_STATUS_SHORT	BIT(1)
#define DEPEVT_STATUS_IOC	BIT(2)
#define DEPEVT_STATUS_LST	BIT(3) /* XferComplete */
#define DEPEVT_STATUS_MISSED_ISOC BIT(3) /* XferInProgress */

/* Stream event only */
#define DEPEVT_STREAMEVT_FOUND		1
#define DEPEVT_STREAMEVT_NOTFOUND	2

/* Control-only Status */
#define DEPEVT_STATUS_CONTROL_DATA	1
#define DEPEVT_STATUS_CONTROL_STATUS	2
#define DEPEVT_STATUS_CONTROL_PHASE(n)	((n) & 3)

/* In response to Start Transfer */
#define DEPEVT_TRANSFER_NO_RESOURCE	1
#define DEPEVT_TRANSFER_BUS_EXPIRY	2

	u32	parameters:16;

/* For Command Complete Events */
#define DEPEVT_PARAMETER_CMD(n)	(((n) & (0xf << 8)) >> 8)
} __packed;

/**
 * struct dwc3_event_devt - Device Events
 * @one_bit: indicates this is a non-endpoint event (not used)
 * @device_event: indicates it's a device event. Should read as 0x00
 * @type: indicates the type of device event.
 *	0	- DisconnEvt
 *	1	- USBRst
 *	2	- ConnectDone
 *	3	- ULStChng
 *	4	- WkUpEvt
 *	5	- Reserved
 *	6	- EOPF
 *	7	- SOF
 *	8	- Reserved
 *	9	- ErrticErr
 *	10	- CmdCmplt
 *	11	- EvntOverflow
 *	12	- VndrDevTstRcved
 * @reserved15_12: Reserved, not used
 * @event_info: Information about this event
 * @reserved31_25: Reserved, not used
 */
struct dwc3_event_devt {
	u32	one_bit:1;
	u32	device_event:7;
	u32	type:4;
	u32	reserved15_12:4;
	u32	event_info:9;
	u32	reserved31_25:7;
} __packed;

/**
 * struct dwc3_event_gevt - Other Core Events
 * @one_bit: indicates this is a non-endpoint event (not used)
 * @device_event: indicates it's (0x03) Carkit or (0x04) I2C event.
 * @phy_port_number: self-explanatory
 * @reserved31_12: Reserved, not used.
 */
struct dwc3_event_gevt {
	u32	one_bit:1;
	u32	device_event:7;
	u32	phy_port_number:4;
	u32	reserved31_12:20;
} __packed;

/**
 * union dwc3_event - representation of Event Buffer contents
 * @raw: raw 32-bit event
 * @type: the type of the event
 * @depevt: Device Endpoint Event
 * @devt: Device Event
 * @gevt: Global Event
 */
union dwc3_event {
	u32				raw;
	struct dwc3_event_type		type;
	struct dwc3_event_depevt	depevt;
	struct dwc3_event_devt		devt;
	struct dwc3_event_gevt		gevt;
};

/**
 * struct dwc3_gadget_ep_cmd_params - representation of endpoint command
 * parameters
 * @param2: third parameter
 * @param1: second parameter
 * @param0: first parameter
 */
struct dwc3_gadget_ep_cmd_params {
	u32	param2;
	u32	param1;
	u32	param0;
};

/*
 * DWC3 Features to be used as Driver Data
 */

#define DWC3_HAS_PERIPHERAL		BIT(0)
#define DWC3_HAS_XHCI			BIT(1)
#define DWC3_HAS_OTG			BIT(3)

/* prototypes */
void dwc3_set_prtcap(struct dwc3 *dwc, u32 mode);
void dwc3_set_mode(struct dwc3 *dwc, u32 mode);
int dwc3_soft_reset(struct dwc3 *dwc);
int dwc3_event_buffers_setup(struct dwc3 *dwc);
void dwc3_event_buffers_cleanup(struct dwc3 *dwc);
int dwc3_phy_setup(struct dwc3 *dwc);
int dwc3_core_init(struct dwc3 *dwc);
void dwc3_core_exit(struct dwc3 *dwc);
int dwc3_otg_start(struct dwc3 *dwc);
void dwc3_otg_stop(struct dwc3 *dwc);
//void dwc3_otg_init(struct dwc3 *dwc);
//void dwc3_otg_exit(struct dwc3 *dwc);

u32 dwc3_core_fifo_space(struct dwc3_ep *dep, u8 type);

/* check whether we are on the DWC_usb3 core */
static inline bool dwc3_is_usb3(struct dwc3 *dwc)
{
	return !(dwc->revision & DWC3_REVISION_IS_DWC31);
}

/* check whether we are on the DWC_usb31 core */
static inline bool dwc3_is_usb31(struct dwc3 *dwc)
{
	return !!(dwc->revision & DWC3_REVISION_IS_DWC31);
}

bool dwc3_has_imod(struct dwc3 *dwc);

int dwc3_event_buffers_setup(struct dwc3 *dwc);
void dwc3_event_buffers_cleanup(struct dwc3 *dwc);

#if IS_ENABLED(CONFIG_USB_DWC3_HOST) || IS_ENABLED(CONFIG_USB_DWC3_DUAL_ROLE)
int dwc3_host_init(struct dwc3 *dwc);
void dwc3_host_exit(struct dwc3 *dwc);
#else
static inline int dwc3_host_init(struct dwc3 *dwc)
{ return 0; }
static inline void dwc3_host_exit(struct dwc3 *dwc)
{ }
#endif

#if IS_ENABLED(CONFIG_USB_DWC3_GADGET) || IS_ENABLED(CONFIG_USB_DWC3_DUAL_ROLE)
int dwc3_gadget_init(struct dwc3 *dwc);
void dwc3_gadget_exit(struct dwc3 *dwc);
int dwc3_gadget_set_test_mode(struct dwc3 *dwc, int mode);
int dwc3_gadget_get_link_state(struct dwc3 *dwc);
int dwc3_gadget_set_link_state(struct dwc3 *dwc, enum dwc3_link_state state);
int dwc3_send_gadget_ep_cmd(struct dwc3_ep *dep, unsigned cmd,
		struct dwc3_gadget_ep_cmd_params *params);
int dwc3_send_gadget_generic_command(struct dwc3 *dwc, unsigned cmd, u32 param);
void dwc3_gadget_disconnect_proc(struct dwc3 *dwc);
int dwc3_core_susphy_set(struct dwc3 *dwc, int on);
#else
static inline int dwc3_gadget_init(struct dwc3 *dwc)
{ return 0; }
static inline void dwc3_gadget_exit(struct dwc3 *dwc)
{ }
static inline int dwc3_gadget_set_test_mode(struct dwc3 *dwc, int mode)
{ return 0; }
static inline int dwc3_gadget_get_link_state(struct dwc3 *dwc)
{ return 0; }
static inline int dwc3_gadget_set_link_state(struct dwc3 *dwc,
		enum dwc3_link_state state)
{ return 0; }

static inline int dwc3_send_gadget_ep_cmd(struct dwc3_ep *dep, unsigned cmd,
		struct dwc3_gadget_ep_cmd_params *params)
{ return 0; }
static inline int dwc3_send_gadget_generic_command(struct dwc3 *dwc,
		int cmd, u32 param)
{ return 0; }
static inline void dwc3_gadget_disconnect_proc(struct dwc3 *dwc)
{ }
static inline int dwc3_core_susphy_set(struct dwc3 *dwc, int on)
{ return 0; }
#endif

#if IS_ENABLED(CONFIG_USB_DWC3_DUAL_ROLE)
int dwc3_drd_init(struct dwc3 *dwc);
void dwc3_drd_exit(struct dwc3 *dwc);
int dwc3_otg_init(struct dwc3 *dwc);
void dwc3_otg_exit(struct dwc3 *dwc);
void dwc3_otg_update(struct dwc3 *dwc, bool ignore_idstatus);
void dwc3_otg_host_init(struct dwc3 *dwc);
#else
static inline int dwc3_drd_init(struct dwc3 *dwc)
{ return 0; }
static inline void dwc3_drd_exit(struct dwc3 *dwc)
{ }
//static inline void dwc3_otg_init(struct dwc3 *dwc)
//{ }
//static inline void dwc3_otg_exit(struct dwc3 *dwc)
//{ }
static inline void dwc3_otg_update(struct dwc3 *dwc, bool ignore_idstatus)
{ }
static inline void dwc3_otg_host_init(struct dwc3 *dwc)
{ }
#endif

/* power management interface */
#if !IS_ENABLED(CONFIG_USB_DWC3_HOST)
int dwc3_gadget_suspend(struct dwc3 *dwc);
int dwc3_gadget_resume(struct dwc3 *dwc);
void dwc3_gadget_process_pending_events(struct dwc3 *dwc);
#else
static inline int dwc3_gadget_suspend(struct dwc3 *dwc)
{
	return 0;
}

static inline int dwc3_gadget_resume(struct dwc3 *dwc)
{
	return 0;
}

static inline void dwc3_gadget_process_pending_events(struct dwc3 *dwc)
{
}
#endif /* !IS_ENABLED(CONFIG_USB_DWC3_HOST) */

#if IS_ENABLED(CONFIG_USB_DWC3_ULPI)
int dwc3_ulpi_init(struct dwc3 *dwc);
void dwc3_ulpi_exit(struct dwc3 *dwc);
#else
static inline int dwc3_ulpi_init(struct dwc3 *dwc)
{ return 0; }
static inline void dwc3_ulpi_exit(struct dwc3 *dwc)
{ }
#endif

#endif /* __DRIVERS_USB_DWC3_CORE_H */