#	$OpenBSD: Makefile,v 1.19 2004/02/07 20:23:05 deraadt Exp $
#	$NetBSD: Makefile,v 1.5 1995/09/15 21:05:21 pk Exp $

SUBDIR=	arch/alpha arch/hp300 arch/hppa arch/i386 arch/m68k \
	arch/mac68k arch/macppc arch/mvme68k arch/mvme88k \
	arch/mvmeppc arch/pegasos arch/sparc arch/sparc64 \
	arch/vax arch/cats arch/amd64

.include <bsd.subdir.mk>
