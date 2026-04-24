
Overview
========

This is a proof-of-concept library to provide improved support for managing and interacting with other processes from Python code.

Background
==========

Various POSIX systems have added improved mechnisms for tracking proceses - Such as pidfd on Linux - avoiding some of the pitfalls of using PIDs for this purpose. There have been some admirable improvements recently in Python's libraries to make use of these - for example adding pidfd or kevent waiting for subprocesses instead of busy looping, as well as using pidfds when waiting on a process in asyncio.

However, the fundamental identifier for external processes on POSIX systems in Python is the pid. This library attempts to provide an improved infrastructure to use the best possible mechanism end-to-end.

Note that Windows has provided reliable mechanisms for tracking processes (ie HANDLEs for processes) since before Python existed. Conversely, Python's process tracking on Windows already uses this. This library integrates a bit of existing Windows code, but doesn't change much in this space.

Objectives
==========

Things that are partially or fully implemented:
-----------------------------------------------

* New module process_handle provides a library for monitoring and managing external processes, not just processes we launched (ie subprocesses). Process handles can be attached to existing processes, or created by libraries like subprocess.
* Make use of the best possible underlying OS mechanisms (eg pidfd on Linux, pdfork() on FreeBSD) to avoid pid races, etc.
* Make use of underlying mechanisms to atomically create processes and a matching handle (eg pdfork(), pidfd_spawn, some future linux clone3() wrapper).
* Avoid busy-looping as much as possible and instead use blocking mechanisms to wait for process completion where possible.
* Integrate process_handle with subprocess, and re-use existing code as much as possible.
* Provide pid-recycling detection where a reliable mechanism (like pidfd) is not available.
* New pidfd module giving access to some of the more escoteric features of pidfds (obviously Linux only).
* Provide a mechanism to allow reliable tracking of a process even when continuous maintenance of a file descriptor is not possible - eg via serialization/pickling, or to a separate process. The unique_id mechanism allows recreation of a ProcessHandle that can reliably be said to point at the same process as a previous one (or explicitly fail).
* Have an as consistent as possible interface across operating systems (including Windows).

Other Objectives:
-----------------

* Support kill-on-close functionality if the underlying OS supports it. FreeBSD offers this with pdfork(), linux support also very recently available. On Linux, this would probably require a fork()-style wrapper for clone3() in glibc, and/or changes to pidfd_spawn().
* Integrate with modules such as asyncio and multiprocessing as appropriate.
* Complete unique_id work, and make sure this works well with things like pickling

Points of Interest
------------------

* New module Lib/process_handle.py
* subprocess adapted to make use of process_handle. subprocess.Popen objects now have a process_handle property
* New module pidfd exposes extra pidfd interfaces
* Various new interfaces added to os (pidfd_spawn(), fork_flags(), pdgetpid() etc)
* A few new interfaces exposed via _winapi

Current Status
--------------

This is just a proof of concept, and there is a lot of work to be done if it's to go forward. It's hardly thorougly tested (ie it needs many more test cases written), as well as tested on more OS versions - in particular older Linux / glibc versions where some of the latest facilities are not available.

Linux pidfd status
==================

A lot of the state of pidfd is shifting at the moment, and a lot of docs seem to be a bit out out date. There doesn't seem to be too much of a central guide to all things pidfd, so here we go. Also listed is where access to these mechanisms are available in thie branch.

Syscalls and library functions
------------------------------
These functions now exist in glibc as syscall wrappers; The manpage(s) are a little out of date, and claim you need to call some or all of these via syscall(). Recent glibc versions now appear to provide prototypes for these in sys/pidfd.h.

pidfd_open()
------------
Opens a pidfd given a pid. Already available via os.pidfd_open()

pidfd_send_signal()
-------------------
Sends a signal via a pidfd. Already available via signal.pidfd_signal()

pidfd_getfd()
-------------
Copies an fd from another process via a pidfd. Available in new pidfd module.

pidfd_getpid()
--------------
Exists in glibc. This is not a (direct) syscall, but a library function that maps pidfd to pid (added sep 2023). The existing released versions use /proc to do this mapping, so will fail if /proc is not available. An update in late Jan 2026 also tries the PIDFD_GET_INFO ioctl (in preference to /proc) to fulfill this request. Available in new pidfd module.

waitid(), process_madvise(), setns()
-------------------------------------
pidfds can be used to identify a process to these functions. For example, waitid() allows you to wait directly on a pidfd using idtype == P_PIDFD. Already available via os.waitid()

poll/select/epoll
-----------------
These mechanisms can wait on a pidfd and report it completing and being reaped as separate events.

ioctl PIDFD_GET_INFO
--------------------
iocctl PIDFD_GET_INFO apparently was added to the kernel in v6.13, provides all manner of useful info about the pidfd. There seems to be at least 3 versions of the accomanying struct in the wild. Which values are returned are controlled by a flags arg. Available in new pidfd module.

Importantly, allows the retrieval of the process exit status without using wait() - or even if the process has been reaped.

other ioctls
------------
There are other ioctls for a pidfd, giving access to namespace info. see sys/pidfd.h. Available in new pidfd module.

pidfd_spawn() / pidfd_spawnp
----------------------------
Added to glibc to provide an equivalent to posix_spawn[p] - returns a pidfd, instead of a pid, and I think uses clone3() internally, thus avoiding races. Available via os.pidfd_spawn[p]

These don't let you (yet) specify AUTOREAP and kill on close.
https://sourceware.org/bugzilla/show_bug.cgi?id=34077

clone3()
--------
clone3() now allows creation of a pidfd atomically with the creation of the new process, and passes this pidfd to the parent. Unfortunately, there's no glibc wrapper exposing this feature in an eqivalent of fork(). pidfd_spawn() appears to use it.

You can call clone3 directly (via syscall()), and it works, but it seems like a monumentally bad idea to bypass much of the stuff in glibc.

The current implementation in this branch of Python uses traditional fork(), then immediately calls pidfd_open(). This is race-free if certain conditions hold, see pidfd_open(2). It would be preferable to use a clone3() based call to create the pidfd atomically. Additionally, it will be required to make use of things like AUTOKILL/AUTOREAP.

Kill on close / CLONE_PIDFD_AUTOKILL
-------------------------------------
There's new functionality kicking about to have the kernel kill a process when the last open pidfd closes. Merged into the mainline kernel April 13th, I guess thus for Linux 7.1. This functionality would appear to only apply to pidfds created from clone3() (and duplicates of them), not opened via pidfd_open(). https://lkml.org/lkml/2026/2/23/583

Requires AUTOREAP -

CLONE_AUTOREAP
--------------
Not strictly part of pidfd, but very useful.

Merged into mainline kernel Feb 26th (thus 7.1), causes the kernel to cleanup zombies itself. Exit status is available via ioctl PIDFD_GET_INFO against a pidfd, as well as waitid() against a pidfd. So long as the pidfd is open, the exit status appears to be retained by the kernel.

This would improve, for example, subprocess.Popen()'s __exit__() as it wouldn't have to wait() and potentially deadlock. As well as the whole wait()/locking mechanism in subprocess which needs to lock to ensure only one thread waits on a child. If we decide we don't care about the exit status a child, they can be forgotten about without the risk of zombies.

https://lwn.net/Articles/1059350/

pidfs
-----
Since linux 6.9, pidfs allows queries via statfs()/stat(). Allows you, amongst other things to query that a fd is a pidfd, and get an inode number for it, which is a good unique id that should remain unique until system restart.




