/*	$OpenBSD: glxreg.h,v 1.2 2010/02/12 19:37:29 miod Exp $	*/

/*
 * Copyright (c) 2009 Miodrag Vallat.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * AMD 5536 Geode companion chip MSR registers
 */

/*
 * Base addresses of the MSR groups.
 */

#ifdef __loongson__
#define	SB_MSR_BASE			0x00000000
#define	GLIU_MSR_BASE			0x10000000
#define	USB_MSR_BASE			0x40000000
#define	IDE_MSR_BASE			0x60000000
#define	DIVIL_MSR_BASE			0x80000000
#define	ACC_MSR_BASE			0xa0000000
#define	GLCP_MSR_BASE			0xe0000000
#else
#define	SB_MSR_BASE			0x51000000
#define	GLIU_MSR_BASE			0x51010000
#define	USB_MSR_BASE			0x51200000
#define	IDE_MSR_BASE			0x51300000
#define	DIVIL_MSR_BASE			0x51400000
#define	ACC_MSR_BASE			0x51500000
#define	GLCP_MSR_BASE			0x51700000
#endif

/*
 * GeodeLink Interface Unit (GLIU)
 */

#define	GLIU_GLD_MSR_CAP		(GLIU_MSR_BASE + 0x00)
#define	GLIU_GLD_MSR_CONFIG		(GLIU_MSR_BASE + 0x01)
#define	GLIU_GLD_MSR_SMI		(GLIU_MSR_BASE + 0x02)
#define	GLIU_GLD_MSR_ERROR		(GLIU_MSR_BASE + 0x03)
#define	GLIU_GLD_MSR_PM			(GLIU_MSR_BASE + 0x04)
#define	GLIU_GLD_MSR_DIAG		(GLIU_MSR_BASE + 0x05)

#define	GLIU_P2D_BM0			(GLIU_MSR_BASE + 0x20)
#define	GLIU_P2D_BM1			(GLIU_MSR_BASE + 0x21)
#define	GLIU_P2D_BM2			(GLIU_MSR_BASE + 0x22)
#define	GLIU_P2D_BMK0			(GLIU_MSR_BASE + 0x23)
#define	GLIU_P2D_BMK1			(GLIU_MSR_BASE + 0x24)
#define	GLIU_P2D_BM3			(GLIU_MSR_BASE + 0x25)
#define	GLIU_P2D_BM4			(GLIU_MSR_BASE + 0x26)

#define	GLIU_COH			(GLIU_MSR_BASE + 0x80)
#define	GLIU_PAE			(GLIU_MSR_BASE + 0x81)
#define	GLIU_ARB			(GLIU_MSR_BASE + 0x82)
#define	GLIU_ASMI			(GLIU_MSR_BASE + 0x83)
#define	GLIU_AERR			(GLIU_MSR_BASE + 0x84)
#define	GLIU_DEBUG			(GLIU_MSR_BASE + 0x85)
#define	GLIU_PHY_CAP			(GLIU_MSR_BASE + 0x86)
#define	GLIU_NOUT_RESP			(GLIU_MSR_BASE + 0x87)
#define	GLIU_NOUT_WDATA			(GLIU_MSR_BASE + 0x88)
#define	GLIU_WHOAMI			(GLIU_MSR_BASE + 0x8b)
#define	GLIU_SLV_DIS			(GLIU_MSR_BASE + 0x8c)
#define	GLIU_STATISTIC_CNT0		(GLIU_MSR_BASE + 0xa0)
#define	GLIU_STATISTIC_MASK0		(GLIU_MSR_BASE + 0xa1)
#define	GLIU_STATISTIC_ACTION0		(GLIU_MSR_BASE + 0xa2)
#define	GLIU_STATISTIC_CNT1		(GLIU_MSR_BASE + 0xa4)
#define	GLIU_STATISTIC_MASK1		(GLIU_MSR_BASE + 0xa5)
#define	GLIU_STATISTIC_ACTION1		(GLIU_MSR_BASE + 0xa6)
#define	GLIU_STATISTIC_CNT2		(GLIU_MSR_BASE + 0xa8)
#define	GLIU_STATISTIC_MASK2		(GLIU_MSR_BASE + 0xa9)
#define	GLIU_STATISTIC_ACTION2		(GLIU_MSR_BASE + 0xaa)
#define	GLIU_RQ_COMP_VAL		(GLIU_MSR_BASE + 0xc0)
#define	GLIU_RQ_COMP_MASK		(GLIU_MSR_BASE + 0xc1)
#define	GLIU_DA_COMP_VAL_LO		(GLIU_MSR_BASE + 0xd0)
#define	GLIU_DA_COMP_VAL_HI		(GLIU_MSR_BASE + 0xd1)
#define	GLIU_DA_COMP_MASK_LO		(GLIU_MSR_BASE + 0xd2)
#define	GLIU_DA_COMP_MASK_HI		(GLIU_MSR_BASE + 0xd3)

