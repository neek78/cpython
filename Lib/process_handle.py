"""
process_handle module.

This provides an interface which wraps the OS's mechanism for idenifying and controlling
processes - (eg PID, handle, pidfd etc).
"""

import enum
import errno
import os
import select
import signal
import sys
import time

lazy import stat  # stat only needed on FreeBSD
lazy import threading

import _process_support

try:
    import pidfd  # only available on Linux
except ModuleNotFoundError:
    pidfd = None

try:
    import _procinfo  # only avilable on macOS / *BSD at the moment
except ModuleNotFoundError:
    _procinfo = None

# this is subprocess.py's method for determining if we're on windows.. We'll match it
try:
    import msvcrt
except ModuleNotFoundError:
    _mswindows = False
else:
    _mswindows = True
    import _winapi


def _can_use_pidfd_open():
    # Availability: Linux >= 5.3
    if not hasattr(os, "pidfd_open"):
        return False, False
    try:
        fd = os.pidfd_open(os.getpid(), 0)
    except OSError as err:
        # we used to return true for err.errno in {errno.EMFILE, errno.ENFILE},
        # regarding it as a transitory error.
        # However, that it means we can't test pidfd_getpid(), which has has
        # more very real failure scenarios - for example, an older glibc version
        # coupled with no access to /proc

        # likely blocked by security policy like SECCOMP (EPERM, EACCES, ENOSYS)
        return False, False

    # test pidfd_getpid()
    can_use_getpid = False
    try:
        import pidfd

        pid = pidfd.pidfd_getpid(fd)

        # i'm not sure of a case where this can fail
        # but we might as well check.
        can_use_getpid = pid == os.getpid()
    except:
        pass

    os.close(fd)
    return True, can_use_getpid


def _can_use_kqueue():
    # Availability: macOS, BSD
    names = (
        "kqueue",
        "KQ_EV_ADD",
        "KQ_EV_ONESHOT",
        "KQ_FILTER_PROC",
        "KQ_NOTE_EXIT",
    )
    if not all(hasattr(select, x) for x in names):
        return False
    kq = None
    try:
        kq = select.kqueue()
        kev = select.kevent(
            os.getpid(),
            filter=select.KQ_FILTER_PROC,
            flags=select.KQ_EV_ADD | select.KQ_EV_ONESHOT,
            fflags=select.KQ_NOTE_EXIT,
        )
        kq.control([kev], 1, 0)
        return True
    except OSError as err:
        if err.errno in {errno.EMFILE, errno.ENFILE}:
            # transitory 'too many open files'
            return True
        return False
    finally:
        if kq is not None:
            kq.close()


def _get_system_boottime():
    if _procinfo is not None:
        sec, usec = _procinfo.get_boottime()
        return sec

    if _mswindows:
        ticks_millisec = _winapi.GetTickCount64()
        boottime = time.time() - (ticks_millisec / 1000)
        return int(boottime)

    if sys.platform == "linux":
        with open("/proc/stat", "r", encoding="utf-8") as f:
            for l in f.readlines():
                k, v = l.split()[0:2]
                if k.strip() == "btime":
                    return int(v.strip())

    return None


_USE_KQUEUE = not _mswindows and _can_use_kqueue()
_USE_KQUEUE_EXITSTATUS = _USE_KQUEUE and hasattr(select, "KQ_NOTE_EXITSTATUS")

_USE_PIDFD_OPEN, _USE_PIDFD_GETPID = _can_use_pidfd_open()
_USE_PIDFD_GET_INODE = pidfd is not None and hasattr(pidfd, "get_inode")
_USE_PIDFD_GET_INFO_FOR_RETURNCODE = \
    hasattr(pidfd, "get_info") and hasattr(pidfd, "PIDFD_INFO_EXIT")

_USE_PD = not _mswindows and hasattr(os, "pdgetpid")

_SYSTEM_BOOTTIME = _get_system_boottime()


def _procinfo_get_pstat_and_ctime(pid) -> str:
    pstat, sec, usec = _procinfo.get_pstat_and_ctime(pid)
    return pstat, f"{sec}.{usec}"

def _get_ctime(pid) -> str:
    """
    Return the process create time. This is not in any specified time format
    exactly, as it's only used to check uniqueness. On linux this is time since
    boot, for example.
    Note that this is specifically returned as a string, not as an int or float or
    something fancier. This is because it's used as an indentity check, and it
    includes subsecond resolution. We don't want to go down the path of float
    comparison errors, etc.
    """
    if _procinfo is not None:
        return _procinfo_get_pstat_and_ctime(pid)[1]

    if sys.platform == "linux":
        # try /proc
        fn = f"/proc/{pid}/stat"
        with open(fn, "r", encoding="utf-8") as f:
            line = f.readline()

        # eat everything before the (command name)
        _, after = line.split(")")
        # ctime is the 22nd field. After eating the first two above, it's the
        # 20th, hence 19 (zero based)

        # note that we don't bother doing any convesion here (eg taking clock ticks into
        # account) as we only use this as a unique id value, the value itself doesn't matter.
        # FIXME : can this value change if (say) the SC_CLK_TCK changes?
        ctime = after.split()[19]
        return ctime

        # tick = os.sysconf(os.sysconf_names['SC_CLK_TCK'])
        # return int(ctime) / tick

    raise NotImplementedError


# Exceptions
class ProcessHandleError(Exception):
    pass


