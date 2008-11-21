/*	$NetBSD: vreset.c,v 1.9 2006/01/29 21:42:41 dsl Exp $	*/

/*
 * Copyright (C) 1995-1997 Gary Thomas (gdt@linuxppc.org)
 * All rights reserved.
 *
 * Initialize the VGA control registers to 80x25 text mode.
 *
 * Adapted from a program by:
 *                                      Steve Sellgren
 *                                      San Francisco Indigo Company
 *                                      sfindigo!sellgren@uunet.uu.net
 * Adapted for Moto boxes by:
 *                                      Pat Kane & Mark Scott, 1996
 * Fixed for IBM/PowerStack II          Pat Kane 1997
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Gary Thomas.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef CONS_VGA
#include <lib/libsa/stand.h>
#include <sys/bswap.h>
#include "boot.h"
#include "iso_font.h"

#if 0
static char rcsid[] = "vreset.c 2.0 1997 kane  PEK'97 Exp $";
#endif

/*
 * VGA Register
 */
struct VgaRegs
{
	u_short io_port;
	u_char io_index;
	u_char io_value;
};

/*
 * Default console text mode registers  used to reset
 * graphics adapter.
 */
#define NREGS 54
#define ENDMK  0xFFFF  /* End marker */

#define S3Vendor     	0x5333
#define CirrusVendor 	0x1013
#define DiamondVendor	0x100E
#define MatroxVendor	0x102B

struct VgaRegs GenVgaTextRegs[NREGS+1] = {
/*      port    index   value */
	/* SR Regs */
        { 0x3c4, 0x1, 0x0 },
        { 0x3c4, 0x2, 0x3 },
        { 0x3c4, 0x3, 0x0 },
        { 0x3c4, 0x4, 0x2 },
	/* CR Regs */
        { 0x3d4, 0x0, 0x5f },
        { 0x3d4, 0x1, 0x4f },
        { 0x3d4, 0x2, 0x50 },
        { 0x3d4, 0x3, 0x82 },
        { 0x3d4, 0x4, 0x55 },
        { 0x3d4, 0x5, 0x81 },
        { 0x3d4, 0x6, 0xbf },
        { 0x3d4, 0x7, 0x1f },
        { 0x3d4, 0x8, 0x00 },
        { 0x3d4, 0x9, 0x4f },
        { 0x3d4, 0xa, 0x0d },
        { 0x3d4, 0xb, 0x0e },
        { 0x3d4, 0xc, 0x00 },
        { 0x3d4, 0xd, 0x00 },
        { 0x3d4, 0xe, 0x00 },
        { 0x3d4, 0xf, 0x00 },
        { 0x3d4, 0x10, 0x9c },
        { 0x3d4, 0x11, 0x8e },
        { 0x3d4, 0x12, 0x8f },
        { 0x3d4, 0x13, 0x28 },
        { 0x3d4, 0x14, 0x1f },
        { 0x3d4, 0x15, 0x96 },
        { 0x3d4, 0x16, 0xb9 },
        { 0x3d4, 0x17, 0xa3 },
	/* GR Regs */
        { 0x3ce, 0x0, 0x0 },
        { 0x3ce, 0x1, 0x0 },
        { 0x3ce, 0x2, 0x0 },
        { 0x3ce, 0x3, 0x0 },
        { 0x3ce, 0x4, 0x0 },
        { 0x3ce, 0x5, 0x10 },
        { 0x3ce, 0x6, 0xe },
        { 0x3ce, 0x7, 0x0 },
        { 0x3ce, 0x8, 0xff },
        { ENDMK },
};