#define	GLIU_IOD_BM0			(GLIU_MSR_BASE + 0xe0)
#define	GLIU_IOD_BM1			(GLIU_MSR_BASE + 0xe1)
#define	GLIU_IOD_BM2			(GLIU_MSR_BASE + 0xe2)
#define	GLIU_IOD_BM3			(GLIU_MSR_BASE + 0xe3)
#define	GLIU_IOD_BM4			(GLIU_MSR_BASE + 0xe4)
#define	GLIU_IOD_BM5			(GLIU_MSR_BASE + 0xe5)
#define	GLIU_IOD_BM6			(GLIU_MSR_BASE + 0xe6)
#define	GLIU_IOD_BM7			(GLIU_MSR_BASE + 0xe7)
#define	GLIU_IOD_BM8			(GLIU_MSR_BASE + 0xe8)
#define	GLIU_IOD_BM9			(GLIU_MSR_BASE + 0xe9)
#define	GLIU_IOD_SC0			(GLIU_MSR_BASE + 0xea)
#define	GLIU_IOD_SC1			(GLIU_MSR_BASE + 0xeb)
#define	GLIU_IOD_SC2			(GLIU_MSR_BASE + 0xec)
#define	GLIU_IOD_SC3			(GLIU_MSR_BASE + 0xed)
#define	GLIU_IOD_SC4			(GLIU_MSR_BASE + 0xee)
#define	GLIU_IOD_SC5			(GLIU_MSR_BASE + 0xef)
#define	GLIU_IOD_SC6			(GLIU_MSR_BASE + 0xf0)
#define	GLIU_IOD_SC7			(GLIU_MSR_BASE + 0xf1)

/*
 * GeodeLink PCI South Bridge (SB)
 */

#define	GLPCI_GLD_MSR_CAP		(SB_MSR_BASE + 0x00)
#define	GLPCI_GLD_MSR_CONFIG		(SB_MSR_BASE + 0x01)
#define	GLPCI_GLD_MSR_SMI		(SB_MSR_BASE + 0x02)
#define	GLPCI_GLD_MSR_ERROR		(SB_MSR_BASE + 0x03)
#define	GLPCI_GLD_MSR_PM		(SB_MSR_BASE + 0x04)
#define	GLPCI_GLD_MSR_DIAG		(SB_MSR_BASE + 0x05)

#define	GLPCI_CTRL			(SB_MSR_BASE + 0x10)
#define	GLPCI_R0			(SB_MSR_BASE + 0x20)
#define	GLPCI_R1			(SB_MSR_BASE + 0x21)
#define	GLPCI_R2			(SB_MSR_BASE + 0x22)
#define	GLPCI_R3			(SB_MSR_BASE + 0x23)
#define	GLPCI_R4			(SB_MSR_BASE + 0x24)
#define	GLPCI_R5			(SB_MSR_BASE + 0x25)
#define	GLPCI_R6			(SB_MSR_BASE + 0x26)
#define	GLPCI_R7			(SB_MSR_BASE + 0x27)
#define	GLPCI_R8			(SB_MSR_BASE + 0x28)
#define	GLPCI_R9			(SB_MSR_BASE + 0x29)
#define	GLPCI_R10			(SB_MSR_BASE + 0x2a)
#define	GLPCI_R11			(SB_MSR_BASE + 0x2b)
#define	GLPCI_R12			(SB_MSR_BASE + 0x2c)
#define	GLPCI_R13			(SB_MSR_BASE + 0x2d)
#define	GLPCI_R14			(SB_MSR_BASE + 0x2e)
#define	GLPCI_R15			(SB_MSR_BASE + 0x2f)
#define	GLPCI_PCIHEAD_BYTE0_3		(SB_MSR_BASE + 0x30)
#define	GLPCI_PCIHEAD_BYTE4_7		(SB_MSR_BASE + 0x31)
#define	GLPCI_PCIHEAD_BYTE8_B		(SB_MSR_BASE + 0x32)
#define	GLPCI_PCIHEAD_BYTEC_F		(SB_MSR_BASE + 0x33)