class ProcessMismatchError(ProcessHandleError):
    def __init__(
        self, txt, existing_unique_id: UniqueId = None, new_unique_id: UniqueId = None
    ):
        super().__init__(txt)
        self.new_unique_id = new_unique_id
        self.existing_unique_id = existing_unique_id


class UniqueIdError(ProcessHandleError):
    def __init__(self, txt, unique_id: UniqueId = None):
        super().__init__(txt)


class CorruptUniqueIdError(UniqueIdError):
    pass


class IncompatibleUniqueIdError(UniqueIdError):
    pass


# FIXME: capture some details here
class WaitFailed(ProcessHandleError):
    pass


class BlockFailed(ProcessHandleError):
    pass


class TimeoutExpired(ProcessHandleError):
    def __init__(self, timeout):
        self.timeout = timeout

    def __str__(self):
        return "Timed out after %s seconds" % self.timeout


_unique_id_valid_key_combos = [("pid", "pidfs_inode"), ("pid", "ctime")]


class UniqueId:
    def __init__(self, pid: int):
        self._elems = dict()
        self._elems["pid"] = pid

    def copy(self):
        other = UniqueId(self.pid)
        other._elems = self._elems.copy()
        return other

    def addElem(self, k, v):
        self._elems[k] = v

    @property
    def pid(self) -> int:
        return self._elems["pid"]

    def __eq__(self, other):
        # FIXME: this could be improved - just a simple dict compare for now
        return self._elems == other._elems

    def __repr__(self):
        l = []
        for k, v in self._elems.items():
            l.append(str(k) + "=" + str(v))
        return ":".join(l)

    def is_valid(self):
        for combo in _unique_id_valid_key_combos:
            ok = True
            for key in combo:
                if key not in self._elems:
                    ok = False
                    break
            if ok:
                return True
        return False

    def has_key(self, key):
        return key in self._elems


# FIXME: the "pid" part of this name makes no sense (all unique ids have a pid) - should be ctime
def _gen_pid_unique_id(pid, ctime=None) -> UniqueId:
    u = UniqueId(pid)
    if ctime is None:
        ctime = _get_ctime(pid)
    u.addElem("ctime", ctime)
    return u


class _Poller:
    pass


class _KQueuePoller(_Poller):
    """
    Performs blocking waits on processes using the kqueue() interface
    """

    def __init__(self, filter, wait_object, pid):
        self._filter = filter
        self._wait_object = wait_object
        self._pid = pid
        self._returncode = None

        self._lock = threading.Lock()
        self._kq = None
        self._kev = None
        self._seen_eof = False

    def alloc_waiter(self) -> None:
        with self._lock:
            if self._kq is not None:
                assert self._kev is not None
                return

            self._kq = select.kqueue()

            fflags = select.KQ_NOTE_EXIT
            if _USE_KQUEUE_EXITSTATUS:
                fflags |= select.KQ_NOTE_EXITSTATUS

            try:
                self._kev = select.kevent(
                    self._wait_object,
                    filter=self._filter,
                    flags=select.KQ_EV_ADD,
                    fflags=fflags,
                )
            except:
                self._kev = None
                self._kq = None
                raise

    def close(self):
        with self._lock:
            if self._kq is not None:
                self._kq.close()
                self._kq = None

    @staticmethod
    def _decode_status(status):
        if os.WIFSTOPPED(status):
            return -os.WSTOPSIG(status)
        else:
            return os.waitstatus_to_exitcode(status)

    def poll(self, timeout):
        try:
            self.alloc_waiter()
            events = self._kq.control([self._kev], 1, timeout)
        except:
            # FIXME: catch more info
            raise WaitFailed("kq.control() failed")

        if events == []:
            raise TimeoutExpired(timeout)
        return events

    def block_for_wait(self, timeout):
        # if we've already seen EOF, the process must be gone
        # waiting again will return KQ_EV_ERROR.
        if self._seen_eof:
            return self._returncode

        # poll() will raise if the timeout expires
        events = self.poll(timeout)

        assert len(events) == 1
        ev = events[0]

        # print("\nEV", ev)

        if ev.flags & select.KQ_EV_ERROR:
            # ev.data is errno
            raise OSError(ev.data, "kevent() gave EV_ERROR in flags. ev=%s" % str(ev))

        if ev.flags & select.KQ_EV_EOF:
            # the ev object is still valid ... this time, but future
            # calls will give an error
            self._seen_eof = True

        # got some other kind of event.. can this happen?
        # either way, we don't know how to interpret ev.data
        if ev.filter != self._filter:
            raise WaitFailed(
                "Unexpected Filter Value %d - event = %s" % (ev.filter, ev)
            )

        status = None

        # Try to extract the returncode from the child. Because we blocked atomically with
        # kevent() there's no race between blocking and extracting the return code ...
        # ... however, the process wont have actually been reaped yet -
        # The above doesn't count as a wait().
        if ev.fflags & select.KQ_NOTE_EXIT or (
            _USE_KQUEUE_EXITSTATUS and ev.fflags & select.KQ_NOTE_EXITSTATUS
        ):
            status = ev.data
        else:
            raise WaitFailed(
                "Unexpected fflags Value %d - event = %s" % (ev.fflags, ev)
            )

        if status is not None:
            ret = self._decode_status(status)
            self._returncode = ret
            # print ("RET", ret)
            return ret
        else:
            return None