struct VgaRegs S3TextRegs[NREGS+1] = {
/*	port	index	value */
	/* SR Regs */
	{ 0x3c4, 0x1, 0x0 },
	{ 0x3c4, 0x2, 0x3 },
	{ 0x3c4, 0x3, 0x0 },
	{ 0x3c4, 0x4, 0x2 },
	/* CR Regs */
	{ 0x3d4, 0x0, 0x5f },
	{ 0x3d4, 0x1, 0x4f },
	{ 0x3d4, 0x2, 0x50 },
	{ 0x3d4, 0x3, 0x82 },
	{ 0x3d4, 0x4, 0x55 },
	{ 0x3d4, 0x5, 0x81 },
	{ 0x3d4, 0x6, 0xbf },
	{ 0x3d4, 0x7, 0x1f },
	{ 0x3d4, 0x8, 0x00 },
	{ 0x3d4, 0x9, 0x4f },
	{ 0x3d4, 0xa, 0x0d },
	{ 0x3d4, 0xb, 0x0e },
	{ 0x3d4, 0xc, 0x00 },
	{ 0x3d4, 0xd, 0x00 },
	{ 0x3d4, 0xe, 0x00 },
	{ 0x3d4, 0xf, 0x00 },
	{ 0x3d4, 0x10, 0x9c },
	{ 0x3d4, 0x11, 0x8e },
	{ 0x3d4, 0x12, 0x8f },
	{ 0x3d4, 0x13, 0x28 },
	{ 0x3d4, 0x14, 0x1f },
	{ 0x3d4, 0x15, 0x96 },
	{ 0x3d4, 0x16, 0xb9 },
	{ 0x3d4, 0x17, 0xa3 },
	/* GR Regs */
	{ 0x3ce, 0x0, 0x0 },
	{ 0x3ce, 0x1, 0x0 },
	{ 0x3ce, 0x2, 0x0 },
	{ 0x3ce, 0x3, 0x0 },
	{ 0x3ce, 0x4, 0x0 },
	{ 0x3ce, 0x5, 0x10 },
	{ 0x3ce, 0x6, 0xe },
	{ 0x3ce, 0x7, 0x0 },
	{ 0x3ce, 0x8, 0xff },
        { ENDMK }
};

struct RGBColors {
	u_char r, g, b;
};

/*
 * Default console text mode color table.
 * These values were obtained by booting Linux with
 * text mode firmware & then dumping the registers.
 */
