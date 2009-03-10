/* $NetBSD: osf1_syscallargs.h,v 1.60 2009/01/13 22:33:17 pooka Exp $ */

/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.48 2009/01/13 22:27:43 pooka Exp
 */

#ifndef _OSF1_SYS_SYSCALLARGS_H_
#define	_OSF1_SYS_SYSCALLARGS_H_

#define	OSF1_SYS_MAXSYSARGS	8

#undef	syscallarg
#define	syscallarg(x)							\
	union {								\
		register_t pad;						\
		struct { x datum; } le;					\
		struct { /* LINTED zero array dimension */		\
			int8_t pad[  /* CONSTCOND */			\
				(sizeof (register_t) < sizeof (x))	\
				? 0					\
				: sizeof (register_t) - sizeof (x)];	\
			x datum;					\
		} be;							\
	}

#undef check_syscall_args
#define check_syscall_args(call) \
	typedef char call##_check_args[sizeof (struct call##_args) \
		<= OSF1_SYS_MAXSYSARGS * sizeof (register_t) ? 1 : -1];

struct sys_exit_args;

struct sys_read_args;

struct sys_write_args;

struct sys_close_args;

struct osf1_sys_wait4_args {
	syscallarg(int) pid;
	syscallarg(int *) status;
	syscallarg(int) options;
	syscallarg(struct osf1_rusage *) rusage;
};
check_syscall_args(osf1_sys_wait4)

struct sys_link_args;

struct sys_unlink_args;

struct sys_chdir_args;

struct sys_fchdir_args;

struct osf1_sys_mknod_args {
	syscallarg(const char *) path;
	syscallarg(int) mode;
	syscallarg(int) dev;
};
check_syscall_args(osf1_sys_mknod)

struct sys_chmod_args;

struct sys___posix_chown_args;

struct sys_obreak_args;

struct osf1_sys_getfsstat_args {
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(long) bufsize;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_getfsstat)

struct osf1_sys_lseek_args {
	syscallarg(int) fd;
	syscallarg(off_t) offset;
	syscallarg(int) whence;
};
check_syscall_args(osf1_sys_lseek)

struct osf1_sys_mount_args {
	syscallarg(int) type;
	syscallarg(const char *) path;
	syscallarg(int) flags;
	syscallarg(void *) data;
};
check_syscall_args(osf1_sys_mount)

struct osf1_sys_unmount_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_unmount)

struct osf1_sys_setuid_args {
	syscallarg(uid_t) uid;
};
check_syscall_args(osf1_sys_setuid)

struct osf1_sys_recvmsg_xopen_args {
	syscallarg(int) s;
	syscallarg(struct osf1_msghdr_xopen *) msg;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_recvmsg_xopen)

struct osf1_sys_sendmsg_xopen_args {
	syscallarg(int) s;
	syscallarg(const struct osf1_msghdr_xopen *) msg;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_sendmsg_xopen)

struct osf1_sys_access_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_access)

struct sys_kill_args;

struct sys_setpgid_args;

struct sys_dup_args;

struct osf1_sys_set_program_attributes_args {
	syscallarg(void *) taddr;
	syscallarg(unsigned long) tsize;
	syscallarg(void *) daddr;
	syscallarg(unsigned long) dsize;
};
check_syscall_args(osf1_sys_set_program_attributes)

struct osf1_sys_open_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
	syscallarg(int) mode;
};
check_syscall_args(osf1_sys_open)

struct compat_13_sys_sigprocmask_args;

struct sys___getlogin_args;

struct sys___setlogin_args;

struct sys_acct_args;

struct osf1_sys_classcntl_args {
	syscallarg(int) opcode;
	syscallarg(int) arg1;
	syscallarg(int) arg2;
	syscallarg(int) arg3;
};
check_syscall_args(osf1_sys_classcntl)

struct osf1_sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(int) com;
	syscallarg(void *) data;
};
check_syscall_args(osf1_sys_ioctl)