class _BusyLoopPoller(_Poller):
    def __init__(self, pid):
        self._pid = pid
        self._lock = threading.Lock()
        self._process_completed = False

    def _poll_process(self):
        """Check if the process is completed.
        Returns True if the process is gone"""
        try:
            os.kill(self.pid, 0)
        except ProcessLookupError as e:
            # process is gone.
            return True
        return False

    def _busy_wait(self, timeout: float = None):
        # This busy loop was cribbed from subprocess.py, which in turn cribbed it.
        delay = 0.0005
        if timeout is not None:
            endtime = time.monotonic() + timeout

        while True:
            with self._lock:
                if self._process_completed:
                    return
                if self._poll_process():
                    self._process_completed = True
                    # process is gone
                    return

                self._process_completed = False

            delay = min(delay * 2, 0.05)

            if timeout is not None:
                remaining = endtime - time.monotonic()
                if remaining <= 0:
                    raise TimeoutExpired(timeout)
                delay = min(delay, remaining)

            time.sleep(delay)

    def block_for_wait(self, timeout: float = None):
        # we don't know how to block, as we're just a simple pid holder...
        raise WaitFailed()


def _get_pid_poller(pid):
    """get the best poller type available, using PID as the wait object"""
    if _USE_KQUEUE:
        return _KQueuePoller(select.KQ_FILTER_PROC, pid, pid)
    return _BusyLoopPoller(pid)


class ProcessHandle:
    def __init__(self, pid: int = None):
        # pid is optional here, but if it's none,
        # the child class must provide a _get_pid() method
        if pid is None:
            self._pid = self._get_pid()
        else:
            self._pid = pid

        assert self._pid is not None

        self._returncode = None

        # kill on close is not implemented yet
        self._kill_on_close = False

    @property
    def pid(self) -> int:
        """
        This process handle's process identifier (pid). This will never be None.

        This property only passively returns the pid this handle was created with.
        It doesn't in any way check that a process with this pid exists (or has
        it ever), or that the pid has or hasn't been recycled.
        """
        # PID should always be defined.
        assert self._pid is not None
        return self._pid

    @property
    def returncode(self) -> int:
        """
        If the process has completed, its returncode. Otherwise None.

        This is only guaranteed to be available after wait() is called, and
        only if this object's wait() is used (os opposed to directly calling
        os.wait() or similar). If these conditions don't hold, a returncode
        may or may not be available.

        We make a best-effort to get hold of returncode. If the process in question is
        not our sub-process, we may not be able to get it (in which case it'll be None).

        If this process was created by subprocess, it's better to call
        subprocess.returncode instead.
        """
        if self._returncode is None:
            self._try_get_returncode()
        return self._returncode

    def _try_get_returncode(self):
        pass

    @property
    def kill_on_close(self) -> bool:
        """
        Is kill-on-close enabled for this process_handle?
        This is not implemented yet, so it always returns False.
        """
        return self._kill_on_close

    def _check_unique_id(
        self, existing_unique_id: UniqueId, new_unique_id: UniqueId
    ) -> None:
        if not existing_unique_id.is_valid():
            raise CorruptUniqueIdError("is not valid", existing_unique_id)

        # new_unique_id has just been generated, hope it's valid
        assert new_unique_id.is_valid()

        # print("\nEXISTING", existing_unique_id)
        # print("\nNEW", new_unique_id)
        if existing_unique_id != new_unique_id:
            raise ProcessMismatchError(
                "UniqueIDs do not match "
                + str(new_unique_id)
                + " "
                + str(existing_unique_id),
                new_unique_id=new_unique_id,
                existing_unique_id=existing_unique_id,
            )

    def gen_unique_id(self, force_ctime: bool = False) -> UniqueId:
        raise NotImplementedError

    @property
    def unique_id(self) -> UniqueId:
        """
        A valid unique_id for this process_handle. This is not nescessarily the unique_id
        provided during construction.

        """
        raise NotImplementedError

    @property
    def is_definitive(self) -> bool:
        """
        Is this handle type definitive?

        Definitive in this case meaning the underlying OS mechanism guarantees that
        the process we think are referring to is definitely it, and that the identifier
        has not been recycled to refer to another process. Notably, PIDs are not definitive.
        Linux pidfds and Windows process handles are.

        Also, if a process handle was opened against an existing process, we can only guarantee
        that we're referring to the process that the (say) pid used to open it at the time
        referred to - a pid may have already been recycled by that time from when the intended
        process was launched.
        """
        raise NotImplementedError

    @property
    def is_closed(self) -> bool:
        """
        Returns whether this process_handle is closed. Being closed means any underlying
        resources (such as a file descriptor) have been deallocated.

        When a process handle is closed, most methods are unusable, and in general
        the object is now useless.

        Note that process handle being closed (or not) does not indicate anything at all about
        the state of the process being tracked itself. There is generally no correlation
        between the lifetime of the process and the lifetime of a process handle -
        unless kill on close is enabled (which isn't implemented yet).
        """
        raise NotImplementedError

    def _check_not_closed(self) -> None:
        if self.is_closed:
            raise RuntimeError("handle closed")

    def close(self) -> None:
        """
        Close this process handle. Deallocate any underlying resources
        such as file descriptors. When a process handle is closed, most methods are
        unusable, and in general the object is useless.

        Note that unless kill on close is enabled, this has no effect on the process
        being tracked by this handle.

        If close is not called, resources will be eventually cleaned up by __del__,
        but of course it's not defined exactly when this will happen.
        """
        raise NotImplementedError

    def copy(self) -> ProcessHandle:
        """
        Return a deep copy of this handle, including duplicating any underlying
        OS entities (file descriptors, handles, etc).

        Raises if this fd is closed.
        """
        raise NotImplementedError

    def alloc_waiter(self) -> None:
        """
        In some cases, underlying resources are allocated lazily (eg opening file descriptors).

        This method will actually allocate them (if required), and thus trigger any forthcoming
        exceptions that may be looming.
        """
        raise NotImplementedError

    def process_is_alive(self) -> bool:
        """
        A non-blocking check to see if the process is still alive. Ihis case, alive means
        not a zombie - ie a zombie process will return False here.

        Returns true if the process is alive.
        """
        raise NotImplementedError

    def process_exists(self) -> bool:
        """
        A non-blocking check to see if the process still exists. In other words, it'll
        still show up in ps and top output. Note that this does include zombie processes.

        Returns true if process exists.
        """
        raise NotImplementedError

    def process_is_zombie(self) -> bool:
        """
        Returns true if process is a zombie.
        Always returns False on windows.
        """

    def block_while_alive(self, timeout: float = None) -> None:
        """
        Block until the process has completed. Does not perform a wait().
        Process may be a zombie when this returns.

        Because this call doesn't wait(), returncode may not be availble
        when this call returns.
        """
        raise NotImplementedError

    def block_for_wait(self, timeout=None):
        """
        Similar to block_while_alive(), block until the process is completed, and
        thus wait() should return immediately. If available, will return the status
        (ie returncode/signal), even if a wait has yet to complete.

        This method does NOT perform a wait().

        This method will only block - never busy loop. If blocking is not possible,
        it'll raise WaitFailed.

        If this process was created by subprocess, it's better to call
        subprocess.wait() instead.
        """
        raise NotImplementedError

    def _do_waitid(self, wflags):
        """
        Perform the actual waitid() syscall. This a simple wrapper that will use the best
        available identifier when calling waitid() to avoid pid races.

        """
        raise NotImplementedError

    def wait(self, wflags: int):
        """
        Perform a wait() on the process. Note that no locking is performed, so if multiple
        threads call wait, you may need to handle this yourself.

        If this process was created by subprocess, it's better to call
        subprocess.wait() instead.
        """
        raise NotImplementedError

    def terminate(self):
        raise NotImplementedError

    def kill(self):
        raise NotImplementedError

    def send_signal(self, signum) -> None:
        """
        Send a signal to the process.
        """
        raise NotImplementedError


