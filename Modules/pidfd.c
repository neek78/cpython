/* pidfd module */

// Argument Clinic uses the internal C API
#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "Python.h"

#include <errno.h>
#include <sys/ioctl.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

#ifdef HAVE_SYS_PIDFD_H
#  include <sys/pidfd.h>
#endif

#ifdef HAVE_LINUX_MAGIC_H
#  include <linux/magic.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* we need either sys/vfs.h or sys/statfs.h for fstatfs() - not sure if there's
 * ever a case where we'd ever get just one, but play it safe */
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif

/*[clinic input]
module pidfd
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=f37c8efb7b935927]*/

#include "clinic/pidfd.c.h"

typedef struct {
    PyObject *PidFdGetInfoResultType;
} _pidfd_state;

static _pidfd_state*
_get_pidfd_state(PyObject *module)
{
    void *state = PyModule_GetState(module);
    assert(state != NULL);
    return (_pidfd_state*)state;
}

static int
_pidfd_traverse(PyObject *module, visitproc visit, void *arg)
{
    _pidfd_state *state = _get_pidfd_state(module);
    Py_VISIT(state->PidFdGetInfoResultType);
    return 0;
}

static int
_pidfd_clear(PyObject *module)
{
    _pidfd_state *state = _get_pidfd_state(module);
    Py_CLEAR(state->PidFdGetInfoResultType);
    return 0;
}

static void
_pidfd_free(void *module)
{
   _pidfd_clear((PyObject *)module);
}

#if defined(HAVE_PIDFD_GETPID)
/*[clinic input]
pidfd.getpid as pidfd_pidfd_getpid
  pidfd: int

Query the process ID (PID) from process descriptor *pidfd*.

This function gets its information from either /proc or the PIDFD_GET_INFO
ioctl. Depending on the kernel and/or glibc version, the latter may not be
possible and thus this function will fail if /proc is not accessible.

[clinic start generated code]*/