struct RGBColors TextCLUT[256] = {
/*	red	green	blue */
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x2a },
	{ 0x0,	0x2a,	0x0 },
	{ 0x0,	0x2a,	0x2a },
	{ 0x2a,	0x0,	0x0 },
	{ 0x2a,	0x0,	0x2a },
	{ 0x2a,	0x2a,	0x0 },
	{ 0x2a,	0x2a,	0x2a },
	{ 0x0,	0x0,	0x15 },
	{ 0x0,	0x0,	0x3f },
	{ 0x0,	0x2a,	0x15 },
	{ 0x0,	0x2a,	0x3f },
	{ 0x2a,	0x0,	0x15 },
	{ 0x2a,	0x0,	0x3f },
	{ 0x2a,	0x2a,	0x15 },
	{ 0x2a,	0x2a,	0x3f },
	{ 0x0,	0x15,	0x0 },
	{ 0x0,	0x15,	0x2a },
	{ 0x0,	0x3f,	0x0 },
	{ 0x0,	0x3f,	0x2a },
	{ 0x2a,	0x15,	0x0 },
	{ 0x2a,	0x15,	0x2a },
	{ 0x2a,	0x3f,	0x0 },
	{ 0x2a,	0x3f,	0x2a },
	{ 0x0,	0x15,	0x15 },
	{ 0x0,	0x15,	0x3f },
	{ 0x0,	0x3f,	0x15 },
	{ 0x0,	0x3f,	0x3f },
	{ 0x2a,	0x15,	0x15 },
	{ 0x2a,	0x15,	0x3f },
	{ 0x2a,	0x3f,	0x15 },
	{ 0x2a,	0x3f,	0x3f },
	{ 0x15,	0x0,	0x0 },
	{ 0x15,	0x0,	0x2a },
	{ 0x15,	0x2a,	0x0 },
	{ 0x15,	0x2a,	0x2a },
	{ 0x3f,	0x0,	0x0 },
	{ 0x3f,	0x0,	0x2a },
	{ 0x3f,	0x2a,	0x0 },
	{ 0x3f,	0x2a,	0x2a },
	{ 0x15,	0x0,	0x15 },
	{ 0x15,	0x0,	0x3f },
	{ 0x15,	0x2a,	0x15 },
	{ 0x15,	0x2a,	0x3f },
	{ 0x3f,	0x0,	0x15 },
	{ 0x3f,	0x0,	0x3f },
	{ 0x3f,	0x2a,	0x15 },
	{ 0x3f,	0x2a,	0x3f },
	{ 0x15,	0x15,	0x0 },
	{ 0x15,	0x15,	0x2a },
	{ 0x15,	0x3f,	0x0 },
	{ 0x15,	0x3f,	0x2a },
	{ 0x3f,	0x15,	0x0 },
	{ 0x3f,	0x15,	0x2a },
	{ 0x3f,	0x3f,	0x0 },
	{ 0x3f,	0x3f,	0x2a },
	{ 0x15,	0x15,	0x15 },
	{ 0x15,	0x15,	0x3f },
	{ 0x15,	0x3f,	0x15 },
	{ 0x15,	0x3f,	0x3f },
	{ 0x3f,	0x15,	0x15 },
	{ 0x3f,	0x15,	0x3f },
	{ 0x3f,	0x3f,	0x15 },
	{ 0x3f,	0x3f,	0x3f },
	{ 0x39,	0xc,	0x5 },
	{ 0x15,	0x2c,	0xf },
	{ 0x26,	0x10,	0x3d },
	{ 0x29,	0x29,	0x38 },
	{ 0x4,	0x1a,	0xe },
	{ 0x2,	0x1e,	0x3a },
	{ 0x3c,	0x25,	0x33 },
	{ 0x3c,	0xc,	0x2c },
	{ 0x3f,	0x3,	0x2b },
	{ 0x1c,	0x9,	0x13 },
	{ 0x25,	0x2a,	0x35 },
	{ 0x1e,	0xa,	0x38 },
	{ 0x24,	0x8,	0x3 },
	{ 0x3,	0xe,	0x36 },
	{ 0xc,	0x6,	0x2a },
	{ 0x26,	0x3,	0x32 },
	{ 0x5,	0x2f,	0x33 },
	{ 0x3c,	0x35,	0x2f },
	{ 0x2d,	0x26,	0x3e },
	{ 0xd,	0xa,	0x10 },
	{ 0x25,	0x3c,	0x11 },
	{ 0xd,	0x4,	0x2e },
	{ 0x5,	0x19,	0x3e },
	{ 0xc,	0x13,	0x34 },
	{ 0x2b,	0x6,	0x24 },
	{ 0x4,	0x3,	0xd },
	{ 0x2f,	0x3c,	0xc },
	{ 0x2a,	0x37,	0x1f },
	{ 0xf,	0x12,	0x38 },
	{ 0x38,	0xe,	0x2a },
	{ 0x12,	0x2f,	0x19 },
	{ 0x29,	0x2e,	0x31 },
	{ 0x25,	0x13,	0x3e },
	{ 0x33,	0x3e,	0x33 },
	{ 0x1d,	0x2c,	0x25 },
	{ 0x15,	0x15,	0x5 },
	{ 0x32,	0x25,	0x39 },
	{ 0x1a,	0x7,	0x1f },
	{ 0x13,	0xe,	0x1d },
	{ 0x36,	0x17,	0x34 },
	{ 0xf,	0x15,	0x23 },
	{ 0x2,	0x35,	0xd },
	{ 0x15,	0x3f,	0xc },
	{ 0x14,	0x2f,	0xf },
	{ 0x19,	0x21,	0x3e },
	{ 0x27,	0x11,	0x2f },
	{ 0x38,	0x3f,	0x3c },
	{ 0x36,	0x2d,	0x15 },
	{ 0x16,	0x17,	0x2 },
	{ 0x1,	0xa,	0x3d },
	{ 0x1b,	0x11,	0x3f },
	{ 0x21,	0x3c,	0xd },
	{ 0x1a,	0x39,	0x3d },
	{ 0x8,	0xe,	0xe },
	{ 0x22,	0x21,	0x23 },
	{ 0x1e,	0x30,	0x5 },
	{ 0x1f,	0x22,	0x3d },
	{ 0x1e,	0x2f,	0xa },
	{ 0x0,	0x1c,	0xe },
	{ 0x0,	0x1c,	0x15 },
	{ 0x0,	0x1c,	0x1c },
	{ 0x0,	0x15,	0x1c },
	{ 0x0,	0xe,	0x1c },
	{ 0x0,	0x7,	0x1c },
	{ 0xe,	0xe,	0x1c },
	{ 0x11,	0xe,	0x1c },
	{ 0x15,	0xe,	0x1c },
	{ 0x18,	0xe,	0x1c },
	{ 0x1c,	0xe,	0x1c },
	{ 0x1c,	0xe,	0x18 },
	{ 0x1c,	0xe,	0x15 },
	{ 0x1c,	0xe,	0x11 },
	{ 0x1c,	0xe,	0xe },
	{ 0x1c,	0x11,	0xe },
	{ 0x1c,	0x15,	0xe },
	{ 0x1c,	0x18,	0xe },
	{ 0x1c,	0x1c,	0xe },
	{ 0x18,	0x1c,	0xe },
	{ 0x15,	0x1c,	0xe },
	{ 0x11,	0x1c,	0xe },
	{ 0xe,	0x1c,	0xe },
	{ 0xe,	0x1c,	0x11 },
	{ 0xe,	0x1c,	0x15 },
	{ 0xe,	0x1c,	0x18 },
	{ 0xe,	0x1c,	0x1c },
	{ 0xe,	0x18,	0x1c },
	{ 0xe,	0x15,	0x1c },
	{ 0xe,	0x11,	0x1c },
	{ 0x14,	0x14,	0x1c },
	{ 0x16,	0x14,	0x1c },
	{ 0x18,	0x14,	0x1c },
	{ 0x1a,	0x14,	0x1c },
	{ 0x1c,	0x14,	0x1c },
	{ 0x1c,	0x14,	0x1a },
	{ 0x1c,	0x14,	0x18 },
	{ 0x1c,	0x14,	0x16 },
	{ 0x1c,	0x14,	0x14 },
	{ 0x1c,	0x16,	0x14 },
	{ 0x1c,	0x18,	0x14 },
	{ 0x1c,	0x1a,	0x14 },
	{ 0x1c,	0x1c,	0x14 },
	{ 0x1a,	0x1c,	0x14 },
	{ 0x18,	0x1c,	0x14 },
	{ 0x16,	0x1c,	0x14 },
	{ 0x14,	0x1c,	0x14 },
	{ 0x14,	0x1c,	0x16 },
	{ 0x14,	0x1c,	0x18 },
	{ 0x14,	0x1c,	0x1a },
	{ 0x14,	0x1c,	0x1c },
	{ 0x14,	0x1a,	0x1c },
	{ 0x14,	0x18,	0x1c },
	{ 0x14,	0x16,	0x1c },
	{ 0x0,	0x0,	0x10 },
	{ 0x4,	0x0,	0x10 },
	{ 0x8,	0x0,	0x10 },
	{ 0xc,	0x0,	0x10 },
	{ 0x10,	0x0,	0x10 },
	{ 0x10,	0x0,	0xc },
	{ 0x10,	0x0,	0x8 },
	{ 0x10,	0x0,	0x4 },
	{ 0x10,	0x0,	0x0 },
	{ 0x10,	0x4,	0x0 },
	{ 0x10,	0x8,	0x0 },
	{ 0x10,	0xc,	0x0 },
	{ 0x10,	0x10,	0x0 },
	{ 0xc,	0x10,	0x0 },
	{ 0x8,	0x10,	0x0 },
	{ 0x4,	0x10,	0x0 },
	{ 0x0,	0x10,	0x0 },
	{ 0x0,	0x10,	0x4 },
	{ 0x0,	0x10,	0x8 },
	{ 0x0,	0x10,	0xc },
	{ 0x0,	0x10,	0x10 },
	{ 0x0,	0xc,	0x10 },
	{ 0x0,	0x8,	0x10 },
	{ 0x0,	0x4,	0x10 },
	{ 0x8,	0x8,	0x10 },
	{ 0xa,	0x8,	0x10 },
	{ 0xc,	0x8,	0x10 },
	{ 0xe,	0x8,	0x10 },
	{ 0x10,	0x8,	0x10 },
	{ 0x10,	0x8,	0xe },
	{ 0x10,	0x8,	0xc },
	{ 0x10,	0x8,	0xa },
	{ 0x10,	0x8,	0x8 },
	{ 0x10,	0xa,	0x8 },
	{ 0x10,	0xc,	0x8 },
	{ 0x10,	0xe,	0x8 },
	{ 0x10,	0x10,	0x8 },
	{ 0xe,	0x10,	0x8 },
	{ 0xc,	0x10,	0x8 },
	{ 0xa,	0x10,	0x8 },
	{ 0x8,	0x10,	0x8 },
	{ 0x8,	0x10,	0xa },
	{ 0x8,	0x10,	0xc },
	{ 0x8,	0x10,	0xe },
	{ 0x8,	0x10,	0x10 },
	{ 0x8,	0xe,	0x10 },
	{ 0x8,	0xc,	0x10 },
	{ 0x8,	0xa,	0x10 },
	{ 0xb,	0xb,	0x10 },
	{ 0xc,	0xb,	0x10 },
	{ 0xd,	0xb,	0x10 },
	{ 0xf,	0xb,	0x10 },
	{ 0x10,	0xb,	0x10 },
	{ 0x10,	0xb,	0xf },
	{ 0x10,	0xb,	0xd },
	{ 0x10,	0xb,	0xc },
	{ 0x10,	0xb,	0xb },
	{ 0x10,	0xc,	0xb },
	{ 0x10,	0xd,	0xb },
	{ 0x10,	0xf,	0xb },
	{ 0x10,	0x10,	0xb },
	{ 0xf,	0x10,	0xb },
	{ 0xd,	0x10,	0xb },
	{ 0xc,	0x10,	0xb },
	{ 0xb,	0x10,	0xb },
	{ 0xb,	0x10,	0xc },
	{ 0xb,	0x10,	0xd },
	{ 0xb,	0x10,	0xf },
	{ 0xb,	0x10,	0x10 },
	{ 0xb,	0xf,	0x10 },
	{ 0xb,	0xd,	0x10 },
	{ 0xb,	0xc,	0x10 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
	{ 0x0,	0x0,	0x0 },
};