class WindowsProcessHandle(ProcessHandle):
    def __init__(
        self,
        handle: _process_support.Handle,
        pid: int = None,
        unique_id: UniqueId = None,
    ):
        self._handle = handle

        if pid is not None and unique_id is not None:
            if pid != unique_id.pid:
                raise ValueError("specified pid and unique_id do not match")

        if unique_id is not None:
            pid = unique_id.pid

        super().__init__(pid)

        if unique_id is not None:
            # for now, we only know how to handle unique ids created here
            # it's possible we might be able to handle other types, but for now reject them
            if not unique_id.has_key("ctime"):
                raise IncompatibleUniqueIdError("Can't handle this key type", unique_id)

            # ok, we think the unique ids are sane - now compare them
            my_unique_id = self.unique_id
            self._check_unique_id(
                existing_unique_id=unique_id, new_unique_id=my_unique_id
            )

    def copy(self) -> WindowsProcessHandle:
        self._check_not_closed()
        other = WindowsProcessHandle(handle=self._handle.copy(), pid=self.pid)
        return other

    def close(self) -> None:
        if self._handle is not None:
            self._handle.close()
            self._handle = None

    @property
    def is_closed(self):
        return self._handle is None

    @property
    def is_definitive(self):
        return True

    @property
    def handle(self) -> _process_support.Handle:
        self._check_not_closed()
        return self._handle

    def _get_pid(self) -> int:
        self._check_not_closed()
        return _winapi.GetProcessId(self._handle)

    def _poll_process(self, timeout: float = None) -> None:
        if timeout is None:
            timeout_millis = _winapi.INFINITE
        elif timeout < 0.0:
            # this matches the old behaviour of Popen._wait()
            timeout_millis = 0
        else:
            timeout_millis = int(timeout * 1000)

        result = _winapi.WaitForSingleObject(self._handle, timeout_millis)

        if result == _winapi.WAIT_TIMEOUT:
            raise TimeoutExpired(timeout)
        elif result == _winapi.WAIT_OBJECT_0:
            # process completed
            self._returncode = _winapi.GetExitCodeProcess(self._handle)
            return self.returncode
        else:
            # winapi.WaitForSingleObject() raises on WAIT_FAILED, so
            # we shouldn't get here
            assert False, "unexpected return %d from WaitForSingleObject()" % result

    def process_exists(self) -> bool:
        try:
            self._poll_process(0)
            return False
        except TimeoutExpired:
            return True

    def process_is_alive(self) -> bool:
        # On Windows, as there's no zombies, process_is_alive() and process_exists()
        # are the same thing.
        return self.process_exists()

    def process_is_zombie(self) -> bool:
        return False

    def block_while_alive(self, timeout: float = None):
        self._poll_process(timeout)

    def block_for_wait(self, timeout: float = None):
        return self._poll_process(timeout)

    def wait(self, wflags: int) -> int:
        return self._poll_process()

    # stolen from subprocess
    def send_signal(self, sig):
        """Send a signal to the process."""
        # Don't signal a process that we know has already died.
        if self.returncode is not None:
            return
        if sig == signal.SIGTERM:
            self.terminate()
        elif sig == signal.CTRL_C_EVENT:
            os.kill(self.pid, signal.CTRL_C_EVENT)
        elif sig == signal.CTRL_BREAK_EVENT:
            os.kill(self.pid, signal.CTRL_BREAK_EVENT)
        else:
            raise ValueError("Unsupported signal: {}".format(sig))

    # stolen from subprocess
    def terminate(self):
        """Terminates the process."""
        # Don't terminate a process that we know has already died.
        if self.returncode is not None:
            return
        try:
            _winapi.TerminateProcess(self._handle, 1)
        except PermissionError:
            # ERROR_ACCESS_DENIED (winerror 5) is received when the
            # process already died.
            rc = _winapi.GetExitCodeProcess(self._handle)
            if rc == _winapi.STILL_ACTIVE:
                raise
            self._returncode = rc

    kill = terminate

    def gen_unique_id(self, force_ctime: bool = False) -> UniqueId:
        self._check_not_closed()
        u = UniqueId(self.pid)
        ctime, _, _, _ = _winapi.GetProcessTimes(self._handle)
        u.addElem("ctime", str(ctime))
        return u

    @property
    def unique_id(self) -> UniqueId:
        return self.gen_unique_id()


