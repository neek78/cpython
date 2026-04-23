
#include "Python.h"

#include "_do_fork.h"

#if defined(HAVE_SYS_PIDFD_H)
#  include <sys/pidfd.h>          // pidfd_open()
#endif

#if defined(HAVE_SYS_PROCDESC_H)
#  include <sys/procdesc.h>       // pdfork()
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <assert.h>

/* define to try clone3() instead of fork()/pidfd_open() on linux */
//#define USE_CLONE3 1

#ifdef USE_CLONE3

#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <signal.h>
#include <stdint.h>

/* these have just been added to the kernel, haven't made it to glibc's sched.h yet */
#ifndef CLONE_AUTOREAP
#define CLONE_AUTOREAP		    (1ULL << 34) /* Auto-reap child on exit. */
#endif

#ifndef CLONE_PIDFD_AUTOKILL
#define CLONE_PIDFD_AUTOKILL	(1ULL << 36) /* Kill child when clone pidfd closes. */
#endif

/* experimental call direct to clone3() */
static pid_t do_clone3(int* process_fd, unsigned int flags)
{
    //FIXME: check for other flags we can't support

    uint64_t exit_signal = SIGCHLD;

    struct clone_args cl_args = {
        .exit_signal = exit_signal
    };

    if (flags & FORK_FLAG_OPEN_PROCESS_FD) {
        assert(process_fd != NULL);
        cl_args.flags |= CLONE_PIDFD;
        cl_args.pidfd = (uint64_t)process_fd;
    }

    if (flags & FORK_FLAG_KILL_ON_CLOSE) {
        if (!(flags & FORK_FLAG_OPEN_PROCESS_FD)) {
            PyErr_SetString(PyExc_ValueError,
                    "FORK_FLAG_KILL_ON_CLOSE requires FORK_FLAG_OPEN_PROCESS_FD");
            return -1;
        }
#ifdef CLONE_PIDFD_AUTOKILL
        cl_args.flags |= CLONE_PIDFD_AUTOKILL;
#else
        PyErr_SetString(PyExc_ValueError, "CLONE_PIDFD_AUTOKILL is not compiled in");
        return -1;
#endif
    }

    if (flags & FORK_FLAG_AUTOREAP) {
#ifdef CLONE_AUTOREAP
        cl_args.flags |= CLONE_AUTOREAP;
#else
        PyErr_SetString(PyExc_ValueError, "CLONE_AUTOREAP is not compiled in");
        return -1;
#endif
    }

    size_t size = sizeof(cl_args);

    long ret = syscall(SYS_clone3, &cl_args, size);

    return ret;
}

#else /* USE_CLONE3 */

#ifdef HAVE_PIDFD_OPEN
/* fork and generate a pidfd in the parent process */
static pid_t do_fork_pidfd(int* process_fd, unsigned int flags)
{
    assert(process_fd != NULL);

    pid_t pid = fork();
    if (pid <= 0) {
        /* child process, or error */
        return pid;
    }

    /* parent process.
     * Now open a pidfd for the child process. Per the pidfd_open() manpage,
     * this should be race free and valid, even if the child's already
     * terminated, so long as and no one's been
     * messing with SIGCHILD or reaping with wait() at the same time. See
     * the manpage for more details.
     *
     * It would be nice to just call clone3() which can atomically create
     * a pidfd. And I tested it here, it works(see above).
     * However, there's no libc wrapper for clone3(), and there's a bunch
     * of stuff in glibc's fork() that it seems like it would be really
     * bad to call-around.
     *
     * If pidfd_open() fails it'll return -1, so we'll use that to indicate
     * an error. We wont fail this call, as the child has been created.
     *
     * If the exec() call fails in the child, we'll still return process_fd,
     * and the python code will clean it up as normal. */

    *process_fd = pidfd_open(pid, 0);
    return pid;
}
#endif
#endif

#if defined(HAVE_PDFORK)
pid_t _do_pdfork(int* process_fd, unsigned int flags)
{
    assert(process_fd != NULL);
    //FIXME: check other flags we can't support

    /* PD_DAEMON is the "don't kill on close" flag. */
    unsigned int fl = (flags & FORK_FLAG_KILL_ON_CLOSE) ? 0 : PD_DAEMON;

    return pdfork(process_fd, fl);
}
#endif

pid_t _do_fork(int* process_fd, unsigned int flags)
{
    // check we don't have a null here. process_fd is only
    // is passed from c code, never Python, so we can assert here.
    if (flags & FORK_FLAG_OPEN_PROCESS_FD) {
        assert(process_fd != NULL);
    }

    if (process_fd != NULL) {
        *process_fd = -1;
    }

#if defined(USE_CLONE3)
    /* do_clone3 handles FORK_FLAG_OPEN_PROCESS_FD */
    return do_clone3(process_fd, flags);
#elif defined(HAVE_PIDFD_OPEN)
    if (flags & FORK_FLAG_OPEN_PROCESS_FD) {
        return do_fork_pidfd(process_fd, flags);
    }
#elif defined(HAVE_PDFORK)
    if (flags & FORK_FLAG_OPEN_PROCESS_FD) {
        return do_pdfork(process_fd, flags);
    }
#endif

    /* FIXME: return an error here if flags as for things we can't satisfy
     * eg autoreap, etc */
    return fork();
}