u_char AC[21] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00
};

void enablePCIvideo(int);
static int scanPCI(void);
static int PCIVendor(int);
int delayLoop(int);
void setTextRegs(struct VgaRegs *);
void setTextCLUT(void);
void loadFont(u_char *);
void unlockS3(void);
#ifdef DEBUG
static void printslots(void);
#endif

static inline void
outw(int port, u_short val)
{
	outb(port, val >> 8);
	outb(port+1, val);
}

void
vga_reset(u_char *ISA_mem)
{
	int slot;
        struct VgaRegs *VgaTextRegs;

	/* See if VGA already in TEXT mode - exit if so! */
	outb(0x3CE, 0x06);
	if ((inb(0x3CF) & 0x01) == 0)
		return;

	/* If no VGA responding in text mode, then we have some work to do... */
	slot = scanPCI();            	/* find video card in use  */
	enablePCIvideo(slot);          	/* enable I/O to card      */

	/*
         * Note: the PCI scanning code does not yet work correctly
         *       for non-Moto boxes, so the switch below only
         *       defaults to using an S3 card if it does not
         *       find a Cirrus card.
         *
         *       The only reason we need to scan the bus looking for
         *       a graphics card is so we could do the "enablePCIvideo(slot)"
         *       call above; it is needed because Moto's OpenFirmware
         *       disables I/O to the graphics adapter before it gives
         *       us control.                                       PEK'97
         */

	switch (PCIVendor(slot)) {
	default:			       /* Assume S3 */
#if 0
	case S3Vendor:
#endif
		unlockS3();
		VgaTextRegs = S3TextRegs;
		outw(0x3C4, 0x0120);           /* disable video              */
		setTextRegs(VgaTextRegs);      /* initial register setup     */
		setTextCLUT();                 /* load color lookup table    */
		loadFont(ISA_mem);             /* load font                  */
		setTextRegs(VgaTextRegs);      /* reload registers           */
		outw(0x3C4, 0x0100);           /* re-enable video            */
		outb(0x3c2, 0x63);  	       /* MISC */
		outb(0x3c2, 0x67);  	       /* MISC */
		break;

	case CirrusVendor:
		VgaTextRegs = GenVgaTextRegs;
		outw(0x3C4, 0x0612);	       /* unlock ext regs            */
		outw(0x3C4, 0x0700);	       /* reset ext sequence mode    */
		outw(0x3C4, 0x0120);           /* disable video              */
		setTextRegs(VgaTextRegs);      /* initial register setup     */
		setTextCLUT();                 /* load color lookup table    */
		loadFont(ISA_mem);             /* load font                  */
		setTextRegs(VgaTextRegs);      /* reload registers           */
		outw(0x3C4, 0x0100);           /* re-enable video            */
		outb(0x3c2, 0x63);  	       /* MISC */
		break;

        case DiamondVendor:
        case MatroxVendor:
	  /*
           * The following code is almost enuf to get the Matrox
           * working (on a Moto box) but the video is not stable.
           * We probably need to tweak the TVP3026 Video PLL regs.   PEK'97
           */
		VgaTextRegs = GenVgaTextRegs;
		outw(0x3C4, 0x0120);           /* disable video              */
		setTextRegs(VgaTextRegs);      /* initial register setup     */
		setTextCLUT();                 /* load color lookup table    */
		loadFont(ISA_mem);             /* load font                  */
		setTextRegs(VgaTextRegs);      /* reload registers           */
		outw(0x3C4, 0x0100);           /* re-enable video            */
		outb(0x3c2, 0x63);  	       /* MISC */
		printf("VGA Chip Vendor ID: 0x%08x\n", PCIVendor(slot));
		delayLoop(1);
		break;
	};

#ifdef DEBUG
	printslots();
	delayLoop(5);
#endif
	delayLoop(2);		/* give time for the video monitor to come up */
}

