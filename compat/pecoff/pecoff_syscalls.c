/* $NetBSD: pecoff_syscalls.c,v 1.26 2006/09/01 21:19:45 matt Exp $ */

/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.25 2006/09/01 20:58:18 matt Exp
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: pecoff_syscalls.c,v 1.26 2006/09/01 21:19:45 matt Exp $");

#if defined(_KERNEL_OPT)
#if defined(_KERNEL_OPT)
#include "opt_ktrace.h"
#include "opt_nfsserver.h"
#include "opt_ntp.h"
#include "opt_compat_netbsd.h"
#include "opt_sysv.h"
#include "opt_compat_43.h"
#include "opt_posix.h"
#include "fs_lfs.h"
#include "fs_nfs.h"
#endif
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/sa.h>
#include <sys/shm.h>
#include <sys/syscallargs.h>
#include <compat/pecoff/pecoff_syscallargs.h>
#include <compat/sys/shm.h>
#endif /* _KERNEL_OPT */

const char *const pecoff_syscallnames[] = {
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"open",			/* 5 = open */
	"close",			/* 6 = close */
	"wait4",			/* 7 = wait4 */
	"#8 (excluded { int sys_creat ( const char * path , mode_t mode ) ; } ocreat)",		/* 8 = excluded { int sys_creat ( const char * path , mode_t mode ) ; } ocreat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"#11 (obsolete execv)",		/* 11 = obsolete execv */
	"chdir",			/* 12 = chdir */
	"fchdir",			/* 13 = fchdir */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"break",			/* 17 = break */
#ifdef COMPAT_20
	"getfsstat",			/* 18 = getfsstat */
#else
	"#18 (excluded compat_20_sys_getfsstat)",		/* 18 = excluded compat_20_sys_getfsstat */
#endif
	"#19 (excluded { long sys_lseek ( int fd , long offset , int whence ) ; } olseek)",		/* 19 = excluded { long sys_lseek ( int fd , long offset , int whence ) ; } olseek */
#ifdef COMPAT_43
	"getpid",			/* 20 = getpid */
#else
	"getpid",			/* 20 = getpid */
#endif
	"mount",			/* 21 = mount */
	"unmount",			/* 22 = unmount */
	"setuid",			/* 23 = setuid */
#ifdef COMPAT_43
	"getuid",			/* 24 = getuid */
#else
	"getuid",			/* 24 = getuid */
#endif
	"geteuid",			/* 25 = geteuid */
	"ptrace",			/* 26 = ptrace */
	"recvmsg",			/* 27 = recvmsg */
	"sendmsg",			/* 28 = sendmsg */
	"recvfrom",			/* 29 = recvfrom */
	"accept",			/* 30 = accept */
	"getpeername",			/* 31 = getpeername */
	"getsockname",			/* 32 = getsockname */
	"access",			/* 33 = access */
	"chflags",			/* 34 = chflags */
	"fchflags",			/* 35 = fchflags */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"#38 (excluded { int pecoff_compat_43_sys_stat ( const char * path , struct stat43 * ub ) ; } stat43)",		/* 38 = excluded { int pecoff_compat_43_sys_stat ( const char * path , struct stat43 * ub ) ; } stat43 */
	"getppid",			/* 39 = getppid */
	"#40 (excluded { int pecoff_compat_43_sys_lstat ( const char * path , struct stat43 * ub ) ; } lstat43)",		/* 40 = excluded { int pecoff_compat_43_sys_lstat ( const char * path , struct stat43 * ub ) ; } lstat43 */
	"dup",			/* 41 = dup */
	"pipe",			/* 42 = pipe */
	"getegid",			/* 43 = getegid */
	"profil",			/* 44 = profil */
#if defined(KTRACE) || !defined(_KERNEL)
	"ktrace",			/* 45 = ktrace */
#else
	"#45 (excluded ktrace)",		/* 45 = excluded ktrace */
#endif
	"#46 (excluded { int sys_sigaction ( int signum , const struct sigaction13 * nsa , struct sigaction13 * osa ) ; } sigaction13)",		/* 46 = excluded { int sys_sigaction ( int signum , const struct sigaction13 * nsa , struct sigaction13 * osa ) ; } sigaction13 */
#ifdef COMPAT_43
	"getgid",			/* 47 = getgid */
#else
	"getgid",			/* 47 = getgid */
#endif
	"#48 (excluded { int sys_sigprocmask ( int how , int mask ) ; } sigprocmask13)",		/* 48 = excluded { int sys_sigprocmask ( int how , int mask ) ; } sigprocmask13 */
	"__getlogin",			/* 49 = __getlogin */
	"__setlogin",			/* 50 = __setlogin */
	"acct",			/* 51 = acct */
	"#52 (excluded { int sys_sigpending ( void ) ; } sigpending13)",		/* 52 = excluded { int sys_sigpending ( void ) ; } sigpending13 */
	"#53 (excluded { int sys_sigaltstack ( const struct sigaltstack13 * nss , struct sigaltstack13 * oss ) ; } sigaltstack13)",		/* 53 = excluded { int sys_sigaltstack ( const struct sigaltstack13 * nss , struct sigaltstack13 * oss ) ; } sigaltstack13 */
	"ioctl",			/* 54 = ioctl */
	"#55 (excluded { int sys_reboot ( int opt ) ; } oreboot)",		/* 55 = excluded { int sys_reboot ( int opt ) ; } oreboot */
	"revoke",			/* 56 = revoke */
	"symlink",			/* 57 = symlink */
	"readlink",			/* 58 = readlink */
	"execve",			/* 59 = execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"#62 (excluded { int sys_fstat ( int fd , struct stat43 * sb ) ; } fstat43)",		/* 62 = excluded { int sys_fstat ( int fd , struct stat43 * sb ) ; } fstat43 */
	"#63 (excluded { int sys_getkerninfo ( int op , char * where , int * size , int arg ) ; } ogetkerninfo)",		/* 63 = excluded { int sys_getkerninfo ( int op , char * where , int * size , int arg ) ; } ogetkerninfo */
	"#64 (excluded { int sys_getpagesize ( void ) ; } ogetpagesize)",		/* 64 = excluded { int sys_getpagesize ( void ) ; } ogetpagesize */
	"#65 (excluded { int sys_msync ( caddr_t addr , size_t len ) ; })",		/* 65 = excluded { int sys_msync ( caddr_t addr , size_t len ) ; } */
	"vfork",			/* 66 = vfork */
	"#67 (obsolete vread)",		/* 67 = obsolete vread */
	"#68 (obsolete vwrite)",		/* 68 = obsolete vwrite */
	"sbrk",			/* 69 = sbrk */
	"sstk",			/* 70 = sstk */
	"#71 (excluded { int sys_mmap ( caddr_t addr , size_t len , int prot , int flags , int fd , long pos ) ; } ommap)",		/* 71 = excluded { int sys_mmap ( caddr_t addr , size_t len , int prot , int flags , int fd , long pos ) ; } ommap */
	"vadvise",			/* 72 = vadvise */
	"munmap",			/* 73 = munmap */
	"mprotect",			/* 74 = mprotect */
	"madvise",			/* 75 = madvise */
	"#76 (obsolete vhangup)",		/* 76 = obsolete vhangup */
	"#77 (obsolete vlimit)",		/* 77 = obsolete vlimit */
	"mincore",			/* 78 = mincore */
	"getgroups",			/* 79 = getgroups */
	"setgroups",			/* 80 = setgroups */
	"getpgrp",			/* 81 = getpgrp */
	"setpgid",			/* 82 = setpgid */
	"setitimer",			/* 83 = setitimer */
	"#84 (excluded { int sys_wait ( void ) ; } owait)",		/* 84 = excluded { int sys_wait ( void ) ; } owait */
	"#85 (excluded { int sys_swapon ( const char * name ) ; } oswapon)",		/* 85 = excluded { int sys_swapon ( const char * name ) ; } oswapon */
	"getitimer",			/* 86 = getitimer */
	"#87 (excluded { int sys_gethostname ( char * hostname , u_int len ) ; } ogethostname)",		/* 87 = excluded { int sys_gethostname ( char * hostname , u_int len ) ; } ogethostname */
	"#88 (excluded { int sys_sethostname ( char * hostname , u_int len ) ; } osethostname)",		/* 88 = excluded { int sys_sethostname ( char * hostname , u_int len ) ; } osethostname */
	"#89 (excluded { int sys_getdtablesize ( void ) ; } ogetdtablesize)",		/* 89 = excluded { int sys_getdtablesize ( void ) ; } ogetdtablesize */
	"dup2",			/* 90 = dup2 */
	"#91 (unimplemented getdopt)",		/* 91 = unimplemented getdopt */
	"fcntl",			/* 92 = fcntl */
	"select",			/* 93 = select */
	"#94 (unimplemented setdopt)",		/* 94 = unimplemented setdopt */
	"fsync",			/* 95 = fsync */
	"setpriority",			/* 96 = setpriority */
	"socket",			/* 97 = socket */
	"connect",			/* 98 = connect */
	"#99 (excluded { int sys_accept ( int s , caddr_t name , int * anamelen ) ; } oaccept)",		/* 99 = excluded { int sys_accept ( int s , caddr_t name , int * anamelen ) ; } oaccept */
	"getpriority",			/* 100 = getpriority */
	"#101 (excluded { int sys_send ( int s , caddr_t buf , int len , int flags ) ; } osend)",		/* 101 = excluded { int sys_send ( int s , caddr_t buf , int len , int flags ) ; } osend */
	"#102 (excluded { int sys_recv ( int s , caddr_t buf , int len , int flags ) ; } orecv)",		/* 102 = excluded { int sys_recv ( int s , caddr_t buf , int len , int flags ) ; } orecv */
	"#103 (excluded { int sys_sigreturn ( struct sigcontext13 * sigcntxp ) ; } sigreturn13)",		/* 103 = excluded { int sys_sigreturn ( struct sigcontext13 * sigcntxp ) ; } sigreturn13 */
	"bind",			/* 104 = bind */
	"setsockopt",			/* 105 = setsockopt */
	"listen",			/* 106 = listen */
	"#107 (obsolete vtimes)",		/* 107 = obsolete vtimes */
	"#108 (excluded { int sys_sigvec ( int signum , struct sigvec * nsv , struct sigvec * osv ) ; } osigvec)",		/* 108 = excluded { int sys_sigvec ( int signum , struct sigvec * nsv , struct sigvec * osv ) ; } osigvec */
	"#109 (excluded { int sys_sigblock ( int mask ) ; } osigblock)",		/* 109 = excluded { int sys_sigblock ( int mask ) ; } osigblock */
	"#110 (excluded { int sys_sigsetmask ( int mask ) ; } osigsetmask)",		/* 110 = excluded { int sys_sigsetmask ( int mask ) ; } osigsetmask */
	"#111 (excluded { int sys_sigsuspend ( int mask ) ; } sigsuspend13)",		/* 111 = excluded { int sys_sigsuspend ( int mask ) ; } sigsuspend13 */
	"#112 (excluded { int sys_sigstack ( struct sigstack * nss , struct sigstack * oss ) ; } osigstack)",		/* 112 = excluded { int sys_sigstack ( struct sigstack * nss , struct sigstack * oss ) ; } osigstack */
	"#113 (excluded { int sys_recvmsg ( int s , struct omsghdr * msg , int flags ) ; } orecvmsg)",		/* 113 = excluded { int sys_recvmsg ( int s , struct omsghdr * msg , int flags ) ; } orecvmsg */
	"#114 (excluded { int sys_sendmsg ( int s , caddr_t msg , int flags ) ; } osendmsg)",		/* 114 = excluded { int sys_sendmsg ( int s , caddr_t msg , int flags ) ; } osendmsg */
	"#115 (obsolete vtrace)",		/* 115 = obsolete vtrace */
	"gettimeofday",			/* 116 = gettimeofday */
	"getrusage",			/* 117 = getrusage */
	"getsockopt",			/* 118 = getsockopt */
	"#119 (obsolete resuba)",		/* 119 = obsolete resuba */
	"readv",			/* 120 = readv */
	"writev",			/* 121 = writev */
	"settimeofday",			/* 122 = settimeofday */
	"fchown",			/* 123 = fchown */
	"fchmod",			/* 124 = fchmod */
	"#125 (excluded { int sys_recvfrom ( int s , caddr_t buf , size_t len , int flags , caddr_t from , int * fromlenaddr ) ; } orecvfrom)",		/* 125 = excluded { int sys_recvfrom ( int s , caddr_t buf , size_t len , int flags , caddr_t from , int * fromlenaddr ) ; } orecvfrom */
	"setreuid",			/* 126 = setreuid */
	"setregid",			/* 127 = setregid */
	"rename",			/* 128 = rename */
	"#129 (excluded { int pecoff_compat_43_sys_truncate ( const char * path , long length ) ; } otruncate)",		/* 129 = excluded { int pecoff_compat_43_sys_truncate ( const char * path , long length ) ; } otruncate */
	"#130 (excluded { int sys_ftruncate ( int fd , long length ) ; } oftruncate)",		/* 130 = excluded { int sys_ftruncate ( int fd , long length ) ; } oftruncate */
	"flock",			/* 131 = flock */
	"mkfifo",			/* 132 = mkfifo */
	"sendto",			/* 133 = sendto */
	"shutdown",			/* 134 = shutdown */
	"socketpair",			/* 135 = socketpair */
	"mkdir",			/* 136 = mkdir */
	"rmdir",			/* 137 = rmdir */
	"utimes",			/* 138 = utimes */
	"#139 (obsolete 4.2 sigreturn)",		/* 139 = obsolete 4.2 sigreturn */
	"adjtime",			/* 140 = adjtime */
	"#141 (excluded { int sys_getpeername ( int fdes , caddr_t asa , int * alen ) ; } ogetpeername)",		/* 141 = excluded { int sys_getpeername ( int fdes , caddr_t asa , int * alen ) ; } ogetpeername */
	"#142 (excluded { int32_t sys_gethostid ( void ) ; } ogethostid)",		/* 142 = excluded { int32_t sys_gethostid ( void ) ; } ogethostid */
	"#143 (excluded { int sys_sethostid ( int32_t hostid ) ; } osethostid)",		/* 143 = excluded { int sys_sethostid ( int32_t hostid ) ; } osethostid */
	"#144 (excluded { int sys_getrlimit ( int which , struct orlimit * rlp ) ; } ogetrlimit)",		/* 144 = excluded { int sys_getrlimit ( int which , struct orlimit * rlp ) ; } ogetrlimit */
	"#145 (excluded { int sys_setrlimit ( int which , const struct orlimit * rlp ) ; } osetrlimit)",		/* 145 = excluded { int sys_setrlimit ( int which , const struct orlimit * rlp ) ; } osetrlimit */
	"#146 (excluded { int sys_killpg ( int pgid , int signum ) ; } okillpg)",		/* 146 = excluded { int sys_killpg ( int pgid , int signum ) ; } okillpg */
	"setsid",			/* 147 = setsid */
	"quotactl",			/* 148 = quotactl */
	"#149 (excluded { int sys_quota ( void ) ; } oquota)",		/* 149 = excluded { int sys_quota ( void ) ; } oquota */
	"#150 (excluded { int sys_getsockname ( int fdec , caddr_t asa , int * alen ) ; } ogetsockname)",		/* 150 = excluded { int sys_getsockname ( int fdec , caddr_t asa , int * alen ) ; } ogetsockname */
	"#151 (unimplemented)",		/* 151 = unimplemented */
	"#152 (unimplemented)",		/* 152 = unimplemented */
	"#153 (unimplemented)",		/* 153 = unimplemented */
	"#154 (unimplemented)",		/* 154 = unimplemented */
#if defined(NFS) || defined(NFSSERVER) || !defined(_KERNEL)
	"nfssvc",			/* 155 = nfssvc */
#else
	"#155 (excluded nfssvc)",		/* 155 = excluded nfssvc */
#endif
	"#156 (excluded { int sys_getdirentries ( int fd , char * buf , u_int count , long * basep ) ; } ogetdirentries)",		/* 156 = excluded { int sys_getdirentries ( int fd , char * buf , u_int count , long * basep ) ; } ogetdirentries */
	"statfs",			/* 157 = statfs */
#ifdef COMPAT_20
	"fstatfs",			/* 158 = fstatfs */
#else
	"#158 (excluded compat_20_sys_fstatfs)",		/* 158 = excluded compat_20_sys_fstatfs */
#endif
	"#159 (unimplemented)",		/* 159 = unimplemented */
	"#160 (unimplemented)",		/* 160 = unimplemented */
#ifdef COMPAT_30
	"getfh",			/* 161 = getfh */
#else
	"#161 (excluded compat_30_sys_getfh)",		/* 161 = excluded compat_30_sys_getfh */
#endif
	"#162 (excluded { int sys_getdomainname ( char * domainname , int len ) ; } ogetdomainname)",		/* 162 = excluded { int sys_getdomainname ( char * domainname , int len ) ; } ogetdomainname */
	"#163 (excluded { int sys_setdomainname ( char * domainname , int len ) ; } osetdomainname)",		/* 163 = excluded { int sys_setdomainname ( char * domainname , int len ) ; } osetdomainname */
	"#164 (excluded { int sys_uname ( struct outsname * name ) ; } ouname)",		/* 164 = excluded { int sys_uname ( struct outsname * name ) ; } ouname */
	"sysarch",			/* 165 = sysarch */
	"#166 (unimplemented)",		/* 166 = unimplemented */
	"#167 (unimplemented)",		/* 167 = unimplemented */
	"#168 (unimplemented)",		/* 168 = unimplemented */
#if (defined(SYSVSEM) || !defined(_KERNEL)) && !defined(_LP64)
	"#169 (excluded { int sys_semsys ( int which , int a2 , int a3 , int a4 , int a5 ) ; } osemsys)",		/* 169 = excluded { int sys_semsys ( int which , int a2 , int a3 , int a4 , int a5 ) ; } osemsys */
#else
	"#169 (excluded 1.0 semsys)",		/* 169 = excluded 1.0 semsys */
#endif
#if (defined(SYSVMSG) || !defined(_KERNEL)) && !defined(_LP64)
	"#170 (excluded { int sys_msgsys ( int which , int a2 , int a3 , int a4 , int a5 , int a6 ) ; } omsgsys)",		/* 170 = excluded { int sys_msgsys ( int which , int a2 , int a3 , int a4 , int a5 , int a6 ) ; } omsgsys */
#else
	"#170 (excluded 1.0 msgsys)",		/* 170 = excluded 1.0 msgsys */
#endif
#if (defined(SYSVSHM) || !defined(_KERNEL)) && !defined(_LP64)
	"#171 (excluded { int sys_shmsys ( int which , int a2 , int a3 , int a4 ) ; } oshmsys)",		/* 171 = excluded { int sys_shmsys ( int which , int a2 , int a3 , int a4 ) ; } oshmsys */
#else
	"#171 (excluded 1.0 shmsys)",		/* 171 = excluded 1.0 shmsys */
#endif
	"#172 (unimplemented)",		/* 172 = unimplemented */
	"pread",			/* 173 = pread */
	"pwrite",			/* 174 = pwrite */
	"#175 (unimplemented { int sys_ntp_gettime ( struct ntptimeval * ntvp ) ; })",		/* 175 = unimplemented { int sys_ntp_gettime ( struct ntptimeval * ntvp ) ; } */
#if defined(NTP) || !defined(_KERNEL)
	"ntp_adjtime",			/* 176 = ntp_adjtime */
#else
	"#176 (excluded ntp_adjtime)",		/* 176 = excluded ntp_adjtime */
#endif
	"#177 (unimplemented)",		/* 177 = unimplemented */
	"#178 (unimplemented)",		/* 178 = unimplemented */
	"#179 (unimplemented)",		/* 179 = unimplemented */
	"#180 (unimplemented)",		/* 180 = unimplemented */
	"setgid",			/* 181 = setgid */
	"setegid",			/* 182 = setegid */
	"seteuid",			/* 183 = seteuid */
#if defined(LFS) || !defined(_KERNEL)
	"lfs_bmapv",			/* 184 = lfs_bmapv */
	"lfs_markv",			/* 185 = lfs_markv */
	"lfs_segclean",			/* 186 = lfs_segclean */
	"lfs_segwait",			/* 187 = lfs_segwait */
#else
	"#184 (excluded lfs_bmapv)",		/* 184 = excluded lfs_bmapv */
	"#185 (excluded lfs_markv)",		/* 185 = excluded lfs_markv */
	"#186 (excluded lfs_segclean)",		/* 186 = excluded lfs_segclean */
	"#187 (excluded lfs_segwait)",		/* 187 = excluded lfs_segwait */
#endif
	"#188 (excluded { int pecoff_compat_12_sys_stat ( const char * path , struct stat12 * ub ) ; } stat12)",		/* 188 = excluded { int pecoff_compat_12_sys_stat ( const char * path , struct stat12 * ub ) ; } stat12 */
	"#189 (excluded { int sys_fstat ( int fd , struct stat12 * sb ) ; } fstat12)",		/* 189 = excluded { int sys_fstat ( int fd , struct stat12 * sb ) ; } fstat12 */
	"#190 (excluded { int pecoff_compat_12_sys_lstat ( const char * path , struct stat12 * ub ) ; } lstat12)",		/* 190 = excluded { int pecoff_compat_12_sys_lstat ( const char * path , struct stat12 * ub ) ; } lstat12 */
	"pathconf",			/* 191 = pathconf */
	"fpathconf",			/* 192 = fpathconf */
	"#193 (unimplemented)",		/* 193 = unimplemented */
	"getrlimit",			/* 194 = getrlimit */
	"setrlimit",			/* 195 = setrlimit */
	"#196 (excluded { int sys_getdirentries ( int fd , char * buf , u_int count , long * basep ) ; })",		/* 196 = excluded { int sys_getdirentries ( int fd , char * buf , u_int count , long * basep ) ; } */
	"mmap",			/* 197 = mmap */
	"__syscall",			/* 198 = __syscall */
	"lseek",			/* 199 = lseek */
	"truncate",			/* 200 = truncate */
	"ftruncate",			/* 201 = ftruncate */
	"__sysctl",			/* 202 = __sysctl */
	"mlock",			/* 203 = mlock */
	"munlock",			/* 204 = munlock */
	"undelete",			/* 205 = undelete */
	"futimes",			/* 206 = futimes */
	"getpgid",			/* 207 = getpgid */
	"reboot",			/* 208 = reboot */
	"poll",			/* 209 = poll */
#if defined(LKM) || !defined(_KERNEL)
	"lkmnosys",			/* 210 = lkmnosys */
	"lkmnosys",			/* 211 = lkmnosys */
	"lkmnosys",			/* 212 = lkmnosys */
	"lkmnosys",			/* 213 = lkmnosys */
	"lkmnosys",			/* 214 = lkmnosys */
	"lkmnosys",			/* 215 = lkmnosys */
	"lkmnosys",			/* 216 = lkmnosys */
	"lkmnosys",			/* 217 = lkmnosys */
	"lkmnosys",			/* 218 = lkmnosys */
	"lkmnosys",			/* 219 = lkmnosys */
#else	/* !LKM */
	"#210 (excluded lkmnosys)",		/* 210 = excluded lkmnosys */
	"#211 (excluded lkmnosys)",		/* 211 = excluded lkmnosys */
	"#212 (excluded lkmnosys)",		/* 212 = excluded lkmnosys */
	"#213 (excluded lkmnosys)",		/* 213 = excluded lkmnosys */
	"#214 (excluded lkmnosys)",		/* 214 = excluded lkmnosys */
	"#215 (excluded lkmnosys)",		/* 215 = excluded lkmnosys */
	"#216 (excluded lkmnosys)",		/* 216 = excluded lkmnosys */
	"#217 (excluded lkmnosys)",		/* 217 = excluded lkmnosys */
	"#218 (excluded lkmnosys)",		/* 218 = excluded lkmnosys */
	"#219 (excluded lkmnosys)",		/* 219 = excluded lkmnosys */
#endif	/* !LKM */
#if defined(SYSVSEM) || !defined(_KERNEL)
	"#220 (excluded { int sys___semctl ( int semid , int semnum , int cmd , union __semun * arg ) ; })",		/* 220 = excluded { int sys___semctl ( int semid , int semnum , int cmd , union __semun * arg ) ; } */
	"semget",			/* 221 = semget */
	"semop",			/* 222 = semop */
	"semconfig",			/* 223 = semconfig */
#else
	"#220 (excluded compat_14_semctl)",		/* 220 = excluded compat_14_semctl */
	"#221 (excluded semget)",		/* 221 = excluded semget */
	"#222 (excluded semop)",		/* 222 = excluded semop */
	"#223 (excluded semconfig)",		/* 223 = excluded semconfig */
#endif
#if defined(SYSVMSG) || !defined(_KERNEL)
	"#224 (excluded { int sys_msgctl ( int msqid , int cmd , struct msqid_ds14 * buf ) ; })",		/* 224 = excluded { int sys_msgctl ( int msqid , int cmd , struct msqid_ds14 * buf ) ; } */
	"msgget",			/* 225 = msgget */
	"msgsnd",			/* 226 = msgsnd */
	"msgrcv",			/* 227 = msgrcv */
#else
	"#224 (excluded compat_14_msgctl)",		/* 224 = excluded compat_14_msgctl */
	"#225 (excluded msgget)",		/* 225 = excluded msgget */
	"#226 (excluded msgsnd)",		/* 226 = excluded msgsnd */
	"#227 (excluded msgrcv)",		/* 227 = excluded msgrcv */
#endif
#if defined(SYSVSHM) || !defined(_KERNEL)
	"shmat",			/* 228 = shmat */
	"#229 (excluded { int sys_shmctl ( int shmid , int cmd , struct shmid_ds14 * buf ) ; })",		/* 229 = excluded { int sys_shmctl ( int shmid , int cmd , struct shmid_ds14 * buf ) ; } */
	"shmdt",			/* 230 = shmdt */
	"shmget",			/* 231 = shmget */
#else
	"#228 (excluded shmat)",		/* 228 = excluded shmat */
	"#229 (excluded compat_14_shmctl)",		/* 229 = excluded compat_14_shmctl */
	"#230 (excluded shmdt)",		/* 230 = excluded shmdt */
	"#231 (excluded shmget)",		/* 231 = excluded shmget */
#endif
	"clock_gettime",			/* 232 = clock_gettime */
	"clock_settime",			/* 233 = clock_settime */
	"clock_getres",			/* 234 = clock_getres */
	"timer_create",			/* 235 = timer_create */
	"timer_delete",			/* 236 = timer_delete */
	"timer_settime",			/* 237 = timer_settime */
	"timer_gettime",			/* 238 = timer_gettime */
	"timer_getoverrun",			/* 239 = timer_getoverrun */
	"nanosleep",			/* 240 = nanosleep */
	"fdatasync",			/* 241 = fdatasync */
	"mlockall",			/* 242 = mlockall */
	"munlockall",			/* 243 = munlockall */
	"__sigtimedwait",			/* 244 = __sigtimedwait */
	"#245 (unimplemented sys_sigqueue)",		/* 245 = unimplemented sys_sigqueue */
	"#246 (unimplemented)",		/* 246 = unimplemented */
#if defined(P1003_1B_SEMAPHORE) || (!defined(_KERNEL) && defined(_LIBC))
	"_ksem_init",			/* 247 = _ksem_init */
	"_ksem_open",			/* 248 = _ksem_open */
	"_ksem_unlink",			/* 249 = _ksem_unlink */
	"_ksem_close",			/* 250 = _ksem_close */
	"_ksem_post",			/* 251 = _ksem_post */
	"_ksem_wait",			/* 252 = _ksem_wait */
	"_ksem_trywait",			/* 253 = _ksem_trywait */
	"_ksem_getvalue",			/* 254 = _ksem_getvalue */
	"_ksem_destroy",			/* 255 = _ksem_destroy */
	"#256 (unimplemented sys__ksem_timedwait)",		/* 256 = unimplemented sys__ksem_timedwait */
#else
	"#247 (excluded sys__ksem_init)",		/* 247 = excluded sys__ksem_init */
	"#248 (excluded sys__ksem_open)",		/* 248 = excluded sys__ksem_open */
	"#249 (excluded sys__ksem_unlink)",		/* 249 = excluded sys__ksem_unlink */
	"#250 (excluded sys__ksem_close)",		/* 250 = excluded sys__ksem_close */
	"#251 (excluded sys__ksem_post)",		/* 251 = excluded sys__ksem_post */
	"#252 (excluded sys__ksem_wait)",		/* 252 = excluded sys__ksem_wait */
	"#253 (excluded sys__ksem_trywait)",		/* 253 = excluded sys__ksem_trywait */
	"#254 (excluded sys__ksem_getvalue)",		/* 254 = excluded sys__ksem_getvalue */
	"#255 (excluded sys__ksem_destroy)",		/* 255 = excluded sys__ksem_destroy */
	"#256 (unimplemented sys__ksem_timedwait)",		/* 256 = unimplemented sys__ksem_timedwait */
#endif
	"#257 (unimplemented sys_mq_open)",		/* 257 = unimplemented sys_mq_open */
	"#258 (unimplemented sys_mq_close)",		/* 258 = unimplemented sys_mq_close */
	"#259 (unimplemented sys_mq_unlink)",		/* 259 = unimplemented sys_mq_unlink */
	"#260 (unimplemented sys_mq_getattr)",		/* 260 = unimplemented sys_mq_getattr */
	"#261 (unimplemented sys_mq_setattr)",		/* 261 = unimplemented sys_mq_setattr */
	"#262 (unimplemented sys_mq_notify)",		/* 262 = unimplemented sys_mq_notify */
	"#263 (unimplemented sys_mq_send)",		/* 263 = unimplemented sys_mq_send */
	"#264 (unimplemented sys_mq_receive)",		/* 264 = unimplemented sys_mq_receive */
	"#265 (unimplemented sys_mq_timedsend)",		/* 265 = unimplemented sys_mq_timedsend */
	"#266 (unimplemented sys_mq_timedreceive)",		/* 266 = unimplemented sys_mq_timedreceive */
	"#267 (unimplemented)",		/* 267 = unimplemented */
	"#268 (unimplemented)",		/* 268 = unimplemented */
	"#269 (unimplemented)",		/* 269 = unimplemented */
	"__posix_rename",			/* 270 = __posix_rename */
	"swapctl",			/* 271 = swapctl */
#ifdef COMPAT_30
	"getdents",			/* 272 = getdents */
#endif
	"minherit",			/* 273 = minherit */
	"lchmod",			/* 274 = lchmod */
	"lchown",			/* 275 = lchown */
	"lutimes",			/* 276 = lutimes */
	"__msync13",			/* 277 = __msync13 */
#ifdef COMPAT_30
	"__stat13",			/* 278 = __stat13 */
	"__fstat13",			/* 279 = __fstat13 */
	"__lstat13",			/* 280 = __lstat13 */
#endif
	"__sigaltstack14",			/* 281 = __sigaltstack14 */
	"__vfork14",			/* 282 = __vfork14 */
	"__posix_chown",			/* 283 = __posix_chown */
	"__posix_fchown",			/* 284 = __posix_fchown */
	"__posix_lchown",			/* 285 = __posix_lchown */
	"getsid",			/* 286 = getsid */
	"__clone",			/* 287 = __clone */
#if defined(KTRACE) || !defined(_KERNEL)
	"fktrace",			/* 288 = fktrace */
#else
	"#288 (excluded ktrace)",		/* 288 = excluded ktrace */
#endif
	"preadv",			/* 289 = preadv */
	"pwritev",			/* 290 = pwritev */
#ifdef COMPAT_16
	"__sigaction14",			/* 291 = __sigaction14 */
#else
	"#291 (excluded compat_16_sys___sigaction14)",		/* 291 = excluded compat_16_sys___sigaction14 */
#endif
	"__sigpending14",			/* 292 = __sigpending14 */
	"__sigprocmask14",			/* 293 = __sigprocmask14 */
	"__sigsuspend14",			/* 294 = __sigsuspend14 */
#ifdef COMPAT_16
	"__sigreturn14",			/* 295 = __sigreturn14 */
#else
	"#295 (excluded compat_16_sys___sigreturn14)",		/* 295 = excluded compat_16_sys___sigreturn14 */
#endif
	"__getcwd",			/* 296 = __getcwd */
	"fchroot",			/* 297 = fchroot */
#ifdef COMPAT_30
	"fhopen",			/* 298 = fhopen */
	"fhstat",			/* 299 = fhstat */
#else
	"#298 (excluded compat_30_sys_fhopen)",		/* 298 = excluded compat_30_sys_fhopen */
	"#299 (excluded compat_30_sys_fhstat)",		/* 299 = excluded compat_30_sys_fhstat */
#endif
#ifdef COMPAT_20
	"fhstatfs",			/* 300 = fhstatfs */
#else
	"#300 (excluded compat_20_sys_fhstatfs)",		/* 300 = excluded compat_20_sys_fhstatfs */
#endif
#if defined(SYSVSEM) || !defined(_KERNEL)
	"____semctl13",			/* 301 = ____semctl13 */
#else
	"#301 (excluded ____semctl13)",		/* 301 = excluded ____semctl13 */
#endif
#if defined(SYSVMSG) || !defined(_KERNEL)
	"__msgctl13",			/* 302 = __msgctl13 */
#else
	"#302 (excluded __msgctl13)",		/* 302 = excluded __msgctl13 */
#endif
#if defined(SYSVSHM) || !defined(_KERNEL)
	"__shmctl13",			/* 303 = __shmctl13 */
#else
	"#303 (excluded __shmctl13)",		/* 303 = excluded __shmctl13 */
#endif
	"lchflags",			/* 304 = lchflags */
	"issetugid",			/* 305 = issetugid */
	"utrace",			/* 306 = utrace */
	"getcontext",			/* 307 = getcontext */
	"setcontext",			/* 308 = setcontext */
	"_lwp_create",			/* 309 = _lwp_create */
	"_lwp_exit",			/* 310 = _lwp_exit */
	"_lwp_self",			/* 311 = _lwp_self */
	"_lwp_wait",			/* 312 = _lwp_wait */
	"_lwp_suspend",			/* 313 = _lwp_suspend */
	"_lwp_continue",			/* 314 = _lwp_continue */
	"_lwp_wakeup",			/* 315 = _lwp_wakeup */
	"_lwp_getprivate",			/* 316 = _lwp_getprivate */
	"_lwp_setprivate",			/* 317 = _lwp_setprivate */
	"#318 (unimplemented)",		/* 318 = unimplemented */
	"#319 (unimplemented)",		/* 319 = unimplemented */
	"#320 (unimplemented)",		/* 320 = unimplemented */
	"#321 (unimplemented)",		/* 321 = unimplemented */
	"#322 (unimplemented)",		/* 322 = unimplemented */
	"#323 (unimplemented)",		/* 323 = unimplemented */
	"#324 (unimplemented)",		/* 324 = unimplemented */
	"#325 (unimplemented)",		/* 325 = unimplemented */
	"#326 (unimplemented)",		/* 326 = unimplemented */
	"#327 (unimplemented)",		/* 327 = unimplemented */
	"#328 (unimplemented)",		/* 328 = unimplemented */
	"#329 (unimplemented)",		/* 329 = unimplemented */
	"sa_register",			/* 330 = sa_register */
	"sa_stacks",			/* 331 = sa_stacks */
	"sa_enable",			/* 332 = sa_enable */
	"sa_setconcurrency",			/* 333 = sa_setconcurrency */
	"sa_yield",			/* 334 = sa_yield */
	"sa_preempt",			/* 335 = sa_preempt */
	"#336 (obsolete sys_sa_unblockyield)",		/* 336 = obsolete sys_sa_unblockyield */
	"#337 (unimplemented)",		/* 337 = unimplemented */
	"#338 (unimplemented)",		/* 338 = unimplemented */
	"#339 (unimplemented)",		/* 339 = unimplemented */
	"__sigaction_sigtramp",			/* 340 = __sigaction_sigtramp */
	"pmc_get_info",			/* 341 = pmc_get_info */
	"pmc_control",			/* 342 = pmc_control */
	"rasctl",			/* 343 = rasctl */
	"kqueue",			/* 344 = kqueue */
	"kevent",			/* 345 = kevent */
	"#346 (unimplemented sys_sched_setparam)",		/* 346 = unimplemented sys_sched_setparam */
	"#347 (unimplemented sys_sched_getparam)",		/* 347 = unimplemented sys_sched_getparam */
	"#348 (unimplemented sys_sched_setscheduler)",		/* 348 = unimplemented sys_sched_setscheduler */
	"#349 (unimplemented sys_sched_getscheduler)",		/* 349 = unimplemented sys_sched_getscheduler */
	"#350 (unimplemented sys_sched_yield)",		/* 350 = unimplemented sys_sched_yield */
	"#351 (unimplemented sys_sched_get_priority_max)",		/* 351 = unimplemented sys_sched_get_priority_max */
	"#352 (unimplemented sys_sched_get_priority_min)",		/* 352 = unimplemented sys_sched_get_priority_min */
	"#353 (unimplemented sys_sched_rr_get_interval)",		/* 353 = unimplemented sys_sched_rr_get_interval */
	"fsync_range",			/* 354 = fsync_range */
	"uuidgen",			/* 355 = uuidgen */
	"getvfsstat",			/* 356 = getvfsstat */
	"statvfs1",			/* 357 = statvfs1 */
	"fstatvfs1",			/* 358 = fstatvfs1 */
#ifdef COMPAT_30
	"fhstatvfs1",			/* 359 = fhstatvfs1 */
#else
	"#359 (excluded compat_30_sys_fhstatvfs1)",		/* 359 = excluded compat_30_sys_fhstatvfs1 */
#endif
	"extattrctl",			/* 360 = extattrctl */
	"extattr_set_file",			/* 361 = extattr_set_file */
	"extattr_get_file",			/* 362 = extattr_get_file */
	"extattr_delete_file",			/* 363 = extattr_delete_file */
	"extattr_set_fd",			/* 364 = extattr_set_fd */
	"extattr_get_fd",			/* 365 = extattr_get_fd */
	"extattr_delete_fd",			/* 366 = extattr_delete_fd */
	"extattr_set_link",			/* 367 = extattr_set_link */
	"extattr_get_link",			/* 368 = extattr_get_link */
	"extattr_delete_link",			/* 369 = extattr_delete_link */
	"extattr_list_fd",			/* 370 = extattr_list_fd */
	"extattr_list_file",			/* 371 = extattr_list_file */
	"extattr_list_link",			/* 372 = extattr_list_link */
	"pselect",			/* 373 = pselect */
	"pollts",			/* 374 = pollts */
	"setxattr",			/* 375 = setxattr */
	"lsetxattr",			/* 376 = lsetxattr */
	"fsetxattr",			/* 377 = fsetxattr */
	"getxattr",			/* 378 = getxattr */
	"lgetxattr",			/* 379 = lgetxattr */
	"fgetxattr",			/* 380 = fgetxattr */
	"listxattr",			/* 381 = listxattr */
	"llistxattr",			/* 382 = llistxattr */
	"flistxattr",			/* 383 = flistxattr */
	"removexattr",			/* 384 = removexattr */
	"lremovexattr",			/* 385 = lremovexattr */
	"fremovexattr",			/* 386 = fremovexattr */
	"__stat30",			/* 387 = __stat30 */
	"__fstat30",			/* 388 = __fstat30 */
	"__lstat30",			/* 389 = __lstat30 */
	"__getdents30",			/* 390 = __getdents30 */
	"posix_fadvise",			/* 391 = posix_fadvise */
#ifdef COMPAT_30
	"__fhstat30",			/* 392 = __fhstat30 */
#else
	"#392 (excluded compat_30_sys___fhstat30)",		/* 392 = excluded compat_30_sys___fhstat30 */
#endif
	"__ntp_gettime30",			/* 393 = __ntp_gettime30 */
	"__socket30",			/* 394 = __socket30 */
	"__getfh30",			/* 395 = __getfh30 */
	"__fhopen40",			/* 396 = __fhopen40 */
	"__fhstatvfs140",			/* 397 = __fhstatvfs140 */
};