/*
 * AC97 Audio Codec Controller (ACC)
 */

#define	ACC_GLD_MSR_CAP			(ACC_MSR_BASE + 0x00)
#define	ACC_GLD_MSR_CONFIG		(ACC_MSR_BASE + 0x01)
#define	ACC_GLD_MSR_SMI			(ACC_MSR_BASE + 0x02)
#define	ACC_GLD_MSR_ERROR		(ACC_MSR_BASE + 0x03)
#define	ACC_GLD_MSR_PM			(ACC_MSR_BASE + 0x04)
#define	ACC_GLD_MSR_DIAG		(ACC_MSR_BASE + 0x05)

/*
 * USB Controller Registers (USB)
 */

#define	USB_GLD_MSR_CAP			(USB_MSR_BASE + 0x00)
#define	USB_GLD_MSR_CONFIG		(USB_MSR_BASE + 0x01)
#define	USB_GLD_MSR_SMI			(USB_MSR_BASE + 0x02)
#define	USB_GLD_MSR_ERROR		(USB_MSR_BASE + 0x03)
#define	USB_GLD_MSR_PM			(USB_MSR_BASE + 0x04)
#define	USB_GLD_MSR_DIAG		(USB_MSR_BASE + 0x05)

#define	USB_MSR_OHCB			(USB_MSR_BASE + 0x08)
#define	USB_MSR_EHCB			(USB_MSR_BASE + 0x09)
#define	USB_MSR_UDCB			(USB_MSR_BASE + 0x0a)
#define	USB_MSR_UOCB			(USB_MSR_BASE + 0x0b)

/*
 * IDE Controller Registers (IDE)
 */

#define	IDE_GLD_MSR_CAP			(IDE_MSR_BASE + 0x00)
#define	IDE_GLD_MSR_CONFIG		(IDE_MSR_BASE + 0x01)
#define	IDE_GLD_MSR_SMI			(IDE_MSR_BASE + 0x02)
#define	IDE_GLD_MSR_ERROR		(IDE_MSR_BASE + 0x03)
#define	IDE_GLD_MSR_PM			(IDE_MSR_BASE + 0x04)
#define	IDE_GLD_MSR_DIAG		(IDE_MSR_BASE + 0x05)

#define	IDE_IO_BAR			(IDE_MSR_BASE + 0x08)
#define	IDE_CFG				(IDE_MSR_BASE + 0x10)
#define	IDE_DTC				(IDE_MSR_BASE + 0x12)
#define	IDE_CAST			(IDE_MSR_BASE + 0x13)
#define	IDE_ETC				(IDE_MSR_BASE + 0x14)
#define	IDE_PM				(IDE_MSR_BASE + 0x15)

/*
 * Diverse Integration Logic (DIVIL)
 */

#define	DIVIL_GLD_MSR_CAP		(DIVIL_MSR_BASE + 0x00)
#define	DIVIL_GLD_MSR_CONFIG		(DIVIL_MSR_BASE + 0x01)
#define	DIVIL_GLD_MSR_SMI		(DIVIL_MSR_BASE + 0x02)
#define	DIVIL_GLD_MSR_ERROR		(DIVIL_MSR_BASE + 0x03)
#define	DIVIL_GLD_MSR_PM		(DIVIL_MSR_BASE + 0x04)
#define	DIVIL_GLD_MSR_DIAG		(DIVIL_MSR_BASE + 0x05)