/*
 * Write to VGA Attribute registers.
 */
void
writeAttr(u_char index, u_char data, u_char videoOn)
{
	u_char v;
	v = inb(0x3da);   /* reset attr. address toggle */
	if (videoOn)
		outb(0x3c0, (index & 0x1F) | 0x20);
	else
		outb(0x3c0, (index & 0x1F));
	outb(0x3c0, data);
}

void
setTextRegs(struct VgaRegs *svp)
{
	int i;

	/*
	 *  saved settings
	 */
	while (svp->io_port != ENDMK) {
		outb(svp->io_port,   svp->io_index);
		outb(svp->io_port+1, svp->io_value);
		svp++;
	}

	outb(0x3c2, 0x67);  /* MISC */
	outb(0x3c6, 0xff);  /* MASK */

	for (i = 0; i < 0x10; i++)
		writeAttr(i, AC[i], 0);	/* pallete */
	writeAttr(0x10, 0x0c, 0);	/* text mode */
	writeAttr(0x11, 0x00, 0);	/* overscan color (border) */
	writeAttr(0x12, 0x0f, 0);	/* plane enable */
	writeAttr(0x13, 0x08, 0);	/* pixel panning */
	writeAttr(0x14, 0x00, 1);	/* color select; video on */
}

void
setTextCLUT(void)
{
	int i;

	outb(0x3C6, 0xFF);
	i = inb(0x3C7);
	outb(0x3C8, 0);
	i = inb(0x3C7);

	for (i = 0; i < 256; i++) {
		outb(0x3C9, TextCLUT[i].r);
		outb(0x3C9, TextCLUT[i].g);
		outb(0x3C9, TextCLUT[i].b);
	}
}