struct osf1_sys_reboot_args {
	syscallarg(int) opt;
};
check_syscall_args(osf1_sys_reboot)

struct sys_revoke_args;

struct sys_symlink_args;

struct sys_readlink_args;

struct osf1_sys_execve_args {
	syscallarg(const char *) path;
	syscallarg(char *const *) argp;
	syscallarg(char *const *) envp;
};
check_syscall_args(osf1_sys_execve)

struct sys_umask_args;

struct sys_chroot_args;

struct osf1_sys_stat_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_stat *) ub;
};
check_syscall_args(osf1_sys_stat)

struct osf1_sys_lstat_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_stat *) ub;
};
check_syscall_args(osf1_sys_lstat)

struct osf1_sys_mmap_args {
	syscallarg(void *) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
	syscallarg(int) flags;
	syscallarg(int) fd;
	syscallarg(off_t) pos;
};
check_syscall_args(osf1_sys_mmap)

struct sys_munmap_args;

struct osf1_sys_mprotect_args {
	syscallarg(void *) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
};
check_syscall_args(osf1_sys_mprotect)

struct osf1_sys_madvise_args {
	syscallarg(void *) addr;
	syscallarg(size_t) len;
	syscallarg(int) behav;
};
check_syscall_args(osf1_sys_madvise)

struct sys_getgroups_args;

struct sys_setgroups_args;

struct sys_setpgid_args;

struct osf1_sys_setitimer_args {
	syscallarg(u_int) which;
	syscallarg(struct osf1_itimerval *) itv;
	syscallarg(struct osf1_itimerval *) oitv;
};
check_syscall_args(osf1_sys_setitimer)

struct osf1_sys_getitimer_args {
	syscallarg(u_int) which;
	syscallarg(struct osf1_itimerval *) itv;
};
check_syscall_args(osf1_sys_getitimer)

struct compat_43_sys_gethostname_args;

struct compat_43_sys_sethostname_args;

struct sys_dup2_args;

struct osf1_sys_fstat_args {
	syscallarg(int) fd;
	syscallarg(void *) sb;
};
check_syscall_args(osf1_sys_fstat)

struct osf1_sys_fcntl_args {
	syscallarg(int) fd;
	syscallarg(int) cmd;
	syscallarg(void *) arg;
};
check_syscall_args(osf1_sys_fcntl)

struct osf1_sys_select_args {
	syscallarg(u_int) nd;
	syscallarg(fd_set *) in;
	syscallarg(fd_set *) ou;
	syscallarg(fd_set *) ex;
	syscallarg(struct osf1_timeval *) tv;
};
check_syscall_args(osf1_sys_select)

struct sys_poll_args;

struct sys_fsync_args;

struct sys_setpriority_args;

struct osf1_sys_socket_args {
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
};
check_syscall_args(osf1_sys_socket)

struct sys_connect_args;

struct compat_43_sys_accept_args;

struct sys_getpriority_args;

struct compat_43_sys_send_args;

struct compat_43_sys_recv_args;

struct compat_13_sys_sigreturn_args;

struct sys_bind_args;

struct sys_setsockopt_args;

struct sys_listen_args;

struct compat_13_sys_sigsuspend_args;

struct compat_43_sys_sigstack_args;

struct osf1_sys_gettimeofday_args {
	syscallarg(struct osf1_timeval *) tp;
	syscallarg(struct osf1_timezone *) tzp;
};
check_syscall_args(osf1_sys_gettimeofday)

struct osf1_sys_getrusage_args {
	syscallarg(int) who;
	syscallarg(struct osf1_rusage *) rusage;
};
check_syscall_args(osf1_sys_getrusage)

struct sys_getsockopt_args;

struct osf1_sys_readv_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_iovec *) iovp;
	syscallarg(u_int) iovcnt;
};
check_syscall_args(osf1_sys_readv)

