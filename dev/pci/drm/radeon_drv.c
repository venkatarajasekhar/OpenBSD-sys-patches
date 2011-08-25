/* $OpenBSD: radeon_drv.c,v 1.56 2011/08/22 23:12:09 haesbaert Exp $ */
/* radeon_drv.c -- ATI Radeon driver -*- linux-c -*-
 * Created: Wed Feb 14 17:10:04 2001 by gareth@valinux.com
 */
/*-
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Gareth Hughes <gareth@valinux.com>
 *
 */

#include "drmP.h"
#include "drm.h"
#include "radeon_drm.h"
#include "radeon_drv.h"

int	radeondrm_probe(struct device *, void *, void *);
void	radeondrm_attach(struct device *, struct device *, void *);
int	radeondrm_detach(struct device *, int);
int	radeondrm_activate(struct device *, int);
int	radeondrm_ioctl(struct drm_device *, u_long, caddr_t, struct drm_file *);

int radeon_no_wb;

const static struct drm_pcidev radeondrm_pciidlist[] = {
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M241P,
	    CHIP_RV380|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X300M24,
	    CHIP_RV380|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M24GL,
	    CHIP_RV380|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X600_RV380,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V3200,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP320,
	    CHIP_RS100|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP340,
	    CHIP_RS200|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9500PRO, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AE9700PRO, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AF9600TX, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_AGZ1, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AH_9800SE, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AI_9800, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AJ_9800, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_AKX2, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9600PRO, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9600LE, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9600XT, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9550, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_ATT2, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9650, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_AVT2, CHIP_RV350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP_RS250,
	    CHIP_RS200|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R200_BB, CHIP_R200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R200_BC, CHIP_R200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP320M,
	    CHIP_RS100|RADEON_IS_IGP|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_M6,
	    CHIP_RS200|RADEON_IS_IGP|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_MIGP_RS250,
	    CHIP_RS200|RADEON_IS_IGP|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV250, CHIP_RV250},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IG9000, CHIP_RV250},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_JHX800,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800PRO,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800SE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XT,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_X3256,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_M18,
	    CHIP_R420|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_JOX800SE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XTPE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AIW_X800VE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850XT,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850SE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850PRO,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850XTPE,
	    CHIP_R420|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M7LW,
	    CHIP_RV200|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M7,
	    CHIP_RV200|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M6LY,
	    CHIP_RV100|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M6LZ,
	    CHIP_RV100|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M9LD,
	    CHIP_RV250|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M9Lf,
	    CHIP_RV250},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M9Lg,
	    CHIP_RV250|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R300, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON9500_PRO, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON9600TX, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_X1, CHIP_R300},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R350, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON9800, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9800XT, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_X2, CHIP_R350},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV350,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV350NQ,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV350NR,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV350NS,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV350_WS,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_9550,
	    CHIP_RV350|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_AIW,
	    CHIP_R100|RADEON_SINGLE_CRTC},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QE,
	    CHIP_R100|RADEON_SINGLE_CRTC},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QF,
	    CHIP_R100|RADEON_SINGLE_CRTC},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QG,
	    CHIP_R100|RADEON_SINGLE_CRTC},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QH, CHIP_R200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R200_QL, CHIP_R200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_R200_QM, CHIP_R200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV200_QW, CHIP_RV200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV200_QX, CHIP_RV200},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QY, CHIP_RV100},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_QZ, CHIP_RV100},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_ES1000_1, CHIP_RV100},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M300_M22,
	    CHIP_RV380|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X600_M24C,
	    CHIP_RV380|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M44,
	    CHIP_RV380|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800PRORV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XT_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800SE_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XTPRV430,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XL_RV430,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800SE_RV430,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800_RV430,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V7100_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5100_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_UR_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_UT_RV423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5000_M26,
	    CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5000_M26b,
	    CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700XL_M26,
	    CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700_M26_1,
	    CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700_M26_2,
	    CHIP_RV410|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X550XTX,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP9100_IGP,
	    CHIP_RS300|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_IGP9100,
	    CHIP_RS300|RADEON_IS_IGP|RADEON_IS_MOBILITY},