void
loadFont(u_char *ISA_mem)
{
	int i, j;
	u_char *font_page = (u_char *)&ISA_mem[0xA0000];

	outb(0x3C2, 0x67);
	/*
	 * Load font
	 */
	i = inb(0x3DA);		/* Reset Attr toggle */

	outb(0x3C0,0x30);
	outb(0x3C0, 0x01);	/* graphics mode */

	outw(0x3C4, 0x0001);	/* reset sequencer */
	outw(0x3C4, 0x0204);	/* write to plane 2 */
	outw(0x3C4, 0x0406);	/* enable plane graphics */
	outw(0x3C4, 0x0003);	/* reset sequencer */
	outw(0x3CE, 0x0402);	/* read plane 2 */
	outw(0x3CE, 0x0500);	/* write mode 0, read mode 0 */
	outw(0x3CE, 0x0605);	/* set graphics mode */

	for (i = 0;  i < sizeof(font);  i += 16) {
		for (j = 0;  j < 16;  j++) {
			__asm volatile("eieio");
			font_page[(2*i)+j] = font[i+j];
		}
	}
}

void
unlockS3(void)
{
	/* From the S3 manual */
	outb(0x46E8, 0x10);  /* Put into setup mode */
	outb(0x3C3, 0x10);
	outb(0x102, 0x01);   /* Enable registers */
	outb(0x46E8, 0x08);  /* Enable video */
	outb(0x3C3, 0x08);
	outb(0x4AE8, 0x00);

	outb(0x42E8, 0x80);  /* Reset graphics engine? */

	outb(0x3D4, 0x38);  /* Unlock all registers */
	outb(0x3D5, 0x48);
	outb(0x3D4, 0x39);
	outb(0x3D5, 0xA5);
	outb(0x3D4, 0x40);
	outb(0x3D5, inb(0x3D5)|0x01);
	outb(0x3D4, 0x33);
	outb(0x3D5, inb(0x3D5)&~0x52);
	outb(0x3D4, 0x35);
	outb(0x3D5, inb(0x3D5)&~0x30);
	outb(0x3D4, 0x3A);
	outb(0x3D5, 0x00);
	outb(0x3D4, 0x53);
	outb(0x3D5, 0x00);
	outb(0x3D4, 0x31);
	outb(0x3D5, inb(0x3D5)&~0x4B);
	outb(0x3D4, 0x58);

	outb(0x3D5, 0);

	outb(0x3D4, 0x54);
	outb(0x3D5, 0x38);
	outb(0x3D4, 0x60);
	outb(0x3D5, 0x07);
	outb(0x3D4, 0x61);
	outb(0x3D5, 0x80);
	outb(0x3D4, 0x62);
	outb(0x3D5, 0xA1);
	outb(0x3D4, 0x69);  /* High order bits for cursor address */
	outb(0x3D5, 0);

	outb(0x3D4, 0x32);
	outb(0x3D5, inb(0x3D5)&~0x10);
}