#define	DIVIL_LBAR_IRQ			(DIVIL_MSR_BASE + 0x08)
#define	DIVIL_LBAR_KEL			(DIVIL_MSR_BASE + 0x09)
#define	DIVIL_LBAR_SMB			(DIVIL_MSR_BASE + 0x0b)
#define	DIVIL_LBAR_GPIO			(DIVIL_MSR_BASE + 0x0c)
#define	DIVIL_LBAR_MFGPT		(DIVIL_MSR_BASE + 0x0d)
#define	DIVIL_LBAR_ACPI			(DIVIL_MSR_BASE + 0x0e)
#define	DIVIL_LBAR_PMS			(DIVIL_MSR_BASE + 0x0f)
#define	DIVIL_LBAR_FLSH0		(DIVIL_MSR_BASE + 0x10)
#define	DIVIL_LBAR_FLSH1		(DIVIL_MSR_BASE + 0x11)
#define	DIVIL_LBAR_FLSH2		(DIVIL_MSR_BASE + 0x12)
#define	DIVIL_LBAR_FLSH3		(DIVIL_MSR_BASE + 0x13)
#define	DIVIL_LEG_IO			(DIVIL_MSR_BASE + 0x14)
#define	DIVIL_BALL_OPTS			(DIVIL_MSR_BASE + 0x15)
#define	DIVIL_SOFT_IRQ			(DIVIL_MSR_BASE + 0x16)
#define	DIVIL_SOFT_RESET		(DIVIL_MSR_BASE + 0x17)
#define	NORF_CTL			(DIVIL_MSR_BASE + 0x18)
#define	NORF_T01			(DIVIL_MSR_BASE + 0x19)
#define	NORF_T23			(DIVIL_MSR_BASE + 0x1a)
#define	NANDF_DATA			(DIVIL_MSR_BASE + 0x1b)
#define	NANDF_CTL			(DIVIL_MSR_BASE + 0x1c)
#define	NANDF_RSVD			(DIVIL_MSR_BASE + 0x1d)
#define	DIVIL_AC_DMA			(DIVIL_MSR_BASE + 0x1e)
#define	KELX_CTL			(DIVIL_MSR_BASE + 0x1f)
#define	PIC_YSEL_LOW			(DIVIL_MSR_BASE + 0x20)
#define	PIC_YSEL_HIGH			(DIVIL_MSR_BASE + 0x21)
#define	PIC_ZSEL_LOW			(DIVIL_MSR_BASE + 0x22)
#define	PIC_ZSEL_HIGH			(DIVIL_MSR_BASE + 0x23)
#define	PIC_IRQM_PRIM			(DIVIL_MSR_BASE + 0x24)
#define	PIC_IRQM_LPC			(DIVIL_MSR_BASE + 0x25)
#define	PIC_XIRR_STS_LOW		(DIVIL_MSR_BASE + 0x26)
#define	PIC_XIRR_STS_HIGH		(DIVIL_MSR_BASE + 0x27)
#define	MFGPT_IRQ			(DIVIL_MSR_BASE + 0x28)
#define	MFGPT_NR			(DIVIL_MSR_BASE + 0x29)
#define	MFGPT_RSVD			(DIVIL_MSR_BASE + 0x2a)
#define	MFGPT_SETUP			(DIVIL_MSR_BASE + 0x2b)
#define	FLPY_3F2_SHDW			(DIVIL_MSR_BASE + 0x30)
#define	FLPY_3F7_SHDW			(DIVIL_MSR_BASE + 0x31)
#define	FLPY_372_SHDW			(DIVIL_MSR_BASE + 0x32)
#define	FLPY_377_SHDW			(DIVIL_MSR_BASE + 0x33)
#define	PIC_SHDW			(DIVIL_MSR_BASE + 0x34)
#define	PIT_SHDW			(DIVIL_MSR_BASE + 0x36)
#define	PIT_CNTRL			(DIVIL_MSR_BASE + 0x37)
#define	UART1_MOD			(DIVIL_MSR_BASE + 0x38)
#define	UART1_DONG			(DIVIL_MSR_BASE + 0x39)
#define	UART1_CONF			(DIVIL_MSR_BASE + 0x3a)
#define	UART1_RSVD_MSR			(DIVIL_MSR_BASE + 0x3b)
#define	UART2_MOD			(DIVIL_MSR_BASE + 0x3c)
#define	UART2_DONG			(DIVIL_MSR_BASE + 0x3d)
#define	UART2_CONF			(DIVIL_MSR_BASE + 0x3e)
#define	UART2_RSVD_MSR			(DIVIL_MSR_BASE + 0x3f)
#define	DMA_MAP				(DIVIL_MSR_BASE + 0x40)
#define	DMA_SHDW_CH0			(DIVIL_MSR_BASE + 0x41)
#define	DMA_SHDW_CH1			(DIVIL_MSR_BASE + 0x42)
#define	DMA_SHDW_CH2			(DIVIL_MSR_BASE + 0x43)
#define	DMA_SHDW_CH3			(DIVIL_MSR_BASE + 0x44)
#define	DMA_SHDW_CH4			(DIVIL_MSR_BASE + 0x45)
#define	DMA_SHDW_CH5			(DIVIL_MSR_BASE + 0x46)
#define	DMA_SHDW_CH6			(DIVIL_MSR_BASE + 0x47)
#define	DMA_SHDW_CH7			(DIVIL_MSR_BASE + 0x48)
#define	DMA_MSK_SHDW			(DIVIL_MSR_BASE + 0x49)
#define	LPC_EADDR			(DIVIL_MSR_BASE + 0x4c)
#define	LPC_ESTAT			(DIVIL_MSR_BASE + 0x4d)
#define	LPC_SIRQ			(DIVIL_MSR_BASE + 0x4e)
#define	LPC_RSVD			(DIVIL_MSR_BASE + 0x4f)
#define	PMC_LTMR			(DIVIL_MSR_BASE + 0x50)
#define	PMC_RSVD			(DIVIL_MSR_BASE + 0x51)
#define	RTC_RAM_LOCK			(DIVIL_MSR_BASE + 0x54)
#define	RTC_DOMA_OFFSET			(DIVIL_MSR_BASE + 0x55)
#define	RTC_MONA_OFFSET			(DIVIL_MSR_BASE + 0x56)
#define	RTC_CEN_OFFSET			(DIVIL_MSR_BASE + 0x57)

