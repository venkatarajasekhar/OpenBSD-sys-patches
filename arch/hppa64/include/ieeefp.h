/*	$OpenBSD: ieeefp.h,v 1.1 2005/04/01 10:40:48 mickey Exp $	*/

/* 
 * Written by Miodrag Vallat.  Public domain.
 */

#ifndef _HPPA64_IEEEFP_H_
#define _HPPA64_IEEEFP_H_

typedef int fp_except;
#define FP_X_INV	0x10	/* invalid operation exception */
#define FP_X_DZ		0x08	/* divide-by-zero exception */
#define FP_X_OFL	0x04	/* overflow exception */
#define FP_X_UFL	0x02	/* underflow exception */
#define FP_X_IMP	0x01	/* imprecise (loss of precision) */

typedef enum {
    FP_RN=0,			/* round to nearest representable number */
    FP_RZ=1,			/* round to zero (truncate) */
    FP_RP=2,			/* round toward positive infinity */
    FP_RM=3			/* round toward negative infinity */
} fp_rnd;

#endif /* _HPPA64_IEEEFP_H_ */