struct osf1_sys_writev_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_iovec *) iovp;
	syscallarg(u_int) iovcnt;
};
check_syscall_args(osf1_sys_writev)

struct osf1_sys_settimeofday_args {
	syscallarg(struct osf1_timeval *) tv;
	syscallarg(struct osf1_timezone *) tzp;
};
check_syscall_args(osf1_sys_settimeofday)

struct sys___posix_fchown_args;

struct sys_fchmod_args;

struct compat_43_sys_recvfrom_args;

struct sys_setreuid_args;

struct sys_setregid_args;

struct sys___posix_rename_args;

struct osf1_sys_truncate_args {
	syscallarg(const char *) path;
	syscallarg(off_t) length;
};
check_syscall_args(osf1_sys_truncate)

struct osf1_sys_ftruncate_args {
	syscallarg(int) fd;
	syscallarg(off_t) length;
};
check_syscall_args(osf1_sys_ftruncate)

struct sys_flock_args;

struct osf1_sys_setgid_args {
	syscallarg(gid_t) gid;
};
check_syscall_args(osf1_sys_setgid)

struct osf1_sys_sendto_args {
	syscallarg(int) s;
	syscallarg(void *) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(struct sockaddr *) to;
	syscallarg(int) tolen;
};
check_syscall_args(osf1_sys_sendto)

struct sys_shutdown_args;

struct osf1_sys_socketpair_args {
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
	syscallarg(int *) rsv;
};
check_syscall_args(osf1_sys_socketpair)

struct sys_mkdir_args;

struct sys_rmdir_args;

struct osf1_sys_utimes_args {
	syscallarg(const char *) path;
	syscallarg(const struct osf1_timeval *) tptr;
};
check_syscall_args(osf1_sys_utimes)

struct compat_43_sys_getpeername_args;

struct compat_43_sys_sethostid_args;

struct osf1_sys_getrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct rlimit *) rlp;
};
check_syscall_args(osf1_sys_getrlimit)

struct osf1_sys_setrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct rlimit *) rlp;
};
check_syscall_args(osf1_sys_setrlimit)

struct compat_43_sys_getsockname_args;

struct osf1_sys_sigaction_args {
	syscallarg(int) signum;
	syscallarg(struct osf1_sigaction *) nsa;
	syscallarg(struct osf1_sigaction *) osa;
};
check_syscall_args(osf1_sys_sigaction)

struct osf1_sys_getdirentries_args {
	syscallarg(int) fd;
	syscallarg(char *) buf;
	syscallarg(int) nbytes;
	syscallarg(long *) basep;
};
check_syscall_args(osf1_sys_getdirentries)

struct osf1_sys_statfs_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(int) len;
};
check_syscall_args(osf1_sys_statfs)

struct osf1_sys_fstatfs_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(int) len;
};
check_syscall_args(osf1_sys_fstatfs)

struct compat_09_sys_getdomainname_args;

struct compat_09_sys_setdomainname_args;

struct osf1_sys_uname_args {
	syscallarg(struct osf1_uname *) name;
};
check_syscall_args(osf1_sys_uname)

struct sys___posix_lchown_args;

struct osf1_sys_shmat_args {
	syscallarg(int) shmid;
	syscallarg(const void *) shmaddr;
	syscallarg(int) shmflg;
};
check_syscall_args(osf1_sys_shmat)

struct osf1_sys_shmctl_args {
	syscallarg(int) shmid;
	syscallarg(int) cmd;
	syscallarg(struct osf1_shmid_ds *) buf;
};
check_syscall_args(osf1_sys_shmctl)

struct osf1_sys_shmdt_args {
	syscallarg(const void *) shmaddr;
};
check_syscall_args(osf1_sys_shmdt)

struct osf1_sys_shmget_args {
	syscallarg(osf1_key_t) key;
	syscallarg(size_t) size;
	syscallarg(int) flags;
};
check_syscall_args(osf1_sys_shmget)

struct osf1_sys_stat2_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_stat2 *) ub;
};
check_syscall_args(osf1_sys_stat2)