/*
 * GeodeLink Control Processor (GLCP)
 */

#define	GLCP_GLD_MSR_CAP		(GLCP_MSR_BASE + 0x00)
#define	GLCP_GLD_MSR_CONFIG		(GLCP_MSR_BASE + 0x01)
#define	GLCP_GLD_MSR_SMI		(GLCP_MSR_BASE + 0x02)
#define	GLCP_GLD_MSR_ERROR		(GLCP_MSR_BASE + 0x03)
#define	GLCP_GLD_MSR_PM			(GLCP_MSR_BASE + 0x04)
#define	GLCP_GLD_MSR_DIAG		(GLCP_MSR_BASE + 0x05)

#define	GLCP_CLK_DIS_DELAY		(GLCP_MSR_BASE + 0x08)
#define	GLCP_PMCLKDISABLE		(GLCP_MSR_BASE + 0x09)
#define	GLCP_GLB_PM			(GLCP_MSR_BASE + 0x0b)
#define	GLCP_DBGOUT			(GLCP_MSR_BASE + 0x0c)
#define	GLCP_DOWSER			(GLCP_MSR_BASE + 0x0e)
#define	GLCP_CLKOFF			(GLCP_MSR_BASE + 0x10)
#define	GLCP_CLKACTIVE			(GLCP_MSR_BASE + 0x11)
#define	GLCP_CLKDISABLE			(GLCP_MSR_BASE + 0x12)
#define	GLCP_CLK4ACK			(GLCP_MSR_BASE + 0x13)
#define	GLCP_SYS_RST			(GLCP_MSR_BASE + 0x14)
#define	GLCP_DBGCLKCTRL			(GLCP_MSR_BASE + 0x16)
#define	GLCP_CHIP_REV_ID		(GLCP_MSR_BASE + 0x17)

/*
 * GPIO registers
 */

