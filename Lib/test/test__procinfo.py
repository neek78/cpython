import os
import sys
import unittest

if not sys.platform.startswith(("freebsd", "netbsd", "darwin")):
    raise unittest.SkipTest("_procinfo doesn't work on this platform")

import _procinfo

class ProcinfoTest(unittest.TestCase):
    @unittest.skipUnless(sys.platform == 'darwin', 'Darwin specific test')
    def test_get_bsdinfo(self):
        # for now, just a simple check that the module loads and
        # that we can get our own pid's info
        info = _procinfo.get_bsdinfo(os.getpid())
        self.assertEqual(info.pbi_pid, os.getpid())

        info_short = _procinfo.get_bsdshortinfo(os.getpid())
        self.assertEqual(info_short.pbsi_pid, os.getpid())

    @unittest.skipUnless(sys.platform.startswith('freebsd'), 'FreeBSD specific test')
    def test_kinfo_getproc(self):

        info = _procinfo.kinfo_getproc(os.getpid())
        # FIXME: this will break when converted into a structseq
        self.assertEqual(info['ki_pid'], os.getpid())

    def test_ctime(self):
        # process pstat,  start time.. at least check we get something back
        pstat, ctime_sec, ctime_used  = _procinfo.get_pstat_and_ctime(os.getpid())

        if hasattr(_procinfo, "SACTIVE"):
            self.assertEqual(pstat, _procinfo.SACTIVE)
        elif hasattr(_procinfo, "SRUN"):
            self.assertEqual(pstat, _procinfo.SRUN)
