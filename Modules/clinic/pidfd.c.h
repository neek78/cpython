/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"          // PyGC_Head
#  include "pycore_runtime.h"     // _Py_ID()
#endif
#include "pycore_long.h"          // _PyLong_UnsignedInt_Converter()
#include "pycore_modsupport.h"    // _PyArg_UnpackKeywords()

#if defined(HAVE_PIDFD_GETPID)

PyDoc_STRVAR(pidfd_pidfd_getpid__doc__,
"getpid($module, /, pidfd)\n"
"--\n"
"\n"
"Query the process ID (PID) from process descriptor *pidfd*.\n"
"\n"
"This function gets its information from either /proc or the PIDFD_GET_INFO\n"
"ioctl. Depending on the kernel and/or glibc version, the latter may not be\n"
"possible and thus this function will fail if /proc is not accessible.");

#define PIDFD_PIDFD_GETPID_METHODDEF    \
    {"getpid", _PyCFunction_CAST(pidfd_pidfd_getpid), METH_FASTCALL|METH_KEYWORDS, pidfd_pidfd_getpid__doc__},

static PyObject *
pidfd_pidfd_getpid_impl(PyObject *module, int pidfd);

static PyObject *
pidfd_pidfd_getpid(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 1
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        Py_hash_t ob_hash;
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_hash = -1,
        .ob_item = { &_Py_ID(pidfd), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pidfd", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "getpid",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pidfd;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pidfd = PyLong_AsInt(args[0]);
    if (pidfd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = pidfd_pidfd_getpid_impl(module, pidfd);

exit:
    return return_value;
}

#endif /* defined(HAVE_PIDFD_GETPID) */

#if defined(__NR_pidfd_getfd)

PyDoc_STRVAR(pidfd_pidfd_getfd__doc__,
"getfd($module, /, pidfd, targetfd, flags=0)\n"
"--\n"
"\n"
"Obtain a duplicate of another process\'s file descriptor");

#define PIDFD_PIDFD_GETFD_METHODDEF    \
    {"getfd", _PyCFunction_CAST(pidfd_pidfd_getfd), METH_FASTCALL|METH_KEYWORDS, pidfd_pidfd_getfd__doc__},

static PyObject *
pidfd_pidfd_getfd_impl(PyObject *module, int pidfd, int targetfd,
                       unsigned int flags);

static PyObject *
pidfd_pidfd_getfd(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 3
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        Py_hash_t ob_hash;
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_hash = -1,
        .ob_item = { &_Py_ID(pidfd), &_Py_ID(targetfd), &_Py_ID(flags), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pidfd", "targetfd", "flags", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "getfd",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[3];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 2;
    int pidfd;
    int targetfd;
    unsigned int flags = 0;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 2, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pidfd = PyLong_AsInt(args[0]);
    if (pidfd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    targetfd = PyLong_AsInt(args[1]);
    if (targetfd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    if (!_PyLong_UnsignedInt_Converter(args[2], &flags)) {
        goto exit;
    }
skip_optional_pos:
    return_value = pidfd_pidfd_getfd_impl(module, pidfd, targetfd, flags);

exit:
    return return_value;
}

#endif /* defined(__NR_pidfd_getfd) */

#if defined(PID_FS_MAGIC)

PyDoc_STRVAR(pidfd_pidfd_getinode__doc__,
"get_inode($module, /, fd)\n"
"--\n"
"\n"
"Return the pidfs inode number\n"
"\n"
"Can also be used to test if a given fd is a pidfd - Passing a non-pidfd fd\n"
"will cause ValueError to be raised.");

#define PIDFD_PIDFD_GETINODE_METHODDEF    \
    {"get_inode", _PyCFunction_CAST(pidfd_pidfd_getinode), METH_FASTCALL|METH_KEYWORDS, pidfd_pidfd_getinode__doc__},

static PyObject *
pidfd_pidfd_getinode_impl(PyObject *module, int fd);

static PyObject *
pidfd_pidfd_getinode(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 1
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        Py_hash_t ob_hash;
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_hash = -1,
        .ob_item = { &_Py_ID(fd), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"fd", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_inode",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int fd;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    fd = PyLong_AsInt(args[0]);
    if (fd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = pidfd_pidfd_getinode_impl(module, fd);

exit:
    return return_value;
}

#endif /* defined(PID_FS_MAGIC) */

#if defined(PIDFD_GET_INFO)

PyDoc_STRVAR(pidfd_pidfd_getinfo__doc__,
"get_info($module, /, pidfd, flags=0)\n"
"--\n"
"\n"
"Get all info available for this pidfd from the PIDFD_GET_INFO ioctl.");

#define PIDFD_PIDFD_GETINFO_METHODDEF    \
    {"get_info", _PyCFunction_CAST(pidfd_pidfd_getinfo), METH_FASTCALL|METH_KEYWORDS, pidfd_pidfd_getinfo__doc__},

static PyObject *
pidfd_pidfd_getinfo_impl(PyObject *module, int pidfd, int flags);

static PyObject *
pidfd_pidfd_getinfo(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 2
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        Py_hash_t ob_hash;
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_hash = -1,
        .ob_item = { &_Py_ID(pidfd), &_Py_ID(flags), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pidfd", "flags", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_info",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
    int pidfd;
    int flags = 0;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pidfd = PyLong_AsInt(args[0]);
    if (pidfd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    if (!noptargs) {
        goto skip_optional_pos;
    }
    flags = PyLong_AsInt(args[1]);
    if (flags == -1 && PyErr_Occurred()) {
        goto exit;
    }
skip_optional_pos:
    return_value = pidfd_pidfd_getinfo_impl(module, pidfd, flags);

exit:
    return return_value;
}

#endif /* defined(PIDFD_GET_INFO) */

#if (0) && (defined(__linux__) && defined(PIDFS_IOCTL_MAGIC))

PyDoc_STRVAR(pidfd_pidfd_getnamespace__doc__,
"get_namespace($module, /, pidfd, request)\n"
"--\n"
"\n");

#define PIDFD_PIDFD_GETNAMESPACE_METHODDEF    \
    {"get_namespace", _PyCFunction_CAST(pidfd_pidfd_getnamespace), METH_FASTCALL|METH_KEYWORDS, pidfd_pidfd_getnamespace__doc__},

static int
pidfd_pidfd_getnamespace_impl(PyObject *module, int pidfd, int request);

static PyObject *
pidfd_pidfd_getnamespace(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 2
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        Py_hash_t ob_hash;
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_hash = -1,
        .ob_item = { &_Py_ID(pidfd), &_Py_ID(request), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pidfd", "request", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_namespace",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    int pidfd;
    int request;
    int _return_value;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 2, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pidfd = PyLong_AsInt(args[0]);
    if (pidfd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    request = PyLong_AsInt(args[1]);
    if (request == -1 && PyErr_Occurred()) {
        goto exit;
    }
    _return_value = pidfd_pidfd_getnamespace_impl(module, pidfd, request);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromLong((long)_return_value);

exit:
    return return_value;
}

#endif /* (0) && (defined(__linux__) && defined(PIDFS_IOCTL_MAGIC)) */

#ifndef PIDFD_PIDFD_GETPID_METHODDEF
    #define PIDFD_PIDFD_GETPID_METHODDEF
#endif /* !defined(PIDFD_PIDFD_GETPID_METHODDEF) */

#ifndef PIDFD_PIDFD_GETFD_METHODDEF
    #define PIDFD_PIDFD_GETFD_METHODDEF
#endif /* !defined(PIDFD_PIDFD_GETFD_METHODDEF) */

#ifndef PIDFD_PIDFD_GETINODE_METHODDEF
    #define PIDFD_PIDFD_GETINODE_METHODDEF
#endif /* !defined(PIDFD_PIDFD_GETINODE_METHODDEF) */

#ifndef PIDFD_PIDFD_GETINFO_METHODDEF
    #define PIDFD_PIDFD_GETINFO_METHODDEF
#endif /* !defined(PIDFD_PIDFD_GETINFO_METHODDEF) */

#ifndef PIDFD_PIDFD_GETNAMESPACE_METHODDEF
    #define PIDFD_PIDFD_GETNAMESPACE_METHODDEF
#endif /* !defined(PIDFD_PIDFD_GETNAMESPACE_METHODDEF) */
/*[clinic end generated code: output=fe3fb2198594bc41 input=a9049054013a1b77]*/