struct osf1_sys_lstat2_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_stat2 *) ub;
};
check_syscall_args(osf1_sys_lstat2)

struct osf1_sys_fstat2_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_stat2 *) sb;
};
check_syscall_args(osf1_sys_fstat2)

struct sys_getpgid_args;

struct sys_getsid_args;

struct osf1_sys_sigaltstack_args {
	syscallarg(struct osf1_sigaltstack *) nss;
	syscallarg(struct osf1_sigaltstack *) oss;
};
check_syscall_args(osf1_sys_sigaltstack)

struct osf1_sys_sysinfo_args {
	syscallarg(int) cmd;
	syscallarg(char *) buf;
	syscallarg(long) len;
};
check_syscall_args(osf1_sys_sysinfo)

struct osf1_sys_pathconf_args {
	syscallarg(const char *) path;
	syscallarg(int) name;
};
check_syscall_args(osf1_sys_pathconf)

struct osf1_sys_fpathconf_args {
	syscallarg(int) fd;
	syscallarg(int) name;
};
check_syscall_args(osf1_sys_fpathconf)

struct osf1_sys_usleep_thread_args {
	syscallarg(struct osf1_timeval *) sleep;
	syscallarg(struct osf1_timeval *) slept;
};
check_syscall_args(osf1_sys_usleep_thread)

struct osf1_sys_getsysinfo_args {
	syscallarg(u_long) op;
	syscallarg(void *) buffer;
	syscallarg(u_long) nbytes;
	syscallarg(void *) arg;
	syscallarg(u_long) flag;
};
check_syscall_args(osf1_sys_getsysinfo)

struct osf1_sys_setsysinfo_args {
	syscallarg(u_long) op;
	syscallarg(void *) buffer;
	syscallarg(u_long) nbytes;
	syscallarg(void *) arg;
	syscallarg(u_long) flag;
};
check_syscall_args(osf1_sys_setsysinfo)

/*
 * System call prototypes.
 */

int	sys_nosys(struct lwp *, const void *, register_t *);

int	sys_exit(struct lwp *, const struct sys_exit_args *, register_t *);

int	sys_fork(struct lwp *, const void *, register_t *);

int	sys_read(struct lwp *, const struct sys_read_args *, register_t *);

int	sys_write(struct lwp *, const struct sys_write_args *, register_t *);

int	sys_close(struct lwp *, const struct sys_close_args *, register_t *);

int	osf1_sys_wait4(struct lwp *, const struct osf1_sys_wait4_args *, register_t *);

int	sys_link(struct lwp *, const struct sys_link_args *, register_t *);

int	sys_unlink(struct lwp *, const struct sys_unlink_args *, register_t *);

int	sys_chdir(struct lwp *, const struct sys_chdir_args *, register_t *);

int	sys_fchdir(struct lwp *, const struct sys_fchdir_args *, register_t *);

int	osf1_sys_mknod(struct lwp *, const struct osf1_sys_mknod_args *, register_t *);

int	sys_chmod(struct lwp *, const struct sys_chmod_args *, register_t *);

int	sys___posix_chown(struct lwp *, const struct sys___posix_chown_args *, register_t *);

int	sys_obreak(struct lwp *, const struct sys_obreak_args *, register_t *);

int	osf1_sys_getfsstat(struct lwp *, const struct osf1_sys_getfsstat_args *, register_t *);

int	osf1_sys_lseek(struct lwp *, const struct osf1_sys_lseek_args *, register_t *);

int	sys_getpid_with_ppid(struct lwp *, const void *, register_t *);

int	osf1_sys_mount(struct lwp *, const struct osf1_sys_mount_args *, register_t *);

int	osf1_sys_unmount(struct lwp *, const struct osf1_sys_unmount_args *, register_t *);

int	osf1_sys_setuid(struct lwp *, const struct osf1_sys_setuid_args *, register_t *);

