import os
import subprocess
import sys
import unittest

try:
    import pidfd
except ModuleNotFoundError:
    pidfd = None

if sys.platform != "linux" or pidfd == None:
    raise unittest.SkipTest("pidfd module unavailable on this platform")

if not hasattr(os, "pidfd_open"):
    raise unittest.SkipTest("test only runs with pidfd support compiled in")

import resource
import _process_support

def _create_process(body):
    p = subprocess.Popen(
        [sys.executable, "-c", body],
        stdout=subprocess.PIPE
    )
    return p

def _create_process_simple():
    body = 'import sys; sys.stdout.write("ahoy there"); sys.exit(5)'
    return _create_process(body)

def _create_process_abort():
    body = 'import os; os.abort()'

    return _create_process(body)
def _create_process_block_10sec():
    body = 'import time; time.sleep(10)'
    return _create_process(body)

def _pidfd_open(pid):
    """open a pidfd and wrap it so it gets cleaned up properly"""
    fd = os.pidfd_open(pid)
    return _process_support.FdWrapper(fd)

# older kernels / glibc versions don't have get_info (or its prerequisites)
@unittest.skipIf(not hasattr(pidfd, "get_info"), "pidfd.get_info() not available")
class GetInfoTest(unittest.TestCase):
    @unittest.skipIf(not hasattr(pidfd, "PIDFD_INFO_PID"), "PIDFD_INFO_PID not available")
    def test_get_info(self):
        # this should probably go in test_subprocess, not here
        with _create_process_simple() as p:
            fd = _pidfd_open(p.pid)
            i = pidfd.get_info(fd, pidfd.PIDFD_INFO_PID)

            self.assertIsNotNone(i)
            self.assertEqual(p.pid, i.pid)

            p.communicate()

    def test_non_pidfd_fd(self):
        # try passing a regular file into get_info
        with open(sys.executable, "rb") as f:
            fd = f.fileno()
            with self.assertRaises(OSError):
                pidfd.get_info(fd)

    @unittest.skipIf(not hasattr(pidfd, "PIDFD_INFO_PID"), "PIDFD_INFO_PID not available")
    def test_get_info_after_close(self):
        with _create_process_block_10sec() as p:
            fd = _pidfd_open(p.pid)
            i = pidfd.get_info(fd, pidfd.PIDFD_INFO_PID)

            self.assertIsNotNone(i)
            self.assertEqual(p.pid, i.pid)

            # kill process...
            p.kill()

            # get_info call should still work, pre-reaping
            i2 = pidfd.get_info(fd, pidfd.PIDFD_INFO_PID)
            self.assertIsNotNone(i2)
            self.assertEqual(p.pid, i2.pid)

            # reap
            p.wait()

            # should now fail
            with self.assertRaises(ProcessLookupError):
                pidfd.get_info(fd)

    @unittest.skipIf(
        not hasattr(pidfd, "PIDFD_INFO_EXIT") or not hasattr(pidfd, "PIDFD_INFO_COREDUMP"),
            "pidfd required flags not available")
    def test_get_info_extra_flags(self):
        with _create_process_simple() as p:
            fd = _pidfd_open(p.pid)
            i = pidfd.get_info(fd, pidfd.PIDFD_INFO_PID)

            self.assertIsNotNone(i)
            self.assertEqual(p.pid, i.pid)

            # try some extra flags, they won't make a difference yet, but the
            # call should work.
            i2 = pidfd.get_info(fd,
                pidfd.PIDFD_INFO_PID | pidfd.PIDFD_INFO_EXIT | pidfd.PIDFD_INFO_COREDUMP)
            self.assertIsNotNone(i2)
            self.assertEqual(p.pid, i2.pid)

            p.wait()

            # process is a zombie, so almost no info is available, even though we ask for pid.
            # This call should not raise, however.
            i3 = pidfd.get_info(fd,
                pidfd.PIDFD_INFO_PID | pidfd.PIDFD_INFO_EXIT | pidfd.PIDFD_INFO_COREDUMP)

            self.assertIsNone(i3.pid)

            # ... but we should have an exit status
            e = i3.exit_code
            self.assertTrue(os.WIFEXITED(e))
            self.assertEqual(5, os.WEXITSTATUS(e))

            # now the process has been reaped, calling get_info without INFO_EXIT/COREDUMP
            # will raise a ProcessLookupError
            with self.assertRaises(ProcessLookupError):
                pidfd.get_info(fd)

    @unittest.skipIf(
        not hasattr(pidfd, "PIDFD_INFO_EXIT") or not hasattr(pidfd, "PIDFD_INFO_COREDUMP"),
            "pidfd required flags not available")
    def test_get_info_abort(self):
        # let's not actually coredump
        soft, hard = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, hard))

        try:
            with _create_process_abort() as p:
                fd = _pidfd_open(p.pid)

                p.communicate()
                # process now dead

                check_sig = False
                flags = pidfd.PIDFD_INFO_PID | pidfd.PIDFD_INFO_EXIT | pidfd.PIDFD_INFO_COREDUMP;

                if hasattr(pidfd, "PIDFD_INFO_COREDUMP_SIGNAL"):
                    check_sig = True
                    flags |= pidfd.PIDFD_INFO_COREDUMP_SIGNAL

                if hasattr(pidfd, "PIDFD_INFO_COREDUMP_MASK"):
                    check_sig = True
                    flags |= pidfd.PIDFD_INFO_COREDUMP_MASK

                i2 = pidfd.get_info(fd, flags)
                self.assertIsNotNone(i2)

                # process is a zombie, so almost no info is available
                self.assertIsNone(i2.pid)

                # ... but we should have a signaled exit status
                e = i2.exit_code
                self.assertTrue(os.WIFSIGNALED(e))
                self.assertEqual(6, os.WTERMSIG(e))

                if check_sig:
                    # FIXME:implement - I'm not 100% sure what to check for here
                    pass

                with self.assertRaises(ProcessLookupError):
                    pidfd.get_info(fd)
        finally:
            resource.setrlimit(resource.RLIMIT_CORE, (soft, hard))

@unittest.skipIf(not hasattr(pidfd, "get_inode"), "pidfd.get_inode() not availble")
class GetInodeTest(unittest.TestCase):
    def test_get_inode(self):
        fd = _pidfd_open(os.getpid())
        inode = pidfd.get_inode(fd)
        self.assertGreater(inode, 0)

    def test_non_pidfd(self):
        "calling get_inode() on a non-pidfd fd should raise"
        with open(sys.executable, "rb") as f:
            with self.assertRaises(ValueError):
                inode = pidfd.get_inode(f.fileno())
