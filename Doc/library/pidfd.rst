
:mod:`!pidfd` --- System interfaces pertaining to Linux pid file descriptors
========================================================

.. module:: pidfd
   :synopsis: System interfaces pertaining to Linux pid file descriptors

--------------
function:: getpid(pidfd)

Return the pid for the given process file descriptor *pidfd*.

This function is implemented in glibc, and was added in version 2.39.
The initial version relies on accessing /proc (specifically, /proc/self/fdinfo/).
Thus, this will fail in environments where /proc is not available.

Kernel 6.13 added a new ioctl PIDFD_GET_INFO which allows pidfd_getpid() to
avoid using /proc. Support for this ioctl was added to glibc in late January 2026,
so should be in glibc version 2.44 (?).

.. availability:: Linux >= 5.10
.. versionadded:: FIXME

function getinfo(pidfd, flags)

Execute the PIDFD_GET_INFO ioctl on the pidfd, to obtain info about the fd