int	sys_getuid_with_euid(struct lwp *, const void *, register_t *);

int	osf1_sys_recvmsg_xopen(struct lwp *, const struct osf1_sys_recvmsg_xopen_args *, register_t *);

int	osf1_sys_sendmsg_xopen(struct lwp *, const struct osf1_sys_sendmsg_xopen_args *, register_t *);

int	osf1_sys_access(struct lwp *, const struct osf1_sys_access_args *, register_t *);

int	sys_sync(struct lwp *, const void *, register_t *);

int	sys_kill(struct lwp *, const struct sys_kill_args *, register_t *);

int	sys_setpgid(struct lwp *, const struct sys_setpgid_args *, register_t *);

int	sys_dup(struct lwp *, const struct sys_dup_args *, register_t *);

int	sys_pipe(struct lwp *, const void *, register_t *);

int	osf1_sys_set_program_attributes(struct lwp *, const struct osf1_sys_set_program_attributes_args *, register_t *);

int	osf1_sys_open(struct lwp *, const struct osf1_sys_open_args *, register_t *);

int	sys_getgid_with_egid(struct lwp *, const void *, register_t *);

int	compat_13_sys_sigprocmask(struct lwp *, const struct compat_13_sys_sigprocmask_args *, register_t *);

int	sys___getlogin(struct lwp *, const struct sys___getlogin_args *, register_t *);

int	sys___setlogin(struct lwp *, const struct sys___setlogin_args *, register_t *);

int	sys_acct(struct lwp *, const struct sys_acct_args *, register_t *);

int	osf1_sys_classcntl(struct lwp *, const struct osf1_sys_classcntl_args *, register_t *);

int	osf1_sys_ioctl(struct lwp *, const struct osf1_sys_ioctl_args *, register_t *);

int	osf1_sys_reboot(struct lwp *, const struct osf1_sys_reboot_args *, register_t *);

int	sys_revoke(struct lwp *, const struct sys_revoke_args *, register_t *);

int	sys_symlink(struct lwp *, const struct sys_symlink_args *, register_t *);

int	sys_readlink(struct lwp *, const struct sys_readlink_args *, register_t *);

int	osf1_sys_execve(struct lwp *, const struct osf1_sys_execve_args *, register_t *);

int	sys_umask(struct lwp *, const struct sys_umask_args *, register_t *);

int	sys_chroot(struct lwp *, const struct sys_chroot_args *, register_t *);

int	sys_getpgrp(struct lwp *, const void *, register_t *);

int	compat_43_sys_getpagesize(struct lwp *, const void *, register_t *);

int	sys_vfork(struct lwp *, const void *, register_t *);

int	osf1_sys_stat(struct lwp *, const struct osf1_sys_stat_args *, register_t *);

int	osf1_sys_lstat(struct lwp *, const struct osf1_sys_lstat_args *, register_t *);

int	osf1_sys_mmap(struct lwp *, const struct osf1_sys_mmap_args *, register_t *);

int	sys_munmap(struct lwp *, const struct sys_munmap_args *, register_t *);

int	osf1_sys_mprotect(struct lwp *, const struct osf1_sys_mprotect_args *, register_t *);

int	osf1_sys_madvise(struct lwp *, const struct osf1_sys_madvise_args *, register_t *);

int	sys_getgroups(struct lwp *, const struct sys_getgroups_args *, register_t *);

int	sys_setgroups(struct lwp *, const struct sys_setgroups_args *, register_t *);

int	osf1_sys_setitimer(struct lwp *, const struct osf1_sys_setitimer_args *, register_t *);

int	osf1_sys_getitimer(struct lwp *, const struct osf1_sys_getitimer_args *, register_t *);

int	compat_43_sys_gethostname(struct lwp *, const struct compat_43_sys_gethostname_args *, register_t *);

int	compat_43_sys_sethostname(struct lwp *, const struct compat_43_sys_sethostname_args *, register_t *);