#define	GPIOL_OUT_VAL			0x0000
#define	GPIOL_OUT_EN			0x0004
#define	GPIOL_OUT_OD_EN			0x0008
#define	GPIOL_OUT_INVRT_EN		0x000c
#define	GPIOL_OUT_AUX1_SEL		0x0010
#define	GPIOL_OUT_AUX2_SEL		0x0014
#define	GPIOL_PU_EN			0x0018
#define	GPIOL_PD_EN			0x001c
#define	GPIOL_IN_EN			0x0020
#define	GPIOL_IN_INV_EN			0x0024
#define	GPIOL_IN_FLTR_EN		0x0028
#define	GPIOL_IN_EVNTCNT_EN		0x002c
#define	GPIOL_READ_BACK			0x0030
#define	GPIOL_IN_AUX1_SEL		0x0034
#define	GPIOL_EVNT_EN			0x0038
#define	GPIOL_LOCK_EN			0x003c
#define	GPIOL_POSEDGE_EN		0x0040
#define	GPIOL_NEGEDGE_EN		0x0044
#define	GPIOL_POSEDGE_STS		0x0048
#define	GPIOL_NEGEDGE_STS		0x004c
#define	GPIO_FLTR0_AMNT			0x0050
#define	GPIO_FLTR0_CNT			0x0052
#define	GPIO_EVNTCNT0			0x0054
#define	GPIO_EVNTCNT0_COMP		0x0056
#define	GPIO_FLTR1_AMNT			0x0058
#define	GPIO_FLTR1_CNT			0x005a
#define	GPIO_EVNTCNT1			0x005c
#define	GPIO_EVNTCNT1_COMP		0x005e
#define	GPIO_FLTR2_AMNT			0x0060
#define	GPIO_FLTR2_CNT			0x0062
#define	GPIO_EVNTCNT2			0x0064
#define	GPIO_EVNTCNT2_COMP		0x0066
#define	GPIO_FLTR3_AMNT			0x0068
#define	GPIO_FLTR3_CNT			0x006a
#define	GPIO_EVNTCNT3			0x006c
#define	GPIO_EVNTCNT3_COMP		0x006e
#define	GPIO_FLTR4_AMNT			0x0070
#define	GPIO_FLTR4_CNT			0x0072
#define	GPIO_EVNTCNT4			0x0074
#define	GPIO_EVNTCNT4_COMP		0x0076
#define	GPIO_FLTR5_AMNT			0x0078
#define	GPIO_FLTR5_CNT			0x007a
#define	GPIO_EVNTCNT5			0x007c
#define	GPIO_EVNTCNT5_COMP		0x007e
#define	GPIOH_OUT_VAL			0x0080
#define	GPIOH_OUT_EN			0x0084
#define	GPIOH_OUT_OD_EN			0x0088
#define	GPIOH_OUT_INVRT_EN		0x008c
#define	GPIOH_OUT_AUX1_SEL		0x0090
#define	GPIOH_OUT_AUX2_SEL		0x0094
#define	GPIOH_PU_EN			0x0098
#define	GPIOH_PD_EN			0x009c
#define	GPIOH_IN_EN			0x00a0
#define	GPIOH_IN_INV_EN			0x00a4
#define	GPIOH_IN_FLTR_EN		0x00a8
#define	GPIOH_IN_EVNTCNT_EN		0x00ac
#define	GPIOH_READ_BACK			0x00b0
#define	GPIOH_IN_AUX1_SEL		0x00b4
#define	GPIOH_EVNT_EN			0x00b8
#define	GPIOH_LOCK_EN			0x00bc
#define	GPIOH_POSEDGE_EN		0x00c0
#define	GPIOH_NEGEDGE_EN		0x00c4
#define	GPIOH_POSEDGE_STS		0x00c8
#define	GPIOH_NEGEDGE_STS		0x00cc
#define	GPIO_FLTR6_AMNT			0x00d0
#define	GPIO_FLTR6_CNT			0x00d2
#define	GPIO_EVNTCNT6			0x00d4
#define	GPIO_EVNTCNT6_COMP		0x00d6
#define	GPIO_FLTR7_AMNT			0x00d8
#define	GPIO_FLTR7_CNT			0x00da
#define	GPIO_EVNTCNT7			0x00dc
#define	GPIO_EVNTCNT7_COMP		0x00de
#define	GPIO_MAP_X			0x00e0
#define	GPIO_MAP_Y			0x00e4
#define	GPIO_MAP_Z			0x00e8
#define	GPIO_MAP_W			0x00ec
#define	GPIO_FE0_SEL			0x00f0
#define	GPIO_FE1_SEL			0x00f1
#define	GPIO_FE2_SEL			0x00f2
#define	GPIO_FE3_SEL			0x00f3
#define	GPIO_FE4_SEL			0x00f4
#define	GPIO_FE5_SEL			0x00f5
#define	GPIO_FE6_SEL			0x00f6
#define	GPIO_FE7_SEL			0x00f7
#define	GPIOL_EVNTCNT_DEC		0x00f8
#define	GPIOH_EVNTCNT_DEC		0x00fc

#define	GPIO_ATOMIC_VALUE(pin,feature) \
	((feature) ? \
	    ((0 << (16 + (pin))) | (1 << (pin))) : \
	    ((1 << (16 + (pin))) | (0 << (pin))))