class PosixProcessHandle(ProcessHandle):
    def __init__(self, pid: int = None, mask_status: bool = True):
        super().__init__(pid)
        self._mask_status = mask_status

    def terminate(self):
        """Terminate the process with SIGTERM"""
        self.send_signal(signal.SIGTERM)

    def kill(self):
        """Kill the process with SIGKILL"""
        self.send_signal(signal.SIGKILL)

    def _do_waitid(self, wflags):
        return os.waitid(os.P_PID, self.pid, wflags)

    def _handle_wait_result(self, wait_result):
        assert wait_result is not None
        # CLD_CONTINUED isn't handled, but we shouldn't get that, as
        # we don't ask for WCONTINUED
        assert wait_result.si_code != os.CLD_CONTINUED

        # Keep only the lowest 8 bits of status.
        # This provides compatibility with waitpid() / WEXITSTATUS; This code is a
        # replacement for code which usees waitpid(), so do this by default (though
        # freeBSD's WEXITSTATUS seems to only give the lowest 7 bits).
        #
        # This is problematic when a process returns (say) -1. exit(3) takes an int param
        # and thus it gets extended out to 0xffffffff, and FreeBSD's waitid returns all 32
        # bits of this. MacOS's returns 24 bits. waitpid() on these platforms still only
        # returns 8 bits, due to the nature of how WEXITSTATUS works. Thus changing the
        # syscall used here changes the return. Linux seems to mask si_status off to 8 bits.
        #
        # Posix specifies -
        # "The low-order 8 bits of the status argument that the process passed to _Exit(),
        # _exit(), or exit(), or the low-order 8 bits of the value the process returned
        # from main() "
        #
        # set _mask_status to False if you want the whole value
        status = wait_result.si_status
        if self._mask_status:
            status = status & 0xFF

        if wait_result.si_code == os.CLD_EXITED:
            return status
        elif wait_result.si_code in (os.CLD_KILLED, os.CLD_DUMPED, os.CLD_STOPPED):
            return -status
        else:
            raise ValueError("unknown si_code %d" % wait_result.si_code)

    def wait(self, wflags: int):
        self._check_not_closed()
        self._check_for_mismatch()

        wait_count = 0
        while True:
            wait_result = self._do_waitid(wflags)

            # os.waitid() catches the si_pid == 0 case, and returns None instead.
            if wait_result is None:
                # don't loop for WNOHANG
                if wflags & os.WNOHANG:
                    return None
                # loop if 0 as waitpid has been known to
                # return 0 even without WNOHANG in odd situations.
                # http://bugs.python.org/issue14396.
                wait_count += 1
                if wait_count > 5:
                    raise WaitFailed("too many waitid() failures")

                continue

            assert wait_result.si_pid != 0
            assert wait_result.si_pid == self.pid

            ret = self._handle_wait_result(wait_result)
            self._returncode = ret
            # print("\nWAIT RESULT", wait_result, "ret", ret)
            return ret


class FdProcessHandle(PosixProcessHandle):
    """
    Process handle type for file descriptor based handles
    """

    def __init__(self, fd, pid=None):
        if not isinstance(fd, _process_support.FdWrapper):
            self._fd = _process_support.FdWrapper(fd)
        else:
            self._fd = fd

        super().__init__(pid=pid)

    def close(self):
        if self._fd is not None:
            self._fd.close()
            self._fd = None

    @property
    def is_closed(self):
        return self._fd is None

    @property
    def is_definitive(self):
        return True

    @property
    def fileno(self) -> int:
        """Return the underlying file descriptor. Will raise if the handle is closed"""
        self._check_not_closed()
        return self._fd.fileno()

    def __repr__(self) -> str:
        fd_str = str(self._fd.fileno()) if not self.is_closed else "closed"
        return "%s(%s)" % (self.__class__.__name__, fd_str)

    def _read_poll_fd(self, timeout: float = None) -> int:
        """poll() the fd for readability. timeout in seconds"""
        self._check_not_closed()

        poll = select.poll()
        poll.register(self._fd.fileno(), select.POLLIN | select.POLLHUP)

        # convert sec to millisec
        if timeout is not None:
            timeout *= 1000
        ret = poll.poll(timeout)

        # if we get [], the process isn't "readable"
        # which means it's fully alive (non-zombie)
        if len(ret) == 0:
            return None

        assert len(ret) == 1
        fd, event = ret[0]
        assert fd == self._fd.fileno()
        return event

    def _block_until(self, poll_flags: int, timeout: float = None) -> None:
        while True:
            e = self._read_poll_fd(timeout)

            if e is not None:
                if (e & poll_flags) != 0:
                    return
            if e is None:
                raise TimeoutExpired(timeout)


