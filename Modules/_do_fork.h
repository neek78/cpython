#ifndef Py__DO_FORK_H
#define Py__DO_FORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pyconfig.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef MS_WINDOWS

/* attempt to open a process_fd with the fork() (eg a pidfd / process descriptor) */
#define FORK_FLAG_OPEN_PROCESS_FD (1UL << 0)
/* enable the OS level automatic kill of the child when the fd closes.
 * this implies AUTOREAP too */
#define FORK_FLAG_KILL_ON_CLOSE   (1UL << 1)
/* don't require a wait() to cleanup a zombie process */
#define FORK_FLAG_AUTOREAP        (1UL << 2)

pid_t _do_fork(int* process_fd, unsigned int flags);

#endif // MS_WINDOWS

#ifdef __cplusplus
}
#endif

#endif // Py__DO_FORK_H
