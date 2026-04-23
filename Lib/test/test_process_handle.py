import os
import subprocess
import sys
import unittest
import threading
import time

from test import support
from test.support import warnings_helper

import process_handle


def _create_process(body):
    p = subprocess.Popen(
        [sys.executable, "-c", body],
        stdout=subprocess.PIPE
    )
    return p


def _create_process_simple():
    body = 'import sys; sys.stdout.write("ahoy there")'
    return _create_process(body)


def _create_process_block_20sec():
    body = "import time; time.sleep(20)"
    return _create_process(body)


def _create_process_block_5sec():
    body = "import time; time.sleep(5)"
    return _create_process(body)


def _create_background_waiter(process):
    def thread_fn(p):
        p.wait()

    thread = threading.Thread(target=thread_fn, args=(process,),
                              daemon=True, name="test_process_handle background waiter")
    thread.daemon = True
    return thread


class ProcessHandleTest(unittest.TestCase):
    def test_subprocess_launch_handle(self):
        """test that we get a process_handle for a process we launch"""
        with _create_process_simple() as p:
            h = p.process_handle
            self.assertIsNotNone(h)

            # check pid matches
            self.assertEqual(h.pid, p.pid)
            p.communicate()

    def test_subprocess_attach_handle(self):
        """test that we can attach to a pre-existing process"""
        with _create_process_simple() as p:
            h = process_handle.build_from_pid(p.pid)

            self.assertIsNotNone(h)
            self.assertEqual(h.pid, p.pid)
            p.communicate()

    def test_subprocess_block_until_completed(self):
        # print("My pid is ", os.getpid())
        with _create_process_block_5sec() as p:
            h = p.process_handle

            self.assertTrue(h.process_exists())

            thread = _create_background_waiter(p)

            thread.start()
            h.block_while_alive()

            # should be dead now. Dead includes zombies...
            self.assertFalse(h.process_is_alive())

            # we have to join the waiter thread before checking returncode.
            # block_until_completed() may return any time after the process has
            # exited . This doesn't guarantee p.returncode is available yet.
            thread.join()

            # check there's a return code. This means wait() has completed correctly
            self.assertIsNotNone(p.returncode)

    def test_subprocess_block_until_completed_timeout(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle

            self.assertTrue(h.process_exists())

            with self.assertRaises(process_handle.TimeoutExpired):
                h.block_while_alive(timeout=3)

            # FIXME: check 3 sec actually ticked by

            # we should have timed out, process should still be alive
            self.assertTrue(h.process_is_alive())

            # check there's no return code, process should still be alive
            self.assertIsNone(p.returncode)

            p.kill()

    @unittest.skipIf(sys.platform == "win32", "win32 has no concept of zombie processes")
    def test_zombie_detection(self):
        with _create_process_block_5sec() as p:
            h = p.process_handle

            self.assertTrue(h.process_exists())
            self.assertFalse(h.process_is_zombie())

            h.block_while_alive()

            self.assertTrue(h.process_exists())
            self.assertTrue(h.process_is_zombie())

            p.wait()

            self.assertFalse(h.process_exists())
            with self.assertRaises(ProcessLookupError):
                h.process_is_zombie()

# TODO: test process_exists
#       test process_is_alive

class UniqueIdTest(unittest.TestCase):
    def test_unique_id(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.unique_id
            h2 = process_handle.build_from_unique_id(u)
            self.assertEqual(p.pid, h2.pid)

            # build a new unique_id from new handle, check it matches
            u2 = h2.unique_id
            self.assertEqual(u, u2)

            p.kill()
            p.wait()

    def test_unique_id_force_ctime(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle

            u = h.gen_unique_id(force_ctime=True)
            self.assertIn("ctime", u._elems)
            self.assertNotIn("pidfs_inode", u._elems)

            # Build a new process handle from the ctime unique_id.
            # This should work, even on systems
            # that support pidfs_node unique_ids
            h2 = process_handle.build_from_unique_id(u)
            self.assertIsNotNone(h2)
            self.assertEqual(p.pid, h2.pid)

            p.kill()
            p.wait()

    def test_unique_id_pid_mismatch(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.unique_id

            # put in a junk pid
            u._elems['pid'] += 5

            with self.assertRaises((process_handle.ProcessMismatchError, ProcessLookupError)):
                h2 = process_handle.build_from_unique_id(u)

            p.kill()
            p.wait()

    @unittest.skipUnless(sys.platform == "linux" and process_handle._USE_PIDFD_GET_INODE,
                         "this test specifically tests pidfs inodes")
    def test_unique_id_pidfd_inode_test(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.unique_id

            # put in a junk inode
            u._elems['pidfs_inode'] += 1

            with self.assertRaises(process_handle.ProcessMismatchError):
                h2 = process_handle.build_from_unique_id(u)

            p.kill()
            p.wait()

    def test_unique_id_ctime__test(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.gen_unique_id(force_ctime=True)

            # put in a junk ctime
            u._elems['ctime'] = "bananas"

            with self.assertRaises(process_handle.ProcessMismatchError):
                h2 = process_handle.build_from_unique_id(u)

            p.kill()
            p.wait()

    def test_corrupt_unique_id_missing_key(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.unique_id

            self.assertTrue(u.is_valid())

            # remove an essential part of the identifier so there's not enough
            # info left to reliably match
            if 'pidfs_inode' in u._elems:
                del u._elems['pidfs_inode']
            elif 'ctime' in u._elems:
                del u._elems['ctime']
            else:
                self.fail("Don't know how to handle this unique_id " + str(unique_id))

            self.assertFalse(u.is_valid())

            with self.assertRaises(process_handle.UniqueIdError):
                h2 = process_handle.build_from_unique_id(u)

            p.kill()
            p.wait()

    def test_corrupt_unique_id_junk_data(self):
        with _create_process_block_20sec() as p:
            h = p.process_handle
            u = h.unique_id

            self.assertTrue(u.is_valid())

            # mess with a key to cause a mismatch
            if 'pidfs_inode' in u._elems:
                u._elems['pidfs_inode'] += 25
            elif 'ctime' in u._elems:
                u._elems['ctime'] = "23452"  # ctime is stored as a str
            else:
                self.fail("Don't know how to handle this unique_id " + str(unique_id))

            # should still be valid - it's "structurally valid". The data is junk,
            # but we won't know that until testing it against a real process handle
            self.assertTrue(u.is_valid())

            with self.assertRaises(process_handle.ProcessMismatchError):
                h2 = process_handle.build_from_unique_id(u)

            p.kill()
            p.wait()

    @unittest.skipUnless(process_handle._USE_PIDFD_GET_INODE,
                         "short_form_id() needs pidfd.get_inode")
    def test_short_form_id(self):
        h = process_handle.build_from_pid(os.getpid())

        if not isinstance(h, process_handle.PidFdProcessHandle):
            self.skipTest("test only applies to PidFdProcessHandles")

        #FIXME: actually test something here
        s = h.short_form_id

class CTimeTest(unittest.TestCase):
    @unittest.skipIf(sys.platform == "win32", "not implemented on win32")
    def test_get_ctime(self):
        ctime = process_handle._get_ctime(os.getpid())

    def test_get_ctime_with_space(self):
        #FIXME: test launching a command with spaces in the name
        pass

class BootTimeTest(unittest.TestCase):
    def test_boottime(self):
        bt = process_handle._SYSTEM_BOOTTIME
        self.assertIsNotNone(bt)
        self.assertIsInstance(bt, int)

        # fairly low tech, but let's just check the time is in the past
        now = time.time()
        self.assertLess(bt, now)

class ForkTest(unittest.TestCase):
    # FIXME: is it safe to ignore these thread warnings?
    # it seems like the thread in question is created in faulthandler
    @warnings_helper.ignore_fork_in_thread_deprecation_warnings()
    @support.requires_fork()
    def test_fork(self):
        h = process_handle.fork()
        if h is None:
            # child
            os.execve(sys.executable, [sys.executable, '-c', 'pass'], os.environ)
        else:
            # parent
            self.assertEqual(h.wait(os.WEXITED), 0)

class ReturncodeTest(unittest.TestCase):
    #FIXME: implement
    pass

class DupTest(unittest.TestCase):
    def test_copy(self):
        with _create_process_block_20sec() as p:
        #p = _create_process_block_20sec()
            h = p.process_handle
            h2 = h.copy()

            if hasattr(h, "_fd"):
                assert h._fd != h2._fd

            if hasattr(h, "_handle"):
                assert h._handle != h2._handle

            h3 = h2.copy()

            h2.close()

            self.assertFalse(h3.is_closed)
            self.assertTrue(h3.process_is_alive())

            h3.close()
            p.kill()
            p.wait()

    def test_copy_closed_handle(self):
        with _create_process_block_20sec() as p:
        #p = _create_process_block_20sec()
            h = p.process_handle
            h2 = h.copy()
            h2.close()
            with self.assertRaises(RuntimeError):
                h3 = h2.copy()

            p.kill()
            p.wait()
