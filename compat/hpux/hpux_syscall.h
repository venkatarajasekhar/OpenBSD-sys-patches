/*	$OpenBSD: hpux_syscall.h,v 1.8 2001/10/10 23:44:26 art Exp $	*/

/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.7 2001/10/10 23:43:44 art Exp 
 */

/* syscall: "syscall" ret: "int" args: */
#define	HPUX_SYS_syscall	0

/* syscall: "exit" ret: "int" args: "int" */
#define	HPUX_SYS_exit	1

/* syscall: "fork" ret: "int" args: */
#define	HPUX_SYS_fork	2

/* syscall: "read" ret: "int" args: "int" "char *" "u_int" */
#define	HPUX_SYS_read	3

/* syscall: "write" ret: "int" args: "int" "char *" "u_int" */
#define	HPUX_SYS_write	4

/* syscall: "open" ret: "int" args: "char *" "int" "int" */
#define	HPUX_SYS_open	5

/* syscall: "close" ret: "int" args: "int" */
#define	HPUX_SYS_close	6

/* syscall: "wait" ret: "int" args: "int *" */
#define	HPUX_SYS_wait	7

/* syscall: "creat" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_creat	8

/* syscall: "link" ret: "int" args: "char *" "char *" */
#define	HPUX_SYS_link	9

/* syscall: "unlink" ret: "int" args: "char *" */
#define	HPUX_SYS_unlink	10

/* syscall: "execv" ret: "int" args: "char *" "char **" */
#define	HPUX_SYS_execv	11

/* syscall: "chdir" ret: "int" args: "char *" */
#define	HPUX_SYS_chdir	12

/* syscall: "time_6x" ret: "int" args: "time_t *" */
#define	HPUX_SYS_time_6x	13

/* syscall: "mknod" ret: "int" args: "char *" "int" "int" */
#define	HPUX_SYS_mknod	14

/* syscall: "chmod" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_chmod	15

/* syscall: "chown" ret: "int" args: "char *" "int" "int" */
#define	HPUX_SYS_chown	16

/* syscall: "obreak" ret: "int" args: "char *" */
#define	HPUX_SYS_obreak	17

/* syscall: "stat_6x" ret: "int" args: "char *" "struct hpux_ostat *" */
#define	HPUX_SYS_stat_6x	18

/* syscall: "lseek" ret: "long" args: "int" "long" "int" */
#define	HPUX_SYS_lseek	19

/* syscall: "getpid" ret: "pid_t" args: */
#define	HPUX_SYS_getpid	20

/* syscall: "setuid" ret: "int" args: "uid_t" */
#define	HPUX_SYS_setuid	23

/* syscall: "getuid" ret: "uid_t" args: */
#define	HPUX_SYS_getuid	24

/* syscall: "stime_6x" ret: "int" args: "int" */
#define	HPUX_SYS_stime_6x	25

/* syscall: "ptrace" ret: "int" args: "int" "int" "int *" "int" */
#define	HPUX_SYS_ptrace	26

/* syscall: "alarm_6x" ret: "int" args: "int" */
#define	HPUX_SYS_alarm_6x	27

/* syscall: "fstat_6x" ret: "int" args: "int" "struct hpux_ostat *" */
#define	HPUX_SYS_fstat_6x	28

/* syscall: "pause_6x" ret: "int" args: */
#define	HPUX_SYS_pause_6x	29

/* syscall: "utime_6x" ret: "int" args: "char *" "time_t *" */
#define	HPUX_SYS_utime_6x	30

/* syscall: "stty_6x" ret: "int" args: "int" "caddr_t" */
#define	HPUX_SYS_stty_6x	31

/* syscall: "gtty_6x" ret: "int" args: "int" "caddr_t" */
#define	HPUX_SYS_gtty_6x	32

/* syscall: "access" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_access	33

/* syscall: "nice_6x" ret: "int" args: "int" */
#define	HPUX_SYS_nice_6x	34

/* syscall: "ftime_6x" ret: "int" args: "struct hpux_timeb *" */
#define	HPUX_SYS_ftime_6x	35

/* syscall: "sync" ret: "int" args: */
#define	HPUX_SYS_sync	36

/* syscall: "kill" ret: "int" args: "pid_t" "int" */
#define	HPUX_SYS_kill	37

/* syscall: "stat" ret: "int" args: "char *" "struct hpux_stat *" */
#define	HPUX_SYS_stat	38

/* syscall: "setpgrp_6x" ret: "int" args: */
#define	HPUX_SYS_setpgrp_6x	39