#if 0
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS480,
	    CHIP_RS480|RADEON_IS_IGP|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS480_B,
	    CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS482,
	    CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS482_B,
	    CHIP_RS480|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART},
#endif
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RV280_PRO, CHIP_RV280},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RV280, CHIP_RV280},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RV280_B, CHIP_RV280},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RV280_SE_S, CHIP_RV280},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREMV_2200, CHIP_RV280},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_ES1000, CHIP_RV100},
#if 0
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS400,
	    CHIP_RS400|RADEON_IS_IGP|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS400_B,
	    CHIP_RS400|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RC410,
	    CHIP_RS400|RADEON_IS_IGP|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RC410_B,
	    CHIP_RS400|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_IS_IGPGART},
#endif
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X300,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X600_RV370,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X550,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_RV370,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREMV_2200_5B65,
	    CHIP_RV380|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RV280_M,
	    CHIP_RV280|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_M9PLUS,
	    CHIP_RV280|RADEON_IS_MOBILITY},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XT_M28,
	    CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5100_M28,
	    CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X800_M28,
	    CHIP_R423|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850_R480,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850XTPER480,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850SE_R480,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800_GTO,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_R480,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X850XT_R480,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X800XT_R423,
	    CHIP_R423|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5000_R410,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700XT_R410,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700PRO_R410,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700SE_R410,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700_PCIE,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X700SE_PCIE,
	    CHIP_RV410|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800A,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800XT,
	    CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1800,
	    CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M_V7200,
	    CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M_V7200,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5300,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_M_V7100,
	    CHIP_R520|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800B,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800C,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800D,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800E,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1800F,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V7300,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V7350,
	    CHIP_R520|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV505_1,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_X1550,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_M54_GL,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1400,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550_X1300,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550_64,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_M52,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1300_4A,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1300_4B,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1300_4C,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_4D,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_4E,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV505_2,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV505_3,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V3300,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V3350,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_5E,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550_64_2,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300X1550,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_81,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300PRO,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1450,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X2300,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X2300_2,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1350,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1350_2,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1450,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_8F,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550_2,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1350_3,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREMV_2250,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1550_64_3,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_C0,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1650,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_PRO,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_C3,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5200,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_M,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1650_PRO,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1650_PRO2,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1600_CD,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1300_XT,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V3400,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV530_M56,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1700,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1700XT,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V5200_1,
	    CHIP_RV530|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1700,
	    CHIP_RV530|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X2300HD,
	    CHIP_RV515|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X2300HD,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X2300HD_1,
	    CHIP_RV515|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1950_40,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_43,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1950_44,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_45,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_46,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_47,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_48,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_49,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_4A,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_4B,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_4C,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_4D,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_STREAM_PROCESSOR,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1900_4F,
	    CHIP_R580|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1950_PRO,
	    CHIP_RV570|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV560,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV560_1,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_MOBILITY_X1900,
	    CHIP_R580|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV560_2,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1950GT,
	    CHIP_RV570|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV570,
	    CHIP_RV570|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV570_2,
	    CHIP_RV570|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_FIREGL_V7400,
	    CHIP_RV570|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV560_3,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RX1650_XT,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1650_1,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RV560_4,
	    CHIP_RV560|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_9000IGP,
	    CHIP_RS300|RADEON_IS_IGP|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_RS350IGP,
	    CHIP_RS300|RADEON_IS_IGP|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
#if 0
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1250,
	    CHIP_RS690|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_X1250IGP,
	    CHIP_RS690|RADEON_IS_IGP|RADEON_NEW_MEMMAP|RADEON_IS_IGPGART},