class PdProcessHandle(FdProcessHandle):
    """
    Process handle - Process descriptor (FreeBSD) implementation.
    """
    def __init__(self, fd, pid=None):
        super().__init__(fd, pid)
        self._poller = _KQueuePoller(
            select.KQ_FILTER_PROCDESC, self._fd.fileno(), self.pid
        )

    def copy(self) -> PidFdProcessHandle:
        self._check_not_closed()
        other = PdProcessHandle(fd=self._fd.copy(), pid=self.pid)
        return other

    def _check_ctimes_match(self):
        """test that the ctime of the pd matches the pid create time."""
        if not _procinfo.check_pd_ctimes_match(self.pid, self._fd.fileno()):
            raise ProcessMismatchError("pid and pd ctimes don't match")

    def _check_for_mismatch(self):
        # there's some discussion here as to whether a pd can be polled with pdgetpid()
        # to check that the pid's not been reused.. not exactly sure of the outcome...
        # https://lists.cam.ac.uk/sympa/arc/cl-capsicum-discuss/2015-05/msg00012.html
        self._check_ctimes_match()

    def process_is_alive(self) -> bool:
        # paraphrasing man 2 pdfork -
        # "If the owner read, write, and execute bits are set, this
        # means your baby is still alive."
        self._check_not_closed()
        s = os.fstat(self._fd.fileno())
        mask = stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR
        return s.st_mode & mask == mask

    def process_is_zombie(self) -> bool:
        # As far as I can tell, there's no way to detect a zombie via the pd.
        # Neither stat() nor kevent() seem to give anything in this regard.
        # so we have to go via pid.
        # Whilst we strictly don't have to check the pd is closed,
        # if it's closed, we don't know whether the pid has been recycled.

        # FIXME: does this logic hold? ie if we still hold a pd, does that mean
        # the pid has not been recycled?
        # https://marc.info/?l=freebsd-arch&m=176785930331271
        self._check_not_closed()

        # may raise ProcessLookupError
        pstat, ctime = _procinfo_get_pstat_and_ctime(self.pid)
        self._check_for_mismatch(ctime=ctime)
        return pstat == _procinfo.SZOMB

    def process_exists(self) -> bool:
        if self.process_is_alive():
            return True
        # we also have to report a zombie as 'existing'
        try:
            return self.process_is_zombie()
        except ProcessLookupError:
            return False

    def send_signal(self, signum) -> None:
        """
        Send a signal to the process.
        """
        self._check_not_closed()
        signal.pdkill(self._fd.fileno(), signum)

    def _get_pid(self) -> int:
        self._check_not_closed()
        return os.pdgetpid(self._fd.fileno())

    def block_while_alive(self, timeout: float = None):
        self._block_until(select.POLLHUP, timeout)

    def block_for_wait(self, timeout=None):
        self._check_not_closed()

        returncode = self._poller.block_for_wait(timeout)
        return returncode

    @property
    def unique_id(self) -> UniqueId:
        self._check_not_closed()
        return self.gen_unique_id()

    def gen_unique_id(self, force_ctime: bool = False) -> UniqueId:
        return _gen_pid_unique_id(self.pid)

    def _check_for_mismatch(self, ctime=None):
        u = _gen_pid_unique_id(self.pid, ctime)
        self._check_unique_id(existing_unique_id=self.unique_id, new_unique_id=u)

    # def waitid()
    # there's no waitid here, as we just fallback on waitid(pid).
    # however there's talk of a pdwait()
    # https://marc.info/?l=freebsd-arch&m=176785930331271