/* syscall: "lstat" ret: "int" args: "char *" "struct hpux_stat *" */
#define	HPUX_SYS_lstat	40

/* syscall: "dup" ret: "int" args: "int" */
#define	HPUX_SYS_dup	41

/* syscall: "opipe" ret: "int" args: */
#define	HPUX_SYS_opipe	42

/* syscall: "times_6x" ret: "int" args: "struct tms *" */
#define	HPUX_SYS_times_6x	43

/* syscall: "profil" ret: "int" args: "caddr_t" "u_int" "u_int" "u_int" */
#define	HPUX_SYS_profil	44

/* syscall: "setgid" ret: "int" args: "gid_t" */
#define	HPUX_SYS_setgid	46

/* syscall: "getgid" ret: "gid_t" args: */
#define	HPUX_SYS_getgid	47

/* syscall: "ssig_6x" ret: "int" args: "int" "sig_t" */
#define	HPUX_SYS_ssig_6x	48

/* syscall: "ioctl" ret: "int" args: "int" "int" "caddr_t" */
#define	HPUX_SYS_ioctl	54

/* syscall: "symlink" ret: "int" args: "char *" "char *" */
#define	HPUX_SYS_symlink	56

/* syscall: "utssys" ret: "int" args: "struct hpux_utsname *" "int" "int" */
#define	HPUX_SYS_utssys	57

/* syscall: "readlink" ret: "int" args: "char *" "char *" "int" */
#define	HPUX_SYS_readlink	58

/* syscall: "execve" ret: "int" args: "char *" "char **" "char **" */
#define	HPUX_SYS_execve	59

/* syscall: "umask" ret: "int" args: "int" */
#define	HPUX_SYS_umask	60

/* syscall: "chroot" ret: "int" args: "char *" */
#define	HPUX_SYS_chroot	61

/* syscall: "fcntl" ret: "int" args: "int" "int" "int" */
#define	HPUX_SYS_fcntl	62

/* syscall: "ulimit" ret: "int" args: "int" "int" */
#define	HPUX_SYS_ulimit	63

/* syscall: "vfork" ret: "int" args: */
#define	HPUX_SYS_vfork	66

/* syscall: "vread" ret: "int" args: "int" "char *" "u_int" */
#define	HPUX_SYS_vread	67

/* syscall: "vwrite" ret: "int" args: "int" "char *" "u_int" */
#define	HPUX_SYS_vwrite	68

/* syscall: "mmap" ret: "int" args: "caddr_t" "size_t" "int" "int" "int" "long" */
#define	HPUX_SYS_mmap	71

/* syscall: "munmap" ret: "int" args: "caddr_t" "size_t" */
#define	HPUX_SYS_munmap	73

/* syscall: "mprotect" ret: "int" args: "caddr_t" "size_t" "int" */
#define	HPUX_SYS_mprotect	74

/* syscall: "getgroups" ret: "int" args: "u_int" "gid_t *" */
#define	HPUX_SYS_getgroups	79

/* syscall: "setgroups" ret: "int" args: "u_int" "gid_t *" */
#define	HPUX_SYS_setgroups	80

/* syscall: "getpgrp2" ret: "int" args: "pid_t" */
#define	HPUX_SYS_getpgrp2	81

/* syscall: "setpgrp2" ret: "int" args: "pid_t" "pid_t" */
#define	HPUX_SYS_setpgrp2	82

/* syscall: "setitimer" ret: "int" args: "u_int" "struct itimerval *" "struct itimerval *" */
#define	HPUX_SYS_setitimer	83

/* syscall: "wait3" ret: "int" args: "int *" "int" "int" */
#define	HPUX_SYS_wait3	84

/* syscall: "getitimer" ret: "int" args: "u_int" "struct itimerval *" */
#define	HPUX_SYS_getitimer	86

/* syscall: "dup2" ret: "int" args: "u_int" "u_int" */
#define	HPUX_SYS_dup2	90

/* syscall: "fstat" ret: "int" args: "int" "struct hpux_stat *" */
#define	HPUX_SYS_fstat	92

/* syscall: "select" ret: "int" args: "u_int" "fd_set *" "fd_set *" "fd_set *" "struct timeval *" */
#define	HPUX_SYS_select	93

/* syscall: "fsync" ret: "int" args: "int" */
#define	HPUX_SYS_fsync	95

/* syscall: "sigreturn" ret: "int" args: "struct hpuxsigcontext *" */
#define	HPUX_SYS_sigreturn	103

/* syscall: "sigvec" ret: "int" args: "int" "struct sigvec *" "struct sigvec *" */
#define	HPUX_SYS_sigvec	108

