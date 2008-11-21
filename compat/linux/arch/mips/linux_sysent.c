/* $NetBSD: linux_sysent.c,v 1.25 2006/06/10 21:16:50 christos Exp $ */

/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.21 2006/06/10 21:15:33 christos Exp  
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: linux_sysent.c,v 1.25 2006/06/10 21:16:50 christos Exp $");

#if defined(_KERNEL_OPT)
#include "opt_compat_netbsd.h"
#include "opt_compat_43.h"
#endif
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/sa.h>
#include <sys/syscallargs.h>
#include <compat/linux/common/linux_types.h>
#include <compat/linux/common/linux_signal.h>
#include <compat/linux/common/linux_siginfo.h>
#include <compat/linux/common/linux_machdep.h>
#include <compat/linux/common/linux_mmap.h>
#include <compat/linux/common/linux_socketcall.h>
#include <compat/linux/linux_syscallargs.h>

#define	s(type)	sizeof(type)

struct sysent linux_sysent[] = {
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 0 = syscall */
	{ 1, s(struct sys_exit_args), 0,
	    sys_exit },				/* 1 = exit */
	{ 0, 0, 0,
	    sys_fork },				/* 2 = fork */
	{ 3, s(struct sys_read_args), 0,
	    sys_read },				/* 3 = read */
	{ 3, s(struct sys_write_args), 0,
	    sys_write },			/* 4 = write */
	{ 3, s(struct linux_sys_open_args), 0,
	    linux_sys_open },			/* 5 = open */
	{ 1, s(struct sys_close_args), 0,
	    sys_close },			/* 6 = close */
	{ 3, s(struct linux_sys_waitpid_args), 0,
	    linux_sys_waitpid },		/* 7 = waitpid */
	{ 2, s(struct linux_sys_creat_args), 0,
	    linux_sys_creat },			/* 8 = creat */
	{ 2, s(struct linux_sys_link_args), 0,
	    linux_sys_link },			/* 9 = link */
	{ 1, s(struct linux_sys_unlink_args), 0,
	    linux_sys_unlink },			/* 10 = unlink */
	{ 3, s(struct linux_sys_execve_args), 0,
	    linux_sys_execve },			/* 11 = execve */
	{ 1, s(struct linux_sys_chdir_args), 0,
	    linux_sys_chdir },			/* 12 = chdir */
	{ 1, s(struct linux_sys_time_args), 0,
	    linux_sys_time },			/* 13 = time */
	{ 3, s(struct linux_sys_mknod_args), 0,
	    linux_sys_mknod },			/* 14 = mknod */
	{ 2, s(struct linux_sys_chmod_args), 0,
	    linux_sys_chmod },			/* 15 = chmod */
	{ 3, s(struct linux_sys_lchown_args), 0,
	    linux_sys_lchown },			/* 16 = lchown */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 17 = unimplemented */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 18 = obsolete ostat */
	{ 3, s(struct compat_43_sys_lseek_args), 0,
	    compat_43_sys_lseek },		/* 19 = lseek */
	{ 0, 0, SYCALL_MPSAFE | 0,
	    sys_getpid },			/* 20 = getpid */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 21 = unimplemented mount */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 22 = obsolete umount */
	{ 1, s(struct sys_setuid_args), 0,
	    sys_setuid },			/* 23 = setuid */
	{ 0, 0, 0,
	    sys_getuid },			/* 24 = getuid */
	{ 1, s(struct linux_sys_stime_args), 0,
	    linux_sys_stime },			/* 25 = stime */
	{ 4, s(struct linux_sys_ptrace_args), 0,
	    linux_sys_ptrace },			/* 26 = ptrace */
	{ 1, s(struct linux_sys_alarm_args), 0,
	    linux_sys_alarm },			/* 27 = alarm */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 28 = obsolete ofstat */
	{ 0, 0, 0,
	    linux_sys_pause },			/* 29 = pause */
	{ 2, s(struct linux_sys_utime_args), 0,
	    linux_sys_utime },			/* 30 = utime */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 31 = unimplemented */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 32 = unimplemented */
	{ 2, s(struct linux_sys_access_args), 0,
	    linux_sys_access },			/* 33 = access */
	{ 1, s(struct linux_sys_nice_args), 0,
	    linux_sys_nice },			/* 34 = nice */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 35 = unimplemented */
	{ 0, 0, 0,
	    sys_sync },				/* 36 = sync */
	{ 2, s(struct linux_sys_kill_args), 0,
	    linux_sys_kill },			/* 37 = kill */
	{ 2, s(struct linux_sys_rename_args), 0,
	    linux_sys_rename },			/* 38 = rename */
	{ 2, s(struct linux_sys_mkdir_args), 0,
	    linux_sys_mkdir },			/* 39 = mkdir */
	{ 1, s(struct linux_sys_rmdir_args), 0,
	    linux_sys_rmdir },			/* 40 = rmdir */
	{ 1, s(struct sys_dup_args), 0,
	    sys_dup },				/* 41 = dup */
	{ 1, s(struct linux_sys_pipe_args), 0,
	    linux_sys_pipe },			/* 42 = pipe */
	{ 1, s(struct linux_sys_times_args), 0,
	    linux_sys_times },			/* 43 = times */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 44 = unimplemented */
	{ 1, s(struct linux_sys_brk_args), 0,
	    linux_sys_brk },			/* 45 = brk */
	{ 1, s(struct sys_setgid_args), 0,
	    sys_setgid },			/* 46 = setgid */
	{ 0, 0, 0,
	    sys_getgid },			/* 47 = getgid */
	{ 2, s(struct linux_sys_signal_args), 0,
	    linux_sys_signal },			/* 48 = signal */
	{ 0, 0, 0,
	    sys_geteuid },			/* 49 = geteuid */
	{ 0, 0, 0,
	    sys_getegid },			/* 50 = getegid */
	{ 1, s(struct sys_acct_args), 0,
	    sys_acct },				/* 51 = acct */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 52 = unimplemented umount */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 53 = unimplemented */
	{ 3, s(struct linux_sys_ioctl_args), 0,
	    linux_sys_ioctl },			/* 54 = ioctl */
	{ 3, s(struct linux_sys_fcntl_args), 0,
	    linux_sys_fcntl },			/* 55 = fcntl */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 56 = obsolete mpx */
	{ 2, s(struct sys_setpgid_args), 0,
	    sys_setpgid },			/* 57 = setpgid */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 58 = unimplemented */
	{ 1, s(struct linux_sys_olduname_args), 0,
	    linux_sys_olduname },		/* 59 = olduname */
	{ 1, s(struct sys_umask_args), 0,
	    sys_umask },			/* 60 = umask */
	{ 1, s(struct sys_chroot_args), 0,
	    sys_chroot },			/* 61 = chroot */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 62 = unimplemented ustat */
	{ 2, s(struct sys_dup2_args), 0,
	    sys_dup2 },				/* 63 = dup2 */
	{ 0, 0, 0,
	    sys_getppid },			/* 64 = getppid */
	{ 0, 0, 0,
	    sys_getpgrp },			/* 65 = getpgrp */
	{ 0, 0, 0,
	    sys_setsid },			/* 66 = setsid */
	{ 3, s(struct linux_sys_sigaction_args), 0,
	    linux_sys_sigaction },		/* 67 = sigaction */
	{ 0, 0, 0,
	    linux_sys_siggetmask },		/* 68 = siggetmask */
	{ 1, s(struct linux_sys_sigsetmask_args), 0,
	    linux_sys_sigsetmask },		/* 69 = sigsetmask */
	{ 2, s(struct sys_setreuid_args), 0,
	    sys_setreuid },			/* 70 = setreuid */
	{ 2, s(struct sys_setregid_args), 0,
	    sys_setregid },			/* 71 = setregid */
	{ 3, s(struct linux_sys_sigsuspend_args), 0,
	    linux_sys_sigsuspend },		/* 72 = sigsuspend */
	{ 1, s(struct linux_sys_sigpending_args), 0,
	    linux_sys_sigpending },		/* 73 = sigpending */
	{ 2, s(struct compat_43_sys_sethostname_args), 0,
	    compat_43_sys_sethostname },	/* 74 = sethostname */
	{ 2, s(struct linux_sys_setrlimit_args), 0,
	    linux_sys_setrlimit },		/* 75 = setrlimit */
	{ 2, s(struct linux_sys_getrlimit_args), 0,
	    linux_sys_getrlimit },		/* 76 = getrlimit */
	{ 2, s(struct sys_getrusage_args), 0,
	    sys_getrusage },			/* 77 = getrusage */
	{ 2, s(struct linux_sys_gettimeofday_args), 0,
	    linux_sys_gettimeofday },		/* 78 = gettimeofday */
	{ 2, s(struct linux_sys_settimeofday_args), 0,
	    linux_sys_settimeofday },		/* 79 = settimeofday */
	{ 2, s(struct sys_getgroups_args), 0,
	    sys_getgroups },			/* 80 = getgroups */
	{ 2, s(struct sys_setgroups_args), 0,
	    sys_setgroups },			/* 81 = setgroups */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 82 = unimplemented old_select */
	{ 2, s(struct linux_sys_symlink_args), 0,
	    linux_sys_symlink },		/* 83 = symlink */
	{ 2, s(struct compat_43_sys_lstat_args), 0,
	    compat_43_sys_lstat },		/* 84 = oolstat */
	{ 3, s(struct linux_sys_readlink_args), 0,
	    linux_sys_readlink },		/* 85 = readlink */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 86 = unimplemented uselib */
	{ 1, s(struct linux_sys_swapon_args), 0,
	    linux_sys_swapon },			/* 87 = swapon */
	{ 4, s(struct linux_sys_reboot_args), 0,
	    linux_sys_reboot },			/* 88 = reboot */
	{ 3, s(struct linux_sys_readdir_args), 0,
	    linux_sys_readdir },		/* 89 = readdir */
	{ 6, s(struct linux_sys_mmap_args), 0,
	    linux_sys_mmap },			/* 90 = mmap */
	{ 2, s(struct sys_munmap_args), 0,
	    sys_munmap },			/* 91 = munmap */
	{ 2, s(struct linux_sys_truncate_args), 0,
	    linux_sys_truncate },		/* 92 = truncate */
	{ 2, s(struct compat_43_sys_ftruncate_args), 0,
	    compat_43_sys_ftruncate },		/* 93 = ftruncate */
	{ 2, s(struct sys_fchmod_args), 0,
	    sys_fchmod },			/* 94 = fchmod */
	{ 3, s(struct sys___posix_fchown_args), 0,
	    sys___posix_fchown },		/* 95 = __posix_fchown */
	{ 2, s(struct linux_sys_getpriority_args), 0,
	    linux_sys_getpriority },		/* 96 = getpriority */
	{ 3, s(struct sys_setpriority_args), 0,
	    sys_setpriority },			/* 97 = setpriority */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 98 = unimplemented */
	{ 2, s(struct linux_sys_statfs_args), 0,
	    linux_sys_statfs },			/* 99 = statfs */
	{ 2, s(struct linux_sys_fstatfs_args), 0,
	    linux_sys_fstatfs },		/* 100 = fstatfs */
	{ 3, s(struct linux_sys_ioperm_args), 0,
	    linux_sys_ioperm },			/* 101 = ioperm */
	{ 2, s(struct linux_sys_socketcall_args), 0,
	    linux_sys_socketcall },		/* 102 = socketcall */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 103 = unimplemented syslog */
	{ 3, s(struct sys_setitimer_args), 0,
	    sys_setitimer },			/* 104 = setitimer */
	{ 2, s(struct sys_getitimer_args), 0,
	    sys_getitimer },			/* 105 = getitimer */
	{ 2, s(struct linux_sys_stat_args), 0,
	    linux_sys_stat },			/* 106 = stat */
	{ 2, s(struct linux_sys_lstat_args), 0,
	    linux_sys_lstat },			/* 107 = lstat */
	{ 2, s(struct linux_sys_fstat_args), 0,
	    linux_sys_fstat },			/* 108 = fstat */
	{ 1, s(struct linux_sys_uname_args), 0,
	    linux_sys_uname },			/* 109 = uname */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 110 = unimplemented iopl */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 111 = unimplemented vhangup */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 112 = unimplemented idle */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 113 = unimplemented vm86old */
	{ 4, s(struct linux_sys_wait4_args), 0,
	    linux_sys_wait4 },			/* 114 = wait4 */
	{ 1, s(struct linux_sys_swapoff_args), 0,
	    linux_sys_swapoff },		/* 115 = swapoff */
	{ 1, s(struct linux_sys_sysinfo_args), 0,
	    linux_sys_sysinfo },		/* 116 = sysinfo */
	{ 5, s(struct linux_sys_ipc_args), 0,
	    linux_sys_ipc },			/* 117 = ipc */
	{ 1, s(struct sys_fsync_args), 0,
	    sys_fsync },			/* 118 = fsync */
	{ 1, s(struct linux_sys_sigreturn_args), 0,
	    linux_sys_sigreturn },		/* 119 = sigreturn */
	{ 2, s(struct linux_sys_clone_args), 0,
	    linux_sys_clone },			/* 120 = clone */
	{ 2, s(struct linux_sys_setdomainname_args), 0,
	    linux_sys_setdomainname },		/* 121 = setdomainname */
	{ 1, s(struct linux_sys_new_uname_args), 0,
	    linux_sys_new_uname },		/* 122 = new_uname */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 123 = unimplemented modify_ldt */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 124 = unimplemented adjtimex */
	{ 3, s(struct linux_sys_mprotect_args), 0,
	    linux_sys_mprotect },		/* 125 = mprotect */
	{ 3, s(struct linux_sys_sigprocmask_args), 0,
	    linux_sys_sigprocmask },		/* 126 = sigprocmask */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 127 = unimplemented create_module */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 128 = unimplemented init_module */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 129 = unimplemented delete_module */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 130 = unimplemented get_kernel_syms */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 131 = unimplemented quotactl */
	{ 1, s(struct linux_sys_getpgid_args), 0,
	    linux_sys_getpgid },		/* 132 = getpgid */
	{ 1, s(struct sys_fchdir_args), 0,
	    sys_fchdir },			/* 133 = fchdir */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 134 = unimplemented bdflush */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 135 = unimplemented sysfs */
	{ 1, s(struct linux_sys_personality_args), 0,
	    linux_sys_personality },		/* 136 = personality */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 137 = unimplemented afs_syscall */
	{ 1, s(struct linux_sys_setfsuid_args), 0,
	    linux_sys_setfsuid },		/* 138 = setfsuid */
	{ 0, 0, 0,
	    linux_sys_getfsuid },		/* 139 = getfsuid */
	{ 5, s(struct linux_sys_llseek_args), 0,
	    linux_sys_llseek },			/* 140 = llseek */
	{ 3, s(struct linux_sys_getdents_args), 0,
	    linux_sys_getdents },		/* 141 = getdents */
	{ 5, s(struct linux_sys_select_args), 0,
	    linux_sys_select },			/* 142 = select */
	{ 2, s(struct sys_flock_args), 0,
	    sys_flock },			/* 143 = flock */
	{ 3, s(struct linux_sys_msync_args), 0,
	    linux_sys_msync },			/* 144 = msync */
	{ 3, s(struct sys_readv_args), 0,
	    sys_readv },			/* 145 = readv */
	{ 3, s(struct sys_writev_args), 0,
	    sys_writev },			/* 146 = writev */
	{ 3, s(struct linux_sys_cacheflush_args), 0,
	    linux_sys_cacheflush },		/* 147 = cacheflush */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 148 = unimplemented cachectl */
	{ 4, s(struct linux_sys_sysmips_args), 0,
	    linux_sys_sysmips },		/* 149 = sysmips */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 150 = unimplemented */
	{ 1, s(struct sys_getsid_args), 0,
	    sys_getsid },			/* 151 = getsid */
	{ 1, s(struct linux_sys_fdatasync_args), 0,
	    linux_sys_fdatasync },		/* 152 = fdatasync */
	{ 1, s(struct linux_sys___sysctl_args), 0,
	    linux_sys___sysctl },		/* 153 = __sysctl */
	{ 2, s(struct sys_mlock_args), 0,
	    sys_mlock },			/* 154 = mlock */
	{ 2, s(struct sys_munlock_args), 0,
	    sys_munlock },			/* 155 = munlock */
	{ 1, s(struct sys_mlockall_args), 0,
	    sys_mlockall },			/* 156 = mlockall */
	{ 0, 0, 0,
	    sys_munlockall },			/* 157 = munlockall */
	{ 2, s(struct linux_sys_sched_setparam_args), 0,
	    linux_sys_sched_setparam },		/* 158 = sched_setparam */
	{ 2, s(struct linux_sys_sched_getparam_args), 0,
	    linux_sys_sched_getparam },		/* 159 = sched_getparam */
	{ 3, s(struct linux_sys_sched_setscheduler_args), 0,
	    linux_sys_sched_setscheduler },	/* 160 = sched_setscheduler */
	{ 1, s(struct linux_sys_sched_getscheduler_args), 0,
	    linux_sys_sched_getscheduler },	/* 161 = sched_getscheduler */
	{ 0, 0, 0,
	    linux_sys_sched_yield },		/* 162 = sched_yield */
	{ 1, s(struct linux_sys_sched_get_priority_max_args), 0,
	    linux_sys_sched_get_priority_max },	/* 163 = sched_get_priority_max */
	{ 1, s(struct linux_sys_sched_get_priority_min_args), 0,
	    linux_sys_sched_get_priority_min },	/* 164 = sched_get_priority_min */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 165 = unimplemented sched_rr_get_interval */
	{ 2, s(struct sys_nanosleep_args), 0,
	    sys_nanosleep },			/* 166 = nanosleep */
	{ 4, s(struct linux_sys_mremap_args), 0,
	    linux_sys_mremap },			/* 167 = mremap */
	{ 3, s(struct linux_sys_accept_args), 0,
	    linux_sys_accept },			/* 168 = accept */
	{ 3, s(struct linux_sys_bind_args), 0,
	    linux_sys_bind },			/* 169 = bind */
	{ 3, s(struct linux_sys_connect_args), 0,
	    linux_sys_connect },		/* 170 = connect */
	{ 3, s(struct linux_sys_getpeername_args), 0,
	    linux_sys_getpeername },		/* 171 = getpeername */
	{ 3, s(struct linux_sys_getsockname_args), 0,
	    linux_sys_getsockname },		/* 172 = getsockname */
	{ 5, s(struct linux_sys_getsockopt_args), 0,
	    linux_sys_getsockopt },		/* 173 = getsockopt */
	{ 2, s(struct sys_listen_args), 0,
	    sys_listen },			/* 174 = listen */
	{ 4, s(struct linux_sys_recv_args), 0,
	    linux_sys_recv },			/* 175 = recv */
	{ 6, s(struct linux_sys_recvfrom_args), 0,
	    linux_sys_recvfrom },		/* 176 = recvfrom */
	{ 3, s(struct linux_sys_recvmsg_args), 0,
	    linux_sys_recvmsg },		/* 177 = recvmsg */
	{ 4, s(struct linux_sys_send_args), 0,
	    linux_sys_send },			/* 178 = send */
	{ 3, s(struct linux_sys_sendmsg_args), 0,
	    linux_sys_sendmsg },		/* 179 = sendmsg */
	{ 6, s(struct linux_sys_sendto_args), 0,
	    linux_sys_sendto },			/* 180 = sendto */
	{ 5, s(struct linux_sys_setsockopt_args), 0,
	    linux_sys_setsockopt },		/* 181 = setsockopt */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 182 = unimplemented shutdown */
	{ 3, s(struct linux_sys_socket_args), 0,
	    linux_sys_socket },			/* 183 = socket */
	{ 4, s(struct linux_sys_socketpair_args), 0,
	    linux_sys_socketpair },		/* 184 = socketpair */
	{ 3, s(struct linux_sys_setresuid_args), 0,
	    linux_sys_setresuid },		/* 185 = setresuid */
	{ 3, s(struct linux_sys_getresuid_args), 0,
	    linux_sys_getresuid },		/* 186 = getresuid */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 187 = unimplemented query_module */
	{ 3, s(struct sys_poll_args), 0,
	    sys_poll },				/* 188 = poll */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 189 = unimplemented nfsservctl */
	{ 3, s(struct linux_sys_setresgid_args), 0,
	    linux_sys_setresgid },		/* 190 = setresgid */
	{ 3, s(struct linux_sys_getresgid_args), 0,
	    linux_sys_getresgid },		/* 191 = getresgid */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 192 = unimplemented prctl */
	{ 1, s(struct linux_sys_rt_sigreturn_args), 0,
	    linux_sys_rt_sigreturn },		/* 193 = rt_sigreturn */
	{ 4, s(struct linux_sys_rt_sigaction_args), 0,
	    linux_sys_rt_sigaction },		/* 194 = rt_sigaction */
	{ 4, s(struct linux_sys_rt_sigprocmask_args), 0,
	    linux_sys_rt_sigprocmask },		/* 195 = rt_sigprocmask */
	{ 2, s(struct linux_sys_rt_sigpending_args), 0,
	    linux_sys_rt_sigpending },		/* 196 = rt_sigpending */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 197 = unimplemented rt_sigtimedwait */
	{ 3, s(struct linux_sys_rt_queueinfo_args), 0,
	    linux_sys_rt_queueinfo },		/* 198 = rt_queueinfo */
	{ 2, s(struct linux_sys_rt_sigsuspend_args), 0,
	    linux_sys_rt_sigsuspend },		/* 199 = rt_sigsuspend */
	{ 4, s(struct linux_sys_pread_args), 0,
	    linux_sys_pread },			/* 200 = pread */
	{ 4, s(struct linux_sys_pwrite_args), 0,
	    linux_sys_pwrite },			/* 201 = pwrite */
	{ 3, s(struct linux_sys_chown_args), 0,
	    linux_sys_chown },			/* 202 = chown */
	{ 2, s(struct sys___getcwd_args), 0,
	    sys___getcwd },			/* 203 = __getcwd */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 204 = unimplemented capget */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 205 = unimplemented capset */
	{ 2, s(struct linux_sys_sigaltstack_args), 0,
	    linux_sys_sigaltstack },		/* 206 = sigaltstack */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 207 = unimplemented sendfile */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 208 = unimplemented */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 209 = unimplemented */
	{ 6, s(struct linux_sys_mmap2_args), 0,
	    linux_sys_mmap2 },			/* 210 = mmap2 */
	{ 2, s(struct linux_sys_truncate64_args), 0,
	    linux_sys_truncate64 },		/* 211 = truncate64 */
	{ 2, s(struct linux_sys_ftruncate64_args), 0,
	    linux_sys_ftruncate64 },		/* 212 = ftruncate64 */
	{ 2, s(struct linux_sys_stat64_args), 0,
	    linux_sys_stat64 },			/* 213 = stat64 */
	{ 2, s(struct linux_sys_lstat64_args), 0,
	    linux_sys_lstat64 },		/* 214 = lstat64 */
	{ 2, s(struct linux_sys_fstat64_args), 0,
	    linux_sys_fstat64 },		/* 215 = fstat64 */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 216 = unimplemented pivot_root */
	{ 3, s(struct sys_mincore_args), 0,
	    sys_mincore },			/* 217 = mincore */
	{ 3, s(struct sys_madvise_args), 0,
	    sys_madvise },			/* 218 = madvise */
	{ 3, s(struct linux_sys_getdents64_args), 0,
	    linux_sys_getdents64 },		/* 219 = getdents64 */
	{ 3, s(struct linux_sys_fcntl64_args), 0,
	    linux_sys_fcntl64 },		/* 220 = fcntl64 */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 221 = unimplemented / * reserved * / */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 222 = unimplemented gettid */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 223 = unimplemented readahead */
	{ 5, s(struct linux_sys_setxattr_args), 0,
	    linux_sys_setxattr },		/* 224 = setxattr */
	{ 5, s(struct linux_sys_lsetxattr_args), 0,
	    linux_sys_lsetxattr },		/* 225 = lsetxattr */
	{ 5, s(struct linux_sys_fsetxattr_args), 0,
	    linux_sys_fsetxattr },		/* 226 = fsetxattr */
	{ 4, s(struct linux_sys_getxattr_args), 0,
	    linux_sys_getxattr },		/* 227 = getxattr */
	{ 4, s(struct linux_sys_lgetxattr_args), 0,
	    linux_sys_lgetxattr },		/* 228 = lgetxattr */
	{ 4, s(struct linux_sys_fgetxattr_args), 0,
	    linux_sys_fgetxattr },		/* 229 = fgetxattr */
	{ 3, s(struct linux_sys_listxattr_args), 0,
	    linux_sys_listxattr },		/* 230 = listxattr */
	{ 3, s(struct linux_sys_llistxattr_args), 0,
	    linux_sys_llistxattr },		/* 231 = llistxattr */
	{ 3, s(struct linux_sys_flistxattr_args), 0,
	    linux_sys_flistxattr },		/* 232 = flistxattr */
	{ 2, s(struct linux_sys_removexattr_args), 0,
	    linux_sys_removexattr },		/* 233 = removexattr */
	{ 2, s(struct linux_sys_lremovexattr_args), 0,
	    linux_sys_lremovexattr },		/* 234 = lremovexattr */
	{ 2, s(struct linux_sys_fremovexattr_args), 0,
	    linux_sys_fremovexattr },		/* 235 = fremovexattr */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 236 = unimplemented tkill */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 237 = unimplemented sendfile64 */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 238 = unimplemented futex */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 239 = unimplemented sched_setaffinity */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 240 = unimplemented sched_getaffinity */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 241 = unimplemented io_setup */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 242 = unimplemented io_destroy */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 243 = unimplemented io_getevents */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 244 = unimplemented io_submit */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 245 = unimplemented io_cancel */
	{ 1, s(struct linux_sys_exit_group_args), 0,
	    linux_sys_exit_group },		/* 246 = exit_group */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 247 = unimplemented lookup_dcookie */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 248 = unimplemented epoll_create */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 249 = unimplemented epoll_ctl */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 250 = unimplemented epoll_wait */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 251 = unimplemented remap_file_pages */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 252 = unimplemented set_tid_address */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 253 = unimplemented restart_syscall */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 254 = unimplemented fadvise64 */
	{ 3, s(struct linux_sys_statfs64_args), 0,
	    linux_sys_statfs64 },		/* 255 = statfs64 */
	{ 3, s(struct linux_sys_fstatfs64_args), 0,
	    linux_sys_fstatfs64 },		/* 256 = fstatfs64 */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 257 = unimplemented timer_create */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 258 = unimplemented timer_settime */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 259 = unimplemented timer_gettime */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 260 = unimplemented timer_getoverrun */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 261 = unimplemented timer_delete */
	{ 2, s(struct linux_sys_clock_settime_args), 0,
	    linux_sys_clock_settime },		/* 262 = clock_settime */
	{ 2, s(struct linux_sys_clock_gettime_args), 0,
	    linux_sys_clock_gettime },		/* 263 = clock_gettime */
	{ 2, s(struct linux_sys_clock_getres_args), 0,
	    linux_sys_clock_getres },		/* 264 = clock_getres */
	{ 4, s(struct linux_sys_clock_nanosleep_args), 0,
	    linux_sys_clock_nanosleep },	/* 265 = clock_nanosleep */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 266 = unimplemented tgkill */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 267 = unimplemented utimes */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 268 = unimplemented mbind */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 269 = unimplemented get_mempolicy */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 270 = unimplemented set_mempolicy */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 271 = unimplemented mq_open */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 272 = unimplemented mq_unlink */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 273 = unimplemented mq_timedsend */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 274 = unimplemented mq_timedreceive */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 275 = unimplemented mq_notify */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 276 = unimplemented mq_getsetattr */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 277 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 278 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 279 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 280 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 281 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 282 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 283 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 284 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 285 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 286 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 287 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 288 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 289 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 290 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 291 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 292 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 293 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 294 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 295 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 296 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 297 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 298 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 299 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 300 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 301 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 302 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 303 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 304 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 305 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 306 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 307 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 308 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 309 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 310 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 311 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 312 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 313 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 314 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 315 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 316 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 317 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 318 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 319 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 320 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 321 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 322 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 323 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 324 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 325 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 326 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 327 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 328 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 329 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 330 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 331 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 332 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 333 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 334 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 335 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 336 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 337 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 338 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 339 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 340 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 341 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 342 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 343 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 344 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 345 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 346 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 347 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 348 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 349 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 350 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 351 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 352 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 353 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 354 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 355 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 356 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 357 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 358 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 359 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 360 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 361 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 362 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 363 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 364 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 365 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 366 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 367 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 368 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 369 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 370 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 371 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 372 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 373 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 374 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 375 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 376 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 377 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 378 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 379 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 380 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 381 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 382 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 383 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 384 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 385 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 386 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 387 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 388 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 389 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 390 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 391 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 392 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 393 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 394 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 395 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 396 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 397 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 398 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 399 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 400 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 401 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 402 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 403 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 404 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 405 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 406 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 407 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 408 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 409 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 410 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 411 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 412 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 413 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 414 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 415 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 416 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 417 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 418 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 419 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 420 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 421 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 422 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 423 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 424 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 425 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 426 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 427 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 428 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 429 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 430 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 431 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 432 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 433 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 434 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 435 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 436 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 437 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 438 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 439 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 440 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 441 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 442 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 443 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 444 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 445 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 446 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 447 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 448 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 449 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 450 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 451 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 452 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 453 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 454 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 455 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 456 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 457 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 458 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 459 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 460 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 461 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 462 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 463 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 464 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 465 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 466 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 467 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 468 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 469 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 470 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 471 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 472 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 473 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 474 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 475 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 476 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 477 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 478 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 479 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 480 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 481 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 482 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 483 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 484 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 485 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 486 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 487 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 488 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 489 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 490 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 491 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 492 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 493 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 494 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 495 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 496 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 497 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 498 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 499 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 500 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 501 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 502 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 503 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 504 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 505 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 506 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 507 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 508 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 509 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 510 = filler */
	{ 0, 0, 0,
	    linux_sys_nosys },			/* 511 = filler */
};