class PidFdProcessHandle(FdProcessHandle):
    """
    Process handle - pidfd (Linux) implementation.
    """

    def __init__(self, fd: int, pid: int = None, unique_id: UniqueId = None):
        super().__init__(fd=fd, pid=pid)

        # Start checking the provided unique_id matches the pidfd's unique_id (inode).
        # We always generate a new unique_id, even if no unique_id is provided to compare
        # with; this is because this call has the side effect of checking that fd is a
        # pidfd (as opposed to another kind of fd) and will raise if needed.
        my_unique_id = self.unique_id

        if unique_id is not None:
            # we can handle both pidfs and ctime unique_ids
            if unique_id.has_key("pidfs_inode"):
                self._check_pidfs_unique_id(
                    other_unique_id=unique_id, my_unique_id=my_unique_id
                )
            elif unique_id.has_key("ctime"):
                self._check_ctime_unique_id(other_unique_id=unique_id)
            else:
                raise IncompatibleUniqueIdError("Can't handle this key type", unique_id)

    def copy(self) -> PidFdProcessHandle:
        self._check_not_closed()
        other = PidFdProcessHandle(fd = self._fd.copy(), pid = self.pid)
        return other

    def _check_pidfs_unique_id(self, other_unique_id, my_unique_id):
        self._check_unique_id(
            existing_unique_id=other_unique_id, new_unique_id=my_unique_id
        )

    def _check_ctime_unique_id(self, other_unique_id):
        ctime_unique_id = _gen_pid_unique_id(self.pid)
        self._check_unique_id(
            existing_unique_id=other_unique_id, new_unique_id=ctime_unique_id
        )

    @property
    def inode(self) -> int:
        self._check_not_closed()
        if not _USE_PIDFD_GET_INODE:
            raise RuntimeError("pidfd.get_inode() is not available")
        return pidfd.get_inode(self._fd)

    def gen_unique_id(self, force_ctime: bool = False) -> UniqueId:
        self._check_not_closed()

        # build the best unique_id we can
        if not _USE_PIDFD_GET_INODE or force_ctime:
            return _gen_pid_unique_id(self.pid)

        u = UniqueId(self.pid)
        inode = self.inode
        u.addElem("pidfs_inode", inode)
        return u

    @property
    def unique_id(self) -> UniqueId:
        return self.gen_unique_id()

    @property
    def short_form_id(self) -> str:
        """
        Return a unique id string of the form pid:inode, suitable for
        passing to kill(1). Will raise if pidfs is not available - eg Linux < v6.9

        see also - https://github.com/util-linux/util-linux/issues/3252
        """
        return f"{self.pid}:{self.inode}"

    def process_is_zombie(self) -> bool:
        e = self._read_poll_fd(0)

        if e is None:
            # still runnning (non-zombie)
            return False

        # if a poll for readable returns POLLIN (and not POLLHUP), it's a zombie
        if e & select.POLLIN and not e & select.POLLHUP:
            return True

        # process completely gone
        #FIXME: raising here is a bit inconsisten with the other process_ methods
        raise ProcessLookupError("process not found")

    def process_exists(self) -> bool:
        e = self._read_poll_fd(timeout=0)
        # process not readable at all - means it's alive
        if e is None:
            return True

        # if we get HUP, the process is fully gone, including reaping
        if e & select.POLLHUP:
            return False

        # this means the process still exists, but is a zombie. We should
        # have gotten a POLLIN to indicate this.
        assert e & select.POLLIN
        return True

    def process_is_alive(self) -> bool:
        e = self._read_poll_fd(timeout=0)

        # if we got none, the pidfd is not readable at all, which means
        # it's alive (ie non-zombie)
        return e is None

    def send_signal(self, signum: int) -> None:
        """
        Send a signal to the process
        """
        self._check_not_closed()
        signal.pidfd_send_signal(self._fd.fileno(), signum)

    def _get_pid(self) -> int:
        self._check_not_closed()

        if _USE_PIDFD_GETPID:
            return pidfd.pidfd_getpid(self._fd.fileno())

        # We can try to read /proc directly if pidfd_getpid() is not available
        filename = "/proc/self/fdinfo/%d" % self._fd.fileno()
        with open(filename) as f:
            for l in f.readlines():
                k, v = l.split(":")
                if k.strip() == "Pid":
                    return int(v.strip())

        raise RuntimeError("Could not find Pid: value in %s" % filename)

    def block_while_alive(self, timeout: float = None):
        self._check_not_closed()
        # If we get POLLIN, it's a zombie; HUP, it's fully reaped.
        # Either way, it's not alive.
        self._block_until(select.POLLIN | select.POLLHUP, timeout)

    def block_for_wait(self, timeout: float) -> int:
        self._check_not_closed()

        # If the timeout expires, block_while_alive will raise.
        self.block_while_alive(timeout)

        # no child returncode
        return None

    def _do_waitid(self, wflags):
        # we can wait directly on the pidfd
        # FIXME check __fd is still non-blocking mode
        return os.waitid(os.P_PIDFD, self._fd.fileno(), wflags)

    def _check_for_mismatch(self):
        # we're using pidfd for everything, so nothing to check
        pass

    def _try_get_returncode(self):
        if not _USE_PIDFD_GET_INFO_FOR_RETURNCODE:
            return

        i = pidfd.get_info(self._fd, pidfd.PIDFD_INFO_EXIT)
        if "exit_code" in i:
            self._returncode = os.waitstatus_to_exitcode(i['exit_code'])