/* syscall: "sigblock" ret: "int" args: "int" */
#define	HPUX_SYS_sigblock	109

/* syscall: "sigsetmask" ret: "int" args: "int" */
#define	HPUX_SYS_sigsetmask	110

/* syscall: "sigpause" ret: "int" args: "int" */
#define	HPUX_SYS_sigpause	111

/* syscall: "sigstack" ret: "int" args: "struct sigstack *" "struct sigstack *" */
#define	HPUX_SYS_sigstack	112

/* syscall: "gettimeofday" ret: "int" args: "struct timeval *" */
#define	HPUX_SYS_gettimeofday	116

/* syscall: "readv" ret: "int" args: "int" "struct iovec *" "u_int" */
#define	HPUX_SYS_readv	120

/* syscall: "writev" ret: "int" args: "int" "struct iovec *" "u_int" */
#define	HPUX_SYS_writev	121

/* syscall: "settimeofday" ret: "int" args: "struct timeval *" "struct timezone *" */
#define	HPUX_SYS_settimeofday	122

/* syscall: "fchown" ret: "int" args: "int" "int" "int" */
#define	HPUX_SYS_fchown	123

/* syscall: "fchmod" ret: "int" args: "int" "int" */
#define	HPUX_SYS_fchmod	124

/* syscall: "setresuid" ret: "int" args: "uid_t" "uid_t" "uid_t" */
#define	HPUX_SYS_setresuid	126

/* syscall: "setresgid" ret: "int" args: "gid_t" "gid_t" "gid_t" */
#define	HPUX_SYS_setresgid	127

/* syscall: "rename" ret: "int" args: "char *" "char *" */
#define	HPUX_SYS_rename	128

/* syscall: "truncate" ret: "int" args: "char *" "long" */
#define	HPUX_SYS_truncate	129

/* syscall: "ftruncate" ret: "int" args: "int" "long" */
#define	HPUX_SYS_ftruncate	130

/* syscall: "sysconf" ret: "int" args: "int" */
#define	HPUX_SYS_sysconf	132

/* syscall: "mkdir" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_mkdir	136

/* syscall: "rmdir" ret: "int" args: "char *" */
#define	HPUX_SYS_rmdir	137

/* syscall: "getrlimit" ret: "int" args: "u_int" "struct ogetrlimit *" */
#define	HPUX_SYS_getrlimit	144

/* syscall: "setrlimit" ret: "int" args: "u_int" "struct ogetrlimit *" */
#define	HPUX_SYS_setrlimit	145

/* syscall: "rtprio" ret: "int" args: "pid_t" "int" */
#define	HPUX_SYS_rtprio	152

/* syscall: "netioctl" ret: "int" args: "int" "int *" */
#define	HPUX_SYS_netioctl	154

/* syscall: "lockf" ret: "int" args: "int" "int" "long" */
#define	HPUX_SYS_lockf	155

/* syscall: "semget" ret: "int" args: "key_t" "int" "int" */
#define	HPUX_SYS_semget	156

/* syscall: "__semctl" ret: "int" args: "int" "int" "int" "union semun *" */
#define	HPUX_SYS___semctl	157

/* syscall: "semop" ret: "int" args: "int" "struct sembuf *" "u_int" */
#define	HPUX_SYS_semop	158

/* syscall: "msgget" ret: "int" args: "key_t" "int" */
#define	HPUX_SYS_msgget	159

/* syscall: "msgctl" ret: "int" args: "int" "int" "struct msqid_ds *" */
#define	HPUX_SYS_msgctl	160

/* syscall: "msgsnd" ret: "int" args: "int" "void *" "size_t" "int" */
#define	HPUX_SYS_msgsnd	161

/* syscall: "msgrcv" ret: "int" args: "int" "void *" "size_t" "long" "int" */
#define	HPUX_SYS_msgrcv	162

/* syscall: "shmget" ret: "int" args: "key_t" "int" "int" */
#define	HPUX_SYS_shmget	163

/* syscall: "shmctl" ret: "int" args: "int" "int" "caddr_t" */
#define	HPUX_SYS_shmctl	164

/* syscall: "shmat" ret: "int" args: "int" "void *" "int" */
#define	HPUX_SYS_shmat	165

/* syscall: "shmdt" ret: "int" args: "void *" */
#define	HPUX_SYS_shmdt	166

/* syscall: "advise" ret: "int" args: "int" */
#define	HPUX_SYS_advise	167

