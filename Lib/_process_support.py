"""
_process_support module.

Provides resources used by process_handle and subprocess
"""

import os

# use presence of msvcrt to detect Windows-like platforms (see bpo-8110)
try:
    import msvcrt
except ModuleNotFoundError:
    _mswindows = False
else:
    _mswindows = True
    import _winapi

if _mswindows:
    # taken from subprocess
    class Handle(int):
        closed = False

        def close(self, CloseHandle=_winapi.CloseHandle):
            if not self.closed:
                self.closed = True
                CloseHandle(self)

        def detach(self):
            if not self.closed:
                self.closed = True
                return int(self)
            raise ValueError("already closed")

        def copy(self):
            h = _winapi.DuplicateHandle(
                _winapi.GetCurrentProcess(),   # source_process_Handle
                self,                          # source_handle
                _winapi.GetCurrentProcess(),   # target_process_Handle
                0,                             # desired_access
                False,                         # inherit_handle
                _winapi.DUPLICATE_SAME_ACCESS) # options

            return Handle(h)

        def __repr__(self):
            return "%s(%d)" % (self.__class__.__name__, int(self))

        __del__ = close
else:
    class FdWrapper(int):
        """
        Simple class to hold an (int) file descriptor.

        Holds only this int so we can ensure it's cleaned up
        with a __del__() method, and there's no refs
        coming out of this type that might delay cleanup.
        """

        _closed = False

        def close(self):
            if not self._closed:
                # print("Closing FD", self)
                os.close(self)
            self._closed = True

        @property
        def is_closed(self):
            return self._closed

        def fileno(self) -> int:
            if self.is_closed:
                raise RuntimeError("fd is closed")
            return self

        def detach(self) -> int:
            if self.is_closed:
                raise RuntimeError("can't detach a closed fd")
            self._closed = True
            return int(self)

        def copy(self):
            if self.is_closed:
                raise RuntimeError("can't copy a closed fd")
            new_fd = os.dup(self)
            return FdWrapper(new_fd)

        __del__ = close
