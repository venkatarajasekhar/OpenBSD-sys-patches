/*	$NetBSD: dev_hppa.h,v 1.4 2005/12/11 12:17:25 christos Exp $	*/

/*	$OpenBSD: dev_hppa.h,v 1.3 1999/04/20 20:01:01 mickey Exp $	*/


#define IOPGSHIFT	11
#define	IONBPG		(1 << IOPGSHIFT)
#define IOPGOFSET	(IONBPG - 1)

struct disklabel;
struct hppa_dev {
	dev_t	bootdev;
	struct pz_device *pz_dev;	/* device descriptor */
	u_long	last_blk;		/* byte offset for last read blk */
	size_t	last_read;		/* amount read last time */
	u_long	part_off;		/* byte offset of boot partition */
	/* buffer to cache data (aligned properly) */
	char	*buf;
	char	ua_buf[IODC_MAXIOSIZ + IODC_MINIOSIZ];

};

#ifdef PDCDEBUG
#define	DEVPATH_PRINT(dp) \
	printf("%x, %d.%d.%d.%d.%d.%d, 0x%x, %x.%x.%x.%x.%x.%x\n", \
	       (dp)->dp_flags, (dp)->dp_bc[0], (dp)->dp_bc[1], (dp)->dp_bc[2], \
	       (dp)->dp_bc[3], (dp)->dp_bc[4], (dp)->dp_bc[5], (dp)->dp_mod, \
	       (dp)->dp_layers[0], (dp)->dp_layers[1], (dp)->dp_layers[2], \
	       (dp)->dp_layers[3], (dp)->dp_layers[4], (dp)->dp_layers[5]);
#define	PZDEV_PRINT(dp) \
	printf("devpath={%x, %d.%d.%d.%d.%d.%d, 0x%x, %x.%x.%x.%x.%x.%x}," \
	       "\n\thpa=%p, spa=%p, io=%p, class=%u\n", \
	       (dp)->pz_flags, (dp)->pz_bc[0], (dp)->pz_bc[1], (dp)->pz_bc[2], \
	       (dp)->pz_bc[3], (dp)->pz_bc[4], (dp)->pz_bc[5], (dp)->pz_mod, \
	       (dp)->pz_layers[0], (dp)->pz_layers[1], (dp)->pz_layers[2], \
	       (dp)->pz_layers[3], (dp)->pz_layers[4], (dp)->pz_layers[5], \
	       (dp)->pz_hpa, (dp)->pz_spa, (dp)->pz_iodc_io, (dp)->pz_class);
#endif

extern pdcio_t pdc;
extern int pdcbuf[];			/* PDC returns, pdc.c */
extern struct  pz_device ctdev;		/* cartridge tape (boot) device path */

int iodc_rw(char *, u_int, u_int, int func, struct pz_device *);