/* syscall: "getcontext" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_getcontext	174

/* syscall: "getaccess" ret: "int" args: "char *" "uid_t" "int" "gid_t *" "void *" "void *" */
#define	HPUX_SYS_getaccess	190

/* syscall: "waitpid" ret: "int" args: "pid_t" "int *" "int" "struct rusage *" */
#define	HPUX_SYS_waitpid	200

/* syscall: "pathconf" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_pathconf	225

/* syscall: "fpathconf" ret: "int" args: "int" "int" */
#define	HPUX_SYS_fpathconf	226

/* syscall: "getdirentries" ret: "int" args: "int" "char *" "u_int" "long *" */
#define	HPUX_SYS_getdirentries	231

/* syscall: "getdomainname" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_getdomainname	232

/* syscall: "setdomainname" ret: "int" args: "char *" "int" */
#define	HPUX_SYS_setdomainname	236

/* syscall: "sigaction" ret: "int" args: "int" "struct hpux_sigaction *" "struct hpux_sigaction *" */
#define	HPUX_SYS_sigaction	239

/* syscall: "sigprocmask" ret: "int" args: "int" "hpux_sigset_t *" "hpux_sigset_t *" */
#define	HPUX_SYS_sigprocmask	240

/* syscall: "sigpending" ret: "int" args: "hpux_sigset_t *" */
#define	HPUX_SYS_sigpending	241

/* syscall: "sigsuspend" ret: "int" args: "hpux_sigset_t *" */
#define	HPUX_SYS_sigsuspend	242

/* syscall: "getdtablesize" ret: "int" args: */
#define	HPUX_SYS_getdtablesize	268

/* syscall: "fchdir" ret: "int" args: "int" */
#define	HPUX_SYS_fchdir	272

/* syscall: "accept" ret: "int" args: "int" "caddr_t" "int *" */
#define	HPUX_SYS_accept	275

/* syscall: "bind" ret: "int" args: "int" "caddr_t" "int" */
#define	HPUX_SYS_bind	276

/* syscall: "connect" ret: "int" args: "int" "caddr_t" "int" */
#define	HPUX_SYS_connect	277

/* syscall: "getpeername" ret: "int" args: "int" "caddr_t" "int *" */
#define	HPUX_SYS_getpeername	278

/* syscall: "getsockname" ret: "int" args: "int" "caddr_t" "int *" */
#define	HPUX_SYS_getsockname	279

/* syscall: "getsockopt" ret: "int" args: "int" "int" "int" "caddr_t" "int *" */
#define	HPUX_SYS_getsockopt	280

/* syscall: "listen" ret: "int" args: "int" "int" */
#define	HPUX_SYS_listen	281

/* syscall: "recv" ret: "int" args: "int" "caddr_t" "int" "int" */
#define	HPUX_SYS_recv	282

/* syscall: "recvfrom" ret: "int" args: "int" "caddr_t" "size_t" "int" "caddr_t" "int *" */
#define	HPUX_SYS_recvfrom	283

/* syscall: "recvmsg" ret: "int" args: "int" "struct omsghdr *" "int" */
#define	HPUX_SYS_recvmsg	284

/* syscall: "send" ret: "int" args: "int" "caddr_t" "int" "int" */
#define	HPUX_SYS_send	285

/* syscall: "sendmsg" ret: "int" args: "int" "caddr_t" "int" */
#define	HPUX_SYS_sendmsg	286

/* syscall: "sendto" ret: "int" args: "int" "caddr_t" "size_t" "int" "caddr_t" "int" */
#define	HPUX_SYS_sendto	287

/* syscall: "setsockopt2" ret: "int" args: "int" "int" "int" "caddr_t" "int" */
#define	HPUX_SYS_setsockopt2	288

/* syscall: "shutdown" ret: "int" args: "int" "int" */
#define	HPUX_SYS_shutdown	289

/* syscall: "socket" ret: "int" args: "int" "int" "int" */
#define	HPUX_SYS_socket	290

/* syscall: "socketpair" ret: "int" args: "int" "int" "int" "int *" */
#define	HPUX_SYS_socketpair	291

/* syscall: "nsemctl" ret: "int" args: "int" "int" "int" "union semun *" */
#define	HPUX_SYS_nsemctl	312

/* syscall: "nmsgctl" ret: "int" args: "int" "int" "struct msqid_ds *" */
#define	HPUX_SYS_nmsgctl	313

/* syscall: "nshmctl" ret: "int" args: "int" "int" "caddr_t" */
#define	HPUX_SYS_nshmctl	314

#define	HPUX_SYS_MAXSYSCALL	315