static PyObject *
pidfd_pidfd_getpid_impl(PyObject *module, int pidfd)
/*[clinic end generated code: output=f790ab3062f23c24 input=4bef9ab3ffda5190]*/
{
    pid_t pid = -1;

    Py_BEGIN_ALLOW_THREADS
    pid = pidfd_getpid(pidfd);
    Py_END_ALLOW_THREADS

    if (pid < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    return PyLong_FromPid(pid);
}

#endif


#if defined(__NR_pidfd_getfd)
/*[clinic input]
pidfd.getfd as pidfd_pidfd_getfd
  pidfd: int
  targetfd: int
  flags: unsigned_int = 0

Obtain a duplicate of another process's file descriptor

[clinic start generated code]*/

static PyObject *
pidfd_pidfd_getfd_impl(PyObject *module, int pidfd, int targetfd,
                       unsigned int flags)
/*[clinic end generated code: output=3619737f7102142c input=a9d4deeef7f7d219]*/
{
    /* more recent versions of glibc do have a wrapper fn, but we'll use syscall()
     * for now to support older versions */
    int fd = -1;

    Py_BEGIN_ALLOW_THREADS
    fd = syscall(__NR_pidfd_getfd, pidfd, targetfd, flags);
    Py_END_ALLOW_THREADS

    if (fd < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    return PyLong_FromLong(fd);
}
#endif

#if defined(PID_FS_MAGIC)
/*[clinic input]
pidfd.get_inode as pidfd_pidfd_getinode
  fd: int

Return the pidfs inode number

Can also be used to test if a given fd is a pidfd - Passing a non-pidfd fd
will cause ValueError to be raised.

[clinic start generated code]*/

static PyObject *
pidfd_pidfd_getinode_impl(PyObject *module, int fd)
/*[clinic end generated code: output=47d61cb182116223 input=8ab3dcf48037734d]*/
{
    /* First check this fd is a pidfd using fstatfs() ... */
    struct statfs stfs;
    memset(&stfs, 0, sizeof(stfs));
    int r1 = -1;

    Py_BEGIN_ALLOW_THREADS
    r1 = fstatfs(fd, &stfs);
    Py_END_ALLOW_THREADS

    if (r1 < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    if (stfs.f_type != PID_FS_MAGIC) {
        /* FIXME: maybe a different exception type? */
        PyErr_Format(PyExc_ValueError, "fd is not a pidfd (fd's magic is %x)", stfs.f_type);
        return NULL;
    }

    /* ... now get the inode using stat()*/
    struct stat st;
    memset(&st, 0, sizeof(st));
    int r2 = -1;

    Py_BEGIN_ALLOW_THREADS
    r2 = fstat(fd, &st);
    Py_END_ALLOW_THREADS

    if (r2 < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    static_assert(sizeof(unsigned long long) >= sizeof(st.st_ino),
                  "stat.st_ino is larger than unsigned long long");

    return PyLong_FromUnsignedLongLong(st.st_ino);
}
#endif

#if defined(PIDFD_GET_INFO)

PyDoc_STRVAR(pidfd_getinfo_result__doc__,
    "result struct from pidfd.getinfo()");

static PyStructSequence_Field pidfd_getinfo_result__fields__[] = {
    {"mask", ""},
#ifdef PIDFD_INFO_CGROUPID
    {"cgroupid", ""},
#endif
#ifdef PIDFD_INFO_PID
    {"pid", ""},
#endif
#ifdef PIDFD_INFO_CREDS
    {"tgid", ""},
    {"ppid", ""},
    {"ruid", ""},
    {"rgid", ""},
    {"euid", ""},
    {"egid", ""},
    {"suid", ""},
    {"sgid", ""},
    {"fsuid", ""},
    {"fsgid", ""},
#endif
#ifdef PIDFD_INFO_EXIT
    {"exit_code", ""},
#endif
#ifdef PIDFD_INFO_COREDUMP
    {"coredump_mask", ""},
#endif
#ifdef PIDFD_INFO_COREDUMP_SIGNAL
    {"coredump_signal", ""},
#endif
#ifdef PIDFD_INFO_SUPPORTED_MASK
    {"supported_mask", ""},
#endif
    {0}
};

static PyStructSequence_Desc pidfd_getinfo_result_desc = {
    "pidfd.pidfd_getinfo_result",
    pidfd_getinfo_result__doc__,
    pidfd_getinfo_result__fields__,
    sizeof(pidfd_getinfo_result__fields__) / sizeof(pidfd_getinfo_result__fields__[0]) - 1
};

static void
_fill_pidfd_getinfo_result(int flags, struct pidfd_info* info, PyObject* result)
{
    int pos = 0;

    /* mask is always set */
    PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLongLong(info->mask));

/*[python input]

for s in """

    cgroupid:PyLong_FromUnsignedLongLong:PIDFD_INFO_CGROUPID
    pid:PyLong_FromUnsignedLong:PIDFD_INFO_PID
    tgid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    ppid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    ruid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    rgid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    euid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    egid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    suid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    sgid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    fsuid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    fsgid:PyLong_FromUnsignedLong:PIDFD_INFO_CREDS
    exit_code:PyLong_FromLong:PIDFD_INFO_EXIT
    coredump_mask:PyLong_FromUnsignedLong:PIDFD_INFO_COREDUMP
    coredump_signal:PyLong_FromUnsignedLong:PIDFD_INFO_COREDUMP_SIGNAL
    supported_mask:PyLong_FromUnsignedLongLong:PIDFD_INFO_SUPPORTED_MASK

""".strip().split():
    field, conv_fn, macro = s.split(":")
    print(f"""
#ifdef {macro}
    if((flags & {macro}) && (info->mask & {macro})) {{
        PyStructSequence_SetItem(result, pos++, {conv_fn}(info->{field}));
    }} else {{
        PyStructSequence_SetItem(result, pos++, Py_None);
    }}
#endif""")
[python start generated code]*/

#ifdef PIDFD_INFO_CGROUPID
    if((flags & PIDFD_INFO_CGROUPID) && (info->mask & PIDFD_INFO_CGROUPID)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLongLong(info->cgroupid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_PID
    if((flags & PIDFD_INFO_PID) && (info->mask & PIDFD_INFO_PID)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->pid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->tgid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->ppid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->ruid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->rgid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->euid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->egid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->suid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->sgid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->fsuid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_CREDS
    if((flags & PIDFD_INFO_CREDS) && (info->mask & PIDFD_INFO_CREDS)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->fsgid));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_EXIT
    if((flags & PIDFD_INFO_EXIT) && (info->mask & PIDFD_INFO_EXIT)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromLong(info->exit_code));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_COREDUMP
    if((flags & PIDFD_INFO_COREDUMP) && (info->mask & PIDFD_INFO_COREDUMP)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->coredump_mask));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_COREDUMP_SIGNAL
    if((flags & PIDFD_INFO_COREDUMP_SIGNAL) && (info->mask & PIDFD_INFO_COREDUMP_SIGNAL)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLong(info->coredump_signal));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif

#ifdef PIDFD_INFO_SUPPORTED_MASK
    if((flags & PIDFD_INFO_SUPPORTED_MASK) && (info->mask & PIDFD_INFO_SUPPORTED_MASK)) {
        PyStructSequence_SetItem(result, pos++, PyLong_FromUnsignedLongLong(info->supported_mask));
    } else {
        PyStructSequence_SetItem(result, pos++, Py_None);
    }
#endif
/*[python end generated code: output=cf9eeaca00754773 input=94c172b759366be8]*/
}

static const int PIDFD_INFO_ALL_FLAGS =
#ifdef PIDFD_INFO_PID
        PIDFD_INFO_PID |
#endif
#ifdef PIDFD_INFO_CREDS
        PIDFD_INFO_CREDS |
#endif
#ifdef PIDFD_INFO_CGROUPID
        PIDFD_INFO_CGROUPID |
#endif
#ifdef PIDFD_INFO_EXIT
        PIDFD_INFO_EXIT |
#endif
#ifdef PIDFD_INFO_COREDUMP
        PIDFD_INFO_COREDUMP |
#endif
#ifdef PIDFD_INFO_SUPPORTED_MASK
        PIDFD_INFO_SUPPORTED_MASK |
#endif
#ifdef PIDFD_INFO_COREDUMP_SIGNAL
        PIDFD_INFO_COREDUMP_SIGNAL |
#endif
        0;

/*[clinic input]
pidfd.get_info as pidfd_pidfd_getinfo
  pidfd: int
  flags: int = 0

Get all info available for this pidfd from the PIDFD_GET_INFO ioctl.

[clinic start generated code]*/

static PyObject *
pidfd_pidfd_getinfo_impl(PyObject *module, int pidfd, int flags)
/*[clinic end generated code: output=f65fc0ce0095b9d7 input=d61d2831ec2b8bda]*/
{
    /* check we're only seeing flags we know about.. Newer flags might imply a different
     * size struct pid_info we're not compiled with. */
    if ((flags & PIDFD_INFO_ALL_FLAGS) != flags) {
        PyErr_SetString(PyExc_ValueError, "unexpected flags");
        return NULL;
    }

    struct pidfd_info info = {.mask = flags};
    int ret = -1;

    Py_BEGIN_ALLOW_THREADS
    ret = ioctl(pidfd, PIDFD_GET_INFO, &info);
    Py_END_ALLOW_THREADS

    if (ret < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    PyObject* ty = _get_pidfd_state(module)->PidFdGetInfoResultType;
    PyObject* result = PyStructSequence_New((PyTypeObject *)ty);
    if (!result) {
        return PyErr_NoMemory();
    }

    _fill_pidfd_getinfo_result(flags, &info, result);

    if (PyErr_Occurred()) {
        Py_CLEAR(result);
    }

    return result;
}


#endif

#if 0 // Not finished yet

#if defined(__linux__) && defined(PIDFS_IOCTL_MAGIC)
/*[clinic input]
pidfd.get_namespace as pidfd_pidfd_getnamespace -> int
  pidfd: int
  request: int

[clinic start generated code]*/

static int
pidfd_pidfd_getnamespace_impl(PyObject *module, int pidfd, int request)
/*[clinic end generated code: output=267c6de01296b067 input=81eec98a29cbc097]*/
{
    switch (request) {
        // FIXME: these constants may not be defined, wrap in #ifdef
        case PIDFD_GET_CGROUP_NAMESPACE:
        case PIDFD_GET_IPC_NAMESPACE:
        case PIDFD_GET_MNT_NAMESPACE:
        case PIDFD_GET_NET_NAMESPACE:
        case PIDFD_GET_PID_NAMESPACE:
        case PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE:
        case PIDFD_GET_TIME_NAMESPACE:
        case PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE:
        case PIDFD_GET_USER_NAMESPACE:
        case PIDFD_GET_UTS_NAMESPACE:
            break;

        default:
            PyErr_Format(PyExc_ValueError, "Unexpected request value %d", request);
            return -1;
    };

    int ret = -1;

    Py_BEGIN_ALLOW_THREADS
    ret = ioctl(pidfd, request, 0);
    Py_END_ALLOW_THREADS

    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return -1;
    }

    return ret;
}
#endif
#endif

/* List of functions */

static PyMethodDef pidfd_methods[] = {
    PIDFD_PIDFD_GETFD_METHODDEF
    PIDFD_PIDFD_GETPID_METHODDEF
    PIDFD_PIDFD_GETINFO_METHODDEF
    PIDFD_PIDFD_GETINODE_METHODDEF
    PIDFD_PIDFD_GETNAMESPACE_METHODDEF
    {NULL, NULL}  /* sentinel */
};


/* Module initialisation */

PyDoc_STRVAR(module_doc,
"This module gives access to facilities specific to Linux's pidfd mechanism");

static int
all_ins(PyObject* m)
{

/*[python input]

for s in """
PIDFD_NONBLOCK
PIDFD_THREAD

PIDFD_INFO_PID
PIDFD_INFO_CREDS
PIDFD_INFO_CGROUPID
PIDFD_INFO_EXIT
PIDFD_INFO_COREDUMP
PIDFD_INFO_SUPPORTED_MASK
PIDFD_INFO_COREDUMP_SIGNAL

PIDFD_COREDUMPED
PIDFD_COREDUMP_SKIP
PIDFD_COREDUMP_USER
PIDFD_COREDUMP_ROOT

PIDFD_SELF
PIDFD_SELF_PROCESS

PIDFD_GET_CGROUP_NAMESPACE
PIDFD_GET_IPC_NAMESPACE
PIDFD_GET_MNT_NAMESPACE
PIDFD_GET_NET_NAMESPACE
PIDFD_GET_PID_NAMESPACE
PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE
PIDFD_GET_TIME_NAMESPACE
PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE
PIDFD_GET_USER_NAMESPACE
PIDFD_GET_UTS_NAMESPACE

""".strip().split():
    print(f"#ifdef {s}")
    print(f"    if (PyModule_AddIntMacro(m, {s}) < 0)")
    print(f"        return -1;")
    print("#endif")
[python start generated code]*/
#ifdef PIDFD_NONBLOCK
    if (PyModule_AddIntMacro(m, PIDFD_NONBLOCK) < 0)
        return -1;
#endif
#ifdef PIDFD_THREAD
    if (PyModule_AddIntMacro(m, PIDFD_THREAD) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_PID
    if (PyModule_AddIntMacro(m, PIDFD_INFO_PID) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_CREDS
    if (PyModule_AddIntMacro(m, PIDFD_INFO_CREDS) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_CGROUPID
    if (PyModule_AddIntMacro(m, PIDFD_INFO_CGROUPID) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_EXIT
    if (PyModule_AddIntMacro(m, PIDFD_INFO_EXIT) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_COREDUMP
    if (PyModule_AddIntMacro(m, PIDFD_INFO_COREDUMP) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_SUPPORTED_MASK
    if (PyModule_AddIntMacro(m, PIDFD_INFO_SUPPORTED_MASK) < 0)
        return -1;
#endif
#ifdef PIDFD_INFO_COREDUMP_SIGNAL
    if (PyModule_AddIntMacro(m, PIDFD_INFO_COREDUMP_SIGNAL) < 0)
        return -1;
#endif
#ifdef PIDFD_COREDUMPED
    if (PyModule_AddIntMacro(m, PIDFD_COREDUMPED) < 0)
        return -1;
#endif
#ifdef PIDFD_COREDUMP_SKIP
    if (PyModule_AddIntMacro(m, PIDFD_COREDUMP_SKIP) < 0)
        return -1;
#endif
#ifdef PIDFD_COREDUMP_USER
    if (PyModule_AddIntMacro(m, PIDFD_COREDUMP_USER) < 0)
        return -1;
#endif
#ifdef PIDFD_COREDUMP_ROOT
    if (PyModule_AddIntMacro(m, PIDFD_COREDUMP_ROOT) < 0)
        return -1;
#endif
#ifdef PIDFD_SELF
    if (PyModule_AddIntMacro(m, PIDFD_SELF) < 0)
        return -1;
#endif
#ifdef PIDFD_SELF_PROCESS
    if (PyModule_AddIntMacro(m, PIDFD_SELF_PROCESS) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_CGROUP_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_CGROUP_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_IPC_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_IPC_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_MNT_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_MNT_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_NET_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_NET_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_PID_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_PID_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_TIME_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_TIME_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_USER_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_USER_NAMESPACE) < 0)
        return -1;
#endif
#ifdef PIDFD_GET_UTS_NAMESPACE
    if (PyModule_AddIntMacro(m, PIDFD_GET_UTS_NAMESPACE) < 0)
        return -1;
#endif
/*[python end generated code: output=e62c54281b3502e2 input=7f08b8b2099c0fad]*/

    return 0;
}

static int
pidfd_exec(PyObject *module)
{
    if (all_ins(module) < 0) {
        return -1;
    }

    _pidfd_state* state = _get_pidfd_state(module);

#ifdef PIDFD_GET_INFO
    state->PidFdGetInfoResultType=
        (PyObject *)PyStructSequence_NewType(&pidfd_getinfo_result_desc);

    if (PyModule_AddObjectRef(module, "pidfd_getinfo_result",
                state->PidFdGetInfoResultType) < 0) {
        return -1;
    }
#endif

    return 0;
}

static PyModuleDef_Slot pidfd_slots[] = {
    _Py_ABI_SLOT,
    {Py_mod_exec, pidfd_exec},
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
    {Py_mod_gil, Py_MOD_GIL_NOT_USED},
    {0, NULL}
};

static struct PyModuleDef pidfdmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pidfd",
    .m_doc = module_doc,
    .m_size = sizeof(_pidfd_state),
    .m_methods = pidfd_methods,
    .m_slots = pidfd_slots,
    .m_traverse = _pidfd_traverse,
    .m_clear = _pidfd_clear,
    .m_free = _pidfd_free,
};

PyMODINIT_FUNC
PyInit_pidfd(void)
{
    return PyModuleDef_Init(&pidfdmodule);
}