#endif
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD2400_XT,
	    CHIP_RV610|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3400_M82,
	    CHIP_RV620|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3450,
	    CHIP_RV620|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3470,
	    CHIP_RV620|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD2600_PRO,
	    CHIP_RV630|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD2600_XT,
	    CHIP_RV630|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3650_M,
	    CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3650,
	    CHIP_RV635|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3670_M,
	    CHIP_RV635|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3850,
	    CHIP_RV670|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3000,
	    CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3200_1,
	    CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3200_2,
	    CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD3300,
	    CHIP_RS780|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4350,
	    CHIP_RV710|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4500_M,
	    CHIP_RV710|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4650,
	    CHIP_RV730|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4670,
	    CHIP_RV730|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4850,
	    CHIP_RV770|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4870,
	    CHIP_RV770|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4870_M98,
	    CHIP_RV770|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4890,
	    CHIP_RV770|RADEON_NEW_MEMMAP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4200,
	    CHIP_RS880|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4250,
	    CHIP_RS880|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD4200_M,
	    CHIP_RS880|RADEON_IS_MOBILITY|RADEON_NEW_MEMMAP|RADEON_IS_IGP},
	{PCI_VENDOR_ATI, PCI_PRODUCT_ATI_RADEON_HD5450,
	    CHIP_RS880|RADEON_NEW_MEMMAP},
        {0, 0, 0}
};

static const struct drm_driver_info radeondrm_driver = {
	.buf_priv_size		= sizeof(drm_radeon_buf_priv_t),
	.file_priv_size		= sizeof(struct drm_radeon_file),
	.firstopen		= radeon_driver_firstopen,
	.open			= radeon_driver_open,
	.ioctl			= radeondrm_ioctl,
	.close			= radeon_driver_close,
	.lastclose		= radeon_driver_lastclose,
	.vblank_pipes		= 2,
	.get_vblank_counter	= radeon_get_vblank_counter,
	.enable_vblank		= radeon_enable_vblank,
	.disable_vblank		= radeon_disable_vblank,
	.irq_install		= radeon_driver_irq_install,
	.irq_uninstall		= radeon_driver_irq_uninstall,
	.dma_ioctl		= radeon_cp_buffers,

	.name			= DRIVER_NAME,
	.desc			= DRIVER_DESC,
	.date			= DRIVER_DATE,
	.major			= DRIVER_MAJOR,
	.minor			= DRIVER_MINOR,
	.patchlevel		= DRIVER_PATCHLEVEL,

	.flags			= DRIVER_AGP | DRIVER_MTRR | DRIVER_SG |
				    DRIVER_DMA | DRIVER_IRQ,
};

int
radeondrm_probe(struct device *parent, void *match, void *aux)
{
	return drm_pciprobe((struct pci_attach_args *)aux, radeondrm_pciidlist);
}

void
radeondrm_attach(struct device *parent, struct device *self, void *aux)
{
	drm_radeon_private_t	*dev_priv = (drm_radeon_private_t *)self;
	struct pci_attach_args	*pa = aux;
	struct vga_pci_bar	*bar;
	const struct drm_pcidev	*id_entry;
	int			 is_agp;

	id_entry = drm_find_description(PCI_VENDOR(pa->pa_id),
	    PCI_PRODUCT(pa->pa_id), radeondrm_pciidlist);
	dev_priv->flags = id_entry->driver_private;
	dev_priv->pc = pa->pa_pc;
	dev_priv->bst = pa->pa_memt;

	bar = vga_pci_bar_info((struct vga_pci_softc *)parent, 0);
	if (bar == NULL) {
		printf(": can't get frambuffer info\n");
		return;
	}
	dev_priv->fb_aper_offset = bar->base;
	dev_priv->fb_aper_size = bar->maxsize;

	bar = vga_pci_bar_info((struct vga_pci_softc *)parent, 2);
	if (bar == NULL) {
		printf(": can't get BAR info\n");
		return;
	}

	dev_priv->regs = vga_pci_bar_map((struct vga_pci_softc *)parent, 
	    bar->addr, 0, 0);
	if (dev_priv->regs == NULL) {
		printf(": can't map mmio space\n");
		return;
	}

	if (pci_intr_map(pa, &dev_priv->ih) != 0) {
		printf(": couldn't map interrupt\n");
		return;
	}
	printf(": %s\n", pci_intr_string(pa->pa_pc, dev_priv->ih));
	mtx_init(&dev_priv->swi_lock, IPL_TTY);

	switch (dev_priv->flags & RADEON_FAMILY_MASK) {
	case CHIP_R100:
	case CHIP_RV200:
	case CHIP_R200:
	case CHIP_R300:
	case CHIP_R350:
	case CHIP_R420:
	case CHIP_R423:
	case CHIP_RV410:
	case CHIP_RV515:
	case CHIP_R520:
	case CHIP_RV570:
	case CHIP_R580:
		dev_priv->flags |= RADEON_HAS_HIERZ;
		break;
	default:
		/* all other chips have no hierarchical z buffer */
		break;
	}

	dev_priv->chip_family = dev_priv->flags & RADEON_FAMILY_MASK;
	if (pci_get_capability(pa->pa_pc, pa->pa_tag, PCI_CAP_AGP, NULL, NULL))
		dev_priv->flags |= RADEON_IS_AGP;
	else if (pci_get_capability(pa->pa_pc, pa->pa_tag,
	    PCI_CAP_PCIEXPRESS, NULL, NULL))
		dev_priv->flags |= RADEON_IS_PCIE;
	else
		dev_priv->flags |= RADEON_IS_PCI;

	DRM_DEBUG("%s card detected\n",
		  ((dev_priv->flags & RADEON_IS_AGP) ? "AGP" :
		  (((dev_priv->flags & RADEON_IS_PCIE) ? "PCIE" : "PCI"))));

	is_agp = pci_get_capability(pa->pa_pc, pa->pa_tag, PCI_CAP_AGP,
	    NULL, NULL);

	TAILQ_INIT(&dev_priv->gart_heap);
	TAILQ_INIT(&dev_priv->fb_heap);

	dev_priv->drmdev = drm_attach_pci(&radeondrm_driver, pa, is_agp, self);
}