class PidProcessHandle(PosixProcessHandle):
    """
    Type for all process handles that use a PID as the primary identifier
    """

    def __init__(self, pid=None, unique_id=None):
        if pid is None and unique_id is None:
            raise ValueError(
                "either pid or unique_id must be specififed for PidProcessHandle"
            )

        if pid is not None and unique_id is not None:
            if pid != unique_id.pid:
                raise ValueError("specified pid and unique_id do not match")

        if unique_id is not None:
            pid = unique_id.pid

        self._closed = False
        assert pid > 0
        super().__init__(pid=pid)

        self._poller = _get_pid_poller(self.pid)

        # generate our own unique_id
        self._original_unique_id = self.gen_unique_id(self.pid)

        # compare the provided unique_id to ours
        if unique_id is not None:
            # we only know how to handle ctime unique ids
            if not unique_id.has_key("ctime"):
                raise IncompatibleUniqueIdError("Can't handle this key type", unique_id)

            # ok, we think the unique ids are sane - now compare them
            self._check_unique_id(
                existing_unique_id=self._original_unique_id, new_unique_id=unique_id
            )

    def copy(self) -> PidProcessHandle:
        # we don't strictly need to fail copy if we're closed, but we do this to
        # be consistent with other handle types
        self._check_not_closed()
        # pass unique_id to the copy to ensure we don't casually gloss over a
        # pid recycling event, as the new object will create its own unique_id
        other = PidProcessHandle(self.pid, unique_id = self.unique_id)
        return other

    def close(self) -> None:
        if self._poller is not None:
            self._poller.close()
        self._poller = None
        self._closed = True

    @property
    def is_definitive(self):
        return False

    @property
    def is_closed(self):
        return self._closed

    def alloc_waiter(self) -> None:
        self._check_not_closed()
        self._poller.alloc_waiter()

    def _get_pstat(self) -> int:
        self._check_not_closed()
        # may raise ProcessLookupError
        pstat, ctime = _procinfo_get_pstat_and_ctime(self.pid)
        self._check_for_mismatch(ctime=ctime)
        return pstat

    def process_is_alive(self) -> bool:
        try:
            self._get_pstat()
            return self._get_pstat() != _procinfo.SZOMB
        except ProcessLookupError:
            return False

    def process_is_zombie(self) -> bool:
        return self._get_pstat() == _procinfo.SZOMB

    def process_exists(self) -> bool:
        try:
            self._get_pstat()
            return True
        except ProcessLookupError:
            return False

    def block_while_alive(self, timeout: float = None) -> None:
        "returns when the process exits. Will raise on timeout"
        self._check_not_closed()
        self._check_for_mismatch()
        e = self._poller.poll(timeout)

    def block_for_wait(self, timeout: float) -> int:
        self._check_not_closed()
        self._check_for_mismatch()
        return self._poller.block_for_wait(timeout)

    def send_signal(self, signum: int) -> None:
        """
        Send a signal to the process
        """
        self._check_not_closed()
        self._check_for_mismatch()
        os.kill(self.pid, signum)

    @property
    def unique_id(self) -> UniqueId:
        # return a copy to match the semantics of other handle types which
        # generate a handle on demand
        return self._original_unique_id.copy()

    def gen_unique_id(self, force_ctime: bool = False) -> UniqueId:
        return _gen_pid_unique_id(self.pid)

    def __repr__(self) -> str:
        return "%s(%s)" % (self.__class__.__name__, self.pid)

    def _check_for_mismatch(self, ctime=None):
        u = _gen_pid_unique_id(self.pid, ctime)
        self._check_unique_id(existing_unique_id=self.unique_id, new_unique_id=u)


if _mswindows:

    def build_from_handle(
        handle: _process_support.Handle, pid: int = None
    ) -> WindowsProcessHandle:
        return WindowsProcessHandle(handle, pid)

else:

    def build_from_fd(fd: int, pid: int = None) -> FdProcessHandle:
        """
        build_from_fd() - creates a process handle, wrapped around the file descriptor
        *fd*. Optionally, if the pid is known, it can be supplied in *pid*. (Otherwise,
        we'll look it up).

        In general, you probably don't want to call this function. It's used by modules
        that create processes (eg subprocess) that can also create a process file descriptor
        at the same time. FD must be the appropriate type of file descriptor for the given
        platform (eg a pidfd on Linux). Behaviour when any other kind of fd is supplied
        is undefined.

        If you have a pid and want a process handle, you should call build_from_pid().

        Note that this method steals *fd*, and will clean it up when appropraite. It may
        also modify it, for example changing its non-blocking status.

        Returns a process handle object.
        """

        if _USE_PIDFD_OPEN:
            return PidFdProcessHandle(fd, pid)
        elif _USE_PD:
            return PdProcessHandle(fd, pid)
        else:
            raise RuntimeError(
                "Can't create a file descriptor based process handle on this platform"
            )


def build_from_pid(
    pid: int, force_pid_based=False, force_handle_based_if_exists=False, unique_id=None
) -> ProcessHandle:

    if pid is None:
        raise ValueError("pid must not be None")

    if _mswindows:
        handle = _winapi.OpenProcess(_winapi.PROCESS_ALL_ACCESS, False, pid)
        return WindowsProcessHandle(handle=handle, pid=pid, unique_id=unique_id)

    # POSIX types...
    # try the fancy handle types first
    if _USE_PIDFD_OPEN and not force_pid_based:
        try:
            fd = os.pidfd_open(pid)
            return PidFdProcessHandle(fd=fd, pid=pid, unique_id=unique_id)
        except:
            if force_handle_based_if_exists:
                # not allowed to fall back to pid based
                raise

    return PidProcessHandle(pid=pid, unique_id=unique_id)


def build_from_unique_id(unique_id, force_pid_based = False) -> ProcessHandle:
    if _USE_PIDFD_OPEN and not force_pid_based:
        fd = os.pidfd_open(unique_id.pid)
        return PidFdProcessHandle(fd, unique_id=unique_id)

    if _mswindows:
        try:
            handle = _winapi.OpenProcess(
                _winapi.PROCESS_ALL_ACCESS, False, unique_id.pid
            )
        except OSError:
            # translate this to a more sensible exception
            # FIXME check for 87 the parameter is incorrect
            raise ProcessLookupError
        # print("\nHANDLE", handle, "uid", unique_id)
        # .ctime, _, _, _ = _winapi.GetProcessTimes(handle)
        # print("new ctime", ctime)
        return WindowsProcessHandle(handle=handle, unique_id=unique_id)

    return PidProcessHandle(unique_id=unique_id)

if not _mswindows:
    def fork():
        """
        Fork a child process.

        Return None to the child process and a process handle for the child to
        the parent process.
        """
        pid, fd = os.fork_flags(os.FORK_FLAG_OPEN_PROCESS_FD)

        if pid == 0:
            # child
            return None

        if fd != -1:
            wrapped = _process_support.FdWrapper(fd)
            h = build_from_fd(wrapped)
        else:
            h = build_from_pid(pid)

        return h