int	compat_43_sys_getdtablesize(struct lwp *, const void *, register_t *);

int	sys_dup2(struct lwp *, const struct sys_dup2_args *, register_t *);

int	osf1_sys_fstat(struct lwp *, const struct osf1_sys_fstat_args *, register_t *);

int	osf1_sys_fcntl(struct lwp *, const struct osf1_sys_fcntl_args *, register_t *);

int	osf1_sys_select(struct lwp *, const struct osf1_sys_select_args *, register_t *);

int	sys_poll(struct lwp *, const struct sys_poll_args *, register_t *);

int	sys_fsync(struct lwp *, const struct sys_fsync_args *, register_t *);

int	sys_setpriority(struct lwp *, const struct sys_setpriority_args *, register_t *);

int	osf1_sys_socket(struct lwp *, const struct osf1_sys_socket_args *, register_t *);

int	sys_connect(struct lwp *, const struct sys_connect_args *, register_t *);

int	compat_43_sys_accept(struct lwp *, const struct compat_43_sys_accept_args *, register_t *);

int	sys_getpriority(struct lwp *, const struct sys_getpriority_args *, register_t *);

int	compat_43_sys_send(struct lwp *, const struct compat_43_sys_send_args *, register_t *);

int	compat_43_sys_recv(struct lwp *, const struct compat_43_sys_recv_args *, register_t *);

int	compat_13_sys_sigreturn(struct lwp *, const struct compat_13_sys_sigreturn_args *, register_t *);

int	sys_bind(struct lwp *, const struct sys_bind_args *, register_t *);

int	sys_setsockopt(struct lwp *, const struct sys_setsockopt_args *, register_t *);

int	sys_listen(struct lwp *, const struct sys_listen_args *, register_t *);

int	compat_13_sys_sigsuspend(struct lwp *, const struct compat_13_sys_sigsuspend_args *, register_t *);

int	compat_43_sys_sigstack(struct lwp *, const struct compat_43_sys_sigstack_args *, register_t *);

int	osf1_sys_gettimeofday(struct lwp *, const struct osf1_sys_gettimeofday_args *, register_t *);

int	osf1_sys_getrusage(struct lwp *, const struct osf1_sys_getrusage_args *, register_t *);

int	sys_getsockopt(struct lwp *, const struct sys_getsockopt_args *, register_t *);

int	osf1_sys_readv(struct lwp *, const struct osf1_sys_readv_args *, register_t *);

int	osf1_sys_writev(struct lwp *, const struct osf1_sys_writev_args *, register_t *);

int	osf1_sys_settimeofday(struct lwp *, const struct osf1_sys_settimeofday_args *, register_t *);

int	sys___posix_fchown(struct lwp *, const struct sys___posix_fchown_args *, register_t *);

int	sys_fchmod(struct lwp *, const struct sys_fchmod_args *, register_t *);

int	compat_43_sys_recvfrom(struct lwp *, const struct compat_43_sys_recvfrom_args *, register_t *);

int	sys_setreuid(struct lwp *, const struct sys_setreuid_args *, register_t *);

int	sys_setregid(struct lwp *, const struct sys_setregid_args *, register_t *);

int	sys___posix_rename(struct lwp *, const struct sys___posix_rename_args *, register_t *);

int	osf1_sys_truncate(struct lwp *, const struct osf1_sys_truncate_args *, register_t *);

int	osf1_sys_ftruncate(struct lwp *, const struct osf1_sys_ftruncate_args *, register_t *);

int	sys_flock(struct lwp *, const struct sys_flock_args *, register_t *);

int	osf1_sys_setgid(struct lwp *, const struct osf1_sys_setgid_args *, register_t *);

int	osf1_sys_sendto(struct lwp *, const struct osf1_sys_sendto_args *, register_t *);

int	sys_shutdown(struct lwp *, const struct sys_shutdown_args *, register_t *);

int	osf1_sys_socketpair(struct lwp *, const struct osf1_sys_socketpair_args *, register_t *);