int
radeondrm_detach(struct device *self, int flags)
{
	drm_radeon_private_t *dev_priv = (drm_radeon_private_t *)self;

	DRM_DEBUG("\n");

	if (dev_priv->drmdev != NULL) {
		config_detach(dev_priv->drmdev, flags);
		dev_priv->drmdev = NULL;
	}

	if (dev_priv->regs != NULL)
		vga_pci_bar_unmap(dev_priv->regs);

	return (0);
}

int
radeondrm_activate(struct device *arg, int act)
{
	struct drm_radeon_private *dev_priv = (struct drm_radeon_private *)arg;
	struct drm_device	*dev = (struct drm_device *)dev_priv->drmdev;

	switch (act) {
	case DVACT_SUSPEND:
		/* Interrupts still not supported on r600 */
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600 ||
		    dev->irq_enabled == 0)
			break;
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_RS690)
			RADEON_WRITE(R500_DxMODE_INT_MASK, 0);
		RADEON_WRITE(RADEON_GEN_INT_CNTL, 0);
		break;
	case DVACT_RESUME:
		/* Interrupts still not supported on r600 */
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600 ||
		    dev->irq_enabled == 0)
			break;
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_RS690)
			RADEON_WRITE(R500_DxMODE_INT_MASK,
			    dev_priv->r500_disp_irq_reg);
		RADEON_WRITE(RADEON_GEN_INT_CNTL, dev_priv->irq_enable_reg);
		break;
	}

	return (0);
}

struct cfattach radeondrm_ca = {
        sizeof (drm_radeon_private_t), radeondrm_probe, radeondrm_attach, 
	radeondrm_detach, radeondrm_activate
}; 

struct cfdriver radeondrm_cd = {
	NULL, "radeondrm", DV_DULL
}; 

