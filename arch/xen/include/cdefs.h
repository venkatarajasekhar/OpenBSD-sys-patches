/*	$OpenBSD: cdefs.h,v 1.1.1.1 2005/09/02 16:10:28 hshoexer Exp $	*/
/*	$NetBSD: cdefs.h,v 1.2 1995/03/23 20:10:26 jtc Exp $	*/

/*
 * Written by J.T. Conklin <jtc@wimsey.com> 01/17/95.
 * Public domain.
 */

#ifdef I686_CPU
#include <machine/i386/cdefs.h>
#endif

#ifdef amd64
#include <machine/amd64/cdefs.h>
#endif