/* ============ */


#define NSLOTS 4
#define NPCIREGS  5

/*
 * should use devfunc number/indirect method to be totally safe on
 * all machines, this works for now on 3 slot Moto boxes
 */

struct PCI_ConfigInfo {
	u_long * config_addr;
	u_long regs[NPCIREGS];
} PCI_slots [NSLOTS] = {
	{ (u_long *)0x80802000, { 0xDE, 0xAD, 0xBE, 0xEF } },
	{ (u_long *)0x80804000, { 0xDE, 0xAD, 0xBE, 0xEF } },
	{ (u_long *)0x80808000, { 0xDE, 0xAD, 0xBE, 0xEF } },
	{ (u_long *)0x80810000, { 0xDE, 0xAD, 0xBE, 0xEF } }
};


/*
 * The following code modifies the PCI Command register
 * to enable memory and I/O accesses.
 */
void
enablePCIvideo(int slot)
{
       volatile u_char * ppci;

        ppci =  (u_char *)PCI_slots[slot].config_addr;
	ppci[4] = 0x0003;         /* enable memory and I/O accesses */
	__asm volatile("eieio");

	outb(0x3d4, 0x11);
	outb(0x3d5, 0x0e);   /* unlock CR0-CR7 */
}

#define DEVID   0
#define CMD     1
#define CLASS   2
#define MEMBASE 4

int
scanPCI(void)
{
	int slt, r;
	struct PCI_ConfigInfo *pslot;
	int theSlot = -1;
	int highVgaSlot = -1;

	for (slt = 0; slt < NSLOTS; slt++) {
		pslot = &PCI_slots[slt];
		for (r = 0; r < NPCIREGS; r++) {
			pslot->regs[r] = bswap32(pslot->config_addr[r]);
		}

		if (pslot->regs[DEVID] != 0xFFFFFFFF) {     /* card in slot ? */
			if ((pslot->regs[CLASS] & 0xFFFFFF00) == 0x03000000) { /* VGA ? */
				highVgaSlot = slt;
				if ((pslot->regs[CMD] & 0x03)) { /* did firmware enable it ? */
					theSlot = slt;
				}
			}
		}
	}

	if (theSlot == -1)
		theSlot = highVgaSlot;

	return theSlot;
}

int
delayLoop(int k)
{
	volatile int a, b;
	volatile int i, j;
	a = 0;
	do {
		for (i = 0; i < 500; i++) {
			b = i;
			for (j = 0; j < 200; j++) {
				a = b+j;
			}
		}
	} while (k--);
	return a;
}

/* return Vendor ID of card in the slot */
static int
PCIVendor(int slotnum)
{
	struct PCI_ConfigInfo *pslot;

	pslot = &PCI_slots[slotnum];

	return (pslot->regs[DEVID] & 0xFFFF);
}

#ifdef DEBUG
static void
printslots(void)
{
	int i;
	for (i = 0; i < NSLOTS; i++) {
		printf("PCI Slot number: %d", i);
		printf(" Vendor ID: 0x%08x\n", PCIVendor(i));
	}
}
#endif /* DEBUG */
#endif /* CONS_VGA */