int
radeondrm_ioctl(struct drm_device *dev, u_long cmd, caddr_t data,
    struct drm_file *file_priv)
{
	if (file_priv->authenticated == 1) {
		switch (cmd) {
		case DRM_IOCTL_RADEON_CP_IDLE:
			return (radeon_cp_idle(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CP_RESUME:
			return (radeon_cp_resume(dev));
		case DRM_IOCTL_RADEON_SWAP:
			return (radeon_cp_swap(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CLEAR:
			return (radeon_cp_clear(dev, data, file_priv));
		case DRM_IOCTL_RADEON_TEXTURE:
			return (radeon_cp_texture(dev, data, file_priv));
		case DRM_IOCTL_RADEON_STIPPLE:
			return (radeon_cp_stipple(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CMDBUF:
			return (radeon_cp_cmdbuf(dev, data, file_priv));
		case DRM_IOCTL_RADEON_GETPARAM:
			return (radeon_cp_getparam(dev, data, file_priv));
		case DRM_IOCTL_RADEON_FLIP:
			return (radeon_cp_flip(dev, data, file_priv));
		case DRM_IOCTL_RADEON_ALLOC:
			return (radeon_mem_alloc(dev, data, file_priv));
		case DRM_IOCTL_RADEON_FREE:
			return (radeon_mem_free(dev, data, file_priv));
		case DRM_IOCTL_RADEON_IRQ_EMIT:
			return (radeon_irq_emit(dev, data, file_priv));
		case DRM_IOCTL_RADEON_IRQ_WAIT:
			return (radeon_irq_wait(dev, data, file_priv));
		case DRM_IOCTL_RADEON_SETPARAM:
			return (radeon_cp_setparam(dev, data, file_priv));
		case DRM_IOCTL_RADEON_SURF_ALLOC:
			return (radeon_surface_alloc(dev, data, file_priv));
		case DRM_IOCTL_RADEON_SURF_FREE:
			return (radeon_surface_free(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CS:
			return (radeon_cs_ioctl(dev, data, file_priv));
		}
	}

	if (file_priv->master == 1) {
		switch (cmd) {
		case DRM_IOCTL_RADEON_CP_INIT:
			return (radeon_cp_init(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CP_START:
			return (radeon_cp_start(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CP_STOP:
			return (radeon_cp_stop(dev, data, file_priv));
		case DRM_IOCTL_RADEON_CP_RESET:
			return (radeon_cp_reset(dev, data, file_priv));
		case DRM_IOCTL_RADEON_INDIRECT:
			return (radeon_cp_indirect(dev, data, file_priv));
		case DRM_IOCTL_RADEON_INIT_HEAP:
			return (radeon_mem_init_heap(dev, data, file_priv));
		}
	}
	return (EINVAL);
}

u_int32_t
radeondrm_read_rptr(struct drm_radeon_private *dev_priv, u_int32_t off)
{
	u_int32_t val;

	if (dev_priv->flags & RADEON_IS_AGP) {
		val = bus_space_read_4(dev_priv->ring_rptr->bst,
		    dev_priv->ring_rptr->bsh, off);
	} else {
		val = *(((volatile u_int32_t *)dev_priv->ring_rptr->handle) +
		    (off / sizeof(u_int32_t)));
		val = letoh32(val);
	}
	return (val);
}

void
radeondrm_write_rptr(struct drm_radeon_private *dev_priv, u_int32_t off,
    u_int32_t val)
{
	if (dev_priv->flags & RADEON_IS_AGP) {
		bus_space_write_4(dev_priv->ring_rptr->bst,
		    dev_priv->ring_rptr->bsh, off, val);
	} else
		*(((volatile u_int32_t *)dev_priv->ring_rptr->handle +
		    (off / sizeof(u_int32_t)))) = htole32(val);
}

u_int32_t
radeondrm_get_ring_head(struct drm_radeon_private *dev_priv)
{
	if (dev_priv->writeback_works) {
		return (radeondrm_read_rptr(dev_priv, 0));
	} else {
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600)
			return (RADEON_READ(R600_CP_RB_RPTR));
		else
			return (RADEON_READ(RADEON_CP_RB_RPTR));
	}
}

void
radeondrm_set_ring_head(struct drm_radeon_private *dev_priv, u_int32_t val)
{
	radeondrm_write_rptr(dev_priv, 0, val);
}

u_int32_t
radeondrm_get_scratch(struct drm_radeon_private *dev_priv, u_int32_t off)
{
	if (dev_priv->writeback_works)
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600)
			return radeondrm_read_rptr(dev_priv,
			    R600_SCRATCHOFF(off));
		else
			return radeondrm_read_rptr(dev_priv,
			    RADEON_SCRATCHOFF(off));
	else
		if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600)
			return RADEON_READ(R600_SCRATCH_REG0 + 4 * off);
		else
			return RADEON_READ(RADEON_SCRATCH_REG0 + 4 * off);
}

void
radeondrm_begin_ring(struct drm_radeon_private *dev_priv, int ncmd)
{
	int align_nr;
	RADEON_VPRINTF("%d\n", ncmd);

	align_nr = RADEON_RING_ALIGN - ((dev_priv->ring.tail + ncmd) &
		(RADEON_RING_ALIGN - 1));
	align_nr += ncmd;

	if (dev_priv->ring.space <= align_nr) {
		radeondrm_commit_ring(dev_priv);
		radeon_wait_ring(dev_priv, ncmd);
	}
	dev_priv->ring.space -= ncmd;
	dev_priv->ring.wspace = ncmd;
	dev_priv->ring.woffset = dev_priv->ring.tail;
}

void
radeondrm_advance_ring(struct drm_radeon_private *dev_priv)
{
	RADEON_VPRINTF("wr=0x%06x, tail = 0x%06x\n", dev_priv->ring.woffset,
	    dev_priv->ring.tail);
	if (((dev_priv->ring.tail + dev_priv->ring.wspace) &
	    dev_priv->ring.tail_mask) != dev_priv->ring.woffset) {
		DRM_ERROR("mismatch: nr %x, write %x\n", ((dev_priv->ring.tail +
		    dev_priv->ring.wspace) & dev_priv->ring.tail_mask),
		    dev_priv->ring.woffset);
	} else
		dev_priv->ring.tail = dev_priv->ring.woffset;
}

void
radeondrm_commit_ring(struct drm_radeon_private *dev_priv)
{
	int		 i, tail_aligned, num_p2;
	u_int32_t	*ring;

	/* check if the ring is padded out to 16-dword alignment */

	tail_aligned = dev_priv->ring.tail & (RADEON_RING_ALIGN - 1);
	if (tail_aligned) {
		num_p2 = RADEON_RING_ALIGN - tail_aligned;

		ring = dev_priv->ring.start;
		/* pad with some CP_PACKET2 */
		for (i = 0; i < num_p2; i++)
			ring[dev_priv->ring.tail + i] = CP_PACKET2();

		dev_priv->ring.tail += i;
		dev_priv->ring.space -= num_p2;
		
	}
	dev_priv->ring.tail &= dev_priv->ring.tail_mask;
	/* XXX 128byte aligned stuff */
	/* flush write combining buffer and writes to ring */
	DRM_MEMORYBARRIER();
	radeondrm_get_ring_head(dev_priv);
	if ((dev_priv->flags & RADEON_FAMILY_MASK) >= CHIP_R600) {
		RADEON_WRITE(R600_CP_RB_WPTR, dev_priv->ring.tail);
		/* read from PCI bus to ensure correct posting */
		RADEON_READ(R600_CP_RB_RPTR);
	} else {
		RADEON_WRITE(RADEON_CP_RB_WPTR, dev_priv->ring.tail);
		/* read from PCI bus to ensure correct posting */
		RADEON_READ(RADEON_CP_RB_RPTR);
	}
}

void
radeondrm_out_ring(struct drm_radeon_private *dev_priv, u_int32_t x)
{
	RADEON_VPRINTF("0x%08x at 0x%x\n", x, dev_priv->ring.woffset);
	dev_priv->ring.start[dev_priv->ring.woffset++] = x;
	dev_priv->ring.woffset &= dev_priv->ring.tail_mask;
}

void
radeondrm_out_ring_table(struct drm_radeon_private *dev_priv, u_int32_t *table,
    int size)
{
	if (dev_priv->ring.woffset + size > dev_priv->ring.tail_mask) {
		int i = dev_priv->ring.tail_mask + 1 - dev_priv->ring.woffset;

		size -= i;
		while (i--)
			dev_priv->ring.start[dev_priv->ring.woffset++] =
			    *table++;
		dev_priv->ring.woffset = 0;
	}
	while (size--)
		dev_priv->ring.start[dev_priv->ring.woffset++] = *table++;
	dev_priv->ring.woffset &= dev_priv->ring.tail_mask;
}