int	sys_mkdir(struct lwp *, const struct sys_mkdir_args *, register_t *);

int	sys_rmdir(struct lwp *, const struct sys_rmdir_args *, register_t *);

int	osf1_sys_utimes(struct lwp *, const struct osf1_sys_utimes_args *, register_t *);

int	compat_43_sys_getpeername(struct lwp *, const struct compat_43_sys_getpeername_args *, register_t *);

int	compat_43_sys_gethostid(struct lwp *, const void *, register_t *);

int	compat_43_sys_sethostid(struct lwp *, const struct compat_43_sys_sethostid_args *, register_t *);

int	osf1_sys_getrlimit(struct lwp *, const struct osf1_sys_getrlimit_args *, register_t *);

int	osf1_sys_setrlimit(struct lwp *, const struct osf1_sys_setrlimit_args *, register_t *);

int	sys_setsid(struct lwp *, const void *, register_t *);

int	compat_43_sys_quota(struct lwp *, const void *, register_t *);

int	compat_43_sys_getsockname(struct lwp *, const struct compat_43_sys_getsockname_args *, register_t *);

int	osf1_sys_sigaction(struct lwp *, const struct osf1_sys_sigaction_args *, register_t *);

int	osf1_sys_getdirentries(struct lwp *, const struct osf1_sys_getdirentries_args *, register_t *);

int	osf1_sys_statfs(struct lwp *, const struct osf1_sys_statfs_args *, register_t *);

int	osf1_sys_fstatfs(struct lwp *, const struct osf1_sys_fstatfs_args *, register_t *);

int	compat_09_sys_getdomainname(struct lwp *, const struct compat_09_sys_getdomainname_args *, register_t *);

int	compat_09_sys_setdomainname(struct lwp *, const struct compat_09_sys_setdomainname_args *, register_t *);

int	osf1_sys_uname(struct lwp *, const struct osf1_sys_uname_args *, register_t *);

int	sys___posix_lchown(struct lwp *, const struct sys___posix_lchown_args *, register_t *);

int	osf1_sys_shmat(struct lwp *, const struct osf1_sys_shmat_args *, register_t *);

int	osf1_sys_shmctl(struct lwp *, const struct osf1_sys_shmctl_args *, register_t *);

int	osf1_sys_shmdt(struct lwp *, const struct osf1_sys_shmdt_args *, register_t *);

int	osf1_sys_shmget(struct lwp *, const struct osf1_sys_shmget_args *, register_t *);

int	osf1_sys_stat2(struct lwp *, const struct osf1_sys_stat2_args *, register_t *);

int	osf1_sys_lstat2(struct lwp *, const struct osf1_sys_lstat2_args *, register_t *);

int	osf1_sys_fstat2(struct lwp *, const struct osf1_sys_fstat2_args *, register_t *);

int	sys_getpgid(struct lwp *, const struct sys_getpgid_args *, register_t *);

int	sys_getsid(struct lwp *, const struct sys_getsid_args *, register_t *);

int	osf1_sys_sigaltstack(struct lwp *, const struct osf1_sys_sigaltstack_args *, register_t *);

int	osf1_sys_sysinfo(struct lwp *, const struct osf1_sys_sysinfo_args *, register_t *);

int	osf1_sys_pathconf(struct lwp *, const struct osf1_sys_pathconf_args *, register_t *);

int	osf1_sys_fpathconf(struct lwp *, const struct osf1_sys_fpathconf_args *, register_t *);

int	osf1_sys_usleep_thread(struct lwp *, const struct osf1_sys_usleep_thread_args *, register_t *);

int	osf1_sys_getsysinfo(struct lwp *, const struct osf1_sys_getsysinfo_args *, register_t *);

int	osf1_sys_setsysinfo(struct lwp *, const struct osf1_sys_setsysinfo_args *, register_t *);

#endif /* _OSF1_SYS_SYSCALLARGS_H_ */
