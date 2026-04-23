/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"          // PyGC_Head
#  include "pycore_runtime.h"     // _Py_ID()
#endif
#include "pycore_modsupport.h"    // _PyArg_UnpackKeywords()

#if (defined(__APPLE__) && defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL))

PyDoc_STRVAR(_procinfo_get_kinfo_proc__doc__,
"get_kinfo_proc($module, /, pid)\n"
"--\n"
"\n");

#define _PROCINFO_GET_KINFO_PROC_METHODDEF    \
    {"get_kinfo_proc", _PyCFunction_CAST(_procinfo_get_kinfo_proc), METH_FASTCALL|METH_KEYWORDS, _procinfo_get_kinfo_proc__doc__},

static PyObject *
_procinfo_get_kinfo_proc_impl(PyObject *module, int pid);

static PyObject *
_procinfo_get_kinfo_proc(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_kinfo_proc",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_get_kinfo_proc_impl(module, pid);

exit:
    return return_value;
}

#endif /* (defined(__APPLE__) && defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)) */

#if ((defined(__APPLE__) || defined(__FreeBSD__)) || defined(__NetBSD__))

PyDoc_STRVAR(_procinfo_get_pstat_and_ctime__doc__,
"get_pstat_and_ctime($module, /, pid)\n"
"--\n"
"\n"
"Get a process pstat and create time.\n"
"\n"
"This is a simplified version of get_kinfo_proc() that only returns pstat and\n"
"create time. This avoids building a nested structure and all the work that\n"
"goes into this. This method is used regularly by the process_handle module\n"
"to detect PID reuse, so it should be reasonably cheap to call.\n"
"\n"
"Returns (pstat, ctime.tv_src, ctime.tv_usec)");

#define _PROCINFO_GET_PSTAT_AND_CTIME_METHODDEF    \
    {"get_pstat_and_ctime", _PyCFunction_CAST(_procinfo_get_pstat_and_ctime), METH_FASTCALL|METH_KEYWORDS, _procinfo_get_pstat_and_ctime__doc__},

static PyObject *
_procinfo_get_pstat_and_ctime_impl(PyObject *module, int pid);

static PyObject *
_procinfo_get_pstat_and_ctime(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_pstat_and_ctime",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_get_pstat_and_ctime_impl(module, pid);

exit:
    return return_value;
}

#endif /* ((defined(__APPLE__) || defined(__FreeBSD__)) || defined(__NetBSD__)) */

#if (defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO))

PyDoc_STRVAR(_procinfo_get_bsdinfo__doc__,
"get_bsdinfo($module, /, pid)\n"
"--\n"
"\n");

#define _PROCINFO_GET_BSDINFO_METHODDEF    \
    {"get_bsdinfo", _PyCFunction_CAST(_procinfo_get_bsdinfo), METH_FASTCALL|METH_KEYWORDS, _procinfo_get_bsdinfo__doc__},

static PyObject *
_procinfo_get_bsdinfo_impl(PyObject *module, int pid);

static PyObject *
_procinfo_get_bsdinfo(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_bsdinfo",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_get_bsdinfo_impl(module, pid);

exit:
    return return_value;
}

#endif /* (defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)) */

#if (defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO))

PyDoc_STRVAR(_procinfo_get_bsdshortinfo__doc__,
"get_bsdshortinfo($module, /, pid)\n"
"--\n"
"\n"
"Calls proc_info() giving basic info about the process.");

#define _PROCINFO_GET_BSDSHORTINFO_METHODDEF    \
    {"get_bsdshortinfo", _PyCFunction_CAST(_procinfo_get_bsdshortinfo), METH_FASTCALL|METH_KEYWORDS, _procinfo_get_bsdshortinfo__doc__},

static PyObject *
_procinfo_get_bsdshortinfo_impl(PyObject *module, int pid);

static PyObject *
_procinfo_get_bsdshortinfo(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_bsdshortinfo",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_get_bsdshortinfo_impl(module, pid);

exit:
    return return_value;
}

#endif /* (defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)) */

#if (defined(__APPLE__) && defined(HAVE_PROC_PIDINFO))

PyDoc_STRVAR(_procinfo_get_uniqidentifierinfo__doc__,
"get_uniqidentifierinfo($module, /, pid)\n"
"--\n"
"\n");

#define _PROCINFO_GET_UNIQIDENTIFIERINFO_METHODDEF    \
    {"get_uniqidentifierinfo", _PyCFunction_CAST(_procinfo_get_uniqidentifierinfo), METH_FASTCALL|METH_KEYWORDS, _procinfo_get_uniqidentifierinfo__doc__},

static PyObject *
_procinfo_get_uniqidentifierinfo_impl(PyObject *module, int pid);

static PyObject *
_procinfo_get_uniqidentifierinfo(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "get_uniqidentifierinfo",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_get_uniqidentifierinfo_impl(module, pid);

exit:
    return return_value;
}

#endif /* (defined(__APPLE__) && defined(HAVE_PROC_PIDINFO)) */

#if (HAVE_KINFO_GETPROC)

PyDoc_STRVAR(_procinfo_kinfo_getproc__doc__,
"kinfo_getproc($module, /, pid)\n"
"--\n"
"\n"
"Kinfo_gerproc (FreeBSD)");

#define _PROCINFO_KINFO_GETPROC_METHODDEF    \
    {"kinfo_getproc", _PyCFunction_CAST(_procinfo_kinfo_getproc), METH_FASTCALL|METH_KEYWORDS, _procinfo_kinfo_getproc__doc__},

static PyObject *
_procinfo_kinfo_getproc_impl(PyObject *module, int pid);

static PyObject *
_procinfo_kinfo_getproc(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "kinfo_getproc",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int pid;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = _procinfo_kinfo_getproc_impl(module, pid);

exit:
    return return_value;
}

#endif /* (HAVE_KINFO_GETPROC) */

#if defined(__FreeBSD__)

PyDoc_STRVAR(_procinfo_check_pd_ctimes_match__doc__,
"check_pd_ctimes_match($module, /, pid, pd)\n"
"--\n"
"\n"
"Check that the ctime of the pid matches the ctime of the process descriptor.");

#define _PROCINFO_CHECK_PD_CTIMES_MATCH_METHODDEF    \
    {"check_pd_ctimes_match", _PyCFunction_CAST(_procinfo_check_pd_ctimes_match), METH_FASTCALL|METH_KEYWORDS, _procinfo_check_pd_ctimes_match__doc__},

static int
_procinfo_check_pd_ctimes_match_impl(PyObject *module, int pid, int pd);

static PyObject *
_procinfo_check_pd_ctimes_match(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(pid), &_Py_ID(pd), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"pid", "pd", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "check_pd_ctimes_match",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[2];
    int pid;
    int pd;
    int _return_value;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 2, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    pid = PyLong_AsInt(args[0]);
    if (pid == -1 && PyErr_Occurred()) {
        goto exit;
    }
    pd = PyLong_AsInt(args[1]);
    if (pd == -1 && PyErr_Occurred()) {
        goto exit;
    }
    _return_value = _procinfo_check_pd_ctimes_match_impl(module, pid, pd);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyBool_FromLong((long)_return_value);

exit:
    return return_value;
}

#endif /* defined(__FreeBSD__) */

#if (defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL))

PyDoc_STRVAR(_procinfo_get_boottime__doc__,
"get_boottime($module, /)\n"
"--\n"
"\n"
"Check that the ctime of the pid matches the ctime of the process descriptor.");

#define _PROCINFO_GET_BOOTTIME_METHODDEF    \
    {"get_boottime", (PyCFunction)_procinfo_get_boottime, METH_NOARGS, _procinfo_get_boottime__doc__},

static PyObject *
_procinfo_get_boottime_impl(PyObject *module);

static PyObject *
_procinfo_get_boottime(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return _procinfo_get_boottime_impl(module);
}

#endif /* (defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)) */

PyDoc_STRVAR(_procinfo_pstat_is_zombie__doc__,
"pstat_is_zombie($module, /, p_stat)\n"
"--\n"
"\n"
"Return whether this pstat value indicates a zombie process");

#define _PROCINFO_PSTAT_IS_ZOMBIE_METHODDEF    \
    {"pstat_is_zombie", _PyCFunction_CAST(_procinfo_pstat_is_zombie), METH_FASTCALL|METH_KEYWORDS, _procinfo_pstat_is_zombie__doc__},

static int
_procinfo_pstat_is_zombie_impl(PyObject *module, int p_stat);

static PyObject *
_procinfo_pstat_is_zombie(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
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
        .ob_item = { &_Py_ID(p_stat), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"p_stat", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "pstat_is_zombie",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int p_stat;
    int _return_value;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser,
            /*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
    if (!args) {
        goto exit;
    }
    p_stat = PyLong_AsInt(args[0]);
    if (p_stat == -1 && PyErr_Occurred()) {
        goto exit;
    }
    _return_value = _procinfo_pstat_is_zombie_impl(module, p_stat);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyBool_FromLong((long)_return_value);

exit:
    return return_value;
}

#ifndef _PROCINFO_GET_KINFO_PROC_METHODDEF
    #define _PROCINFO_GET_KINFO_PROC_METHODDEF
#endif /* !defined(_PROCINFO_GET_KINFO_PROC_METHODDEF) */

#ifndef _PROCINFO_GET_PSTAT_AND_CTIME_METHODDEF
    #define _PROCINFO_GET_PSTAT_AND_CTIME_METHODDEF
#endif /* !defined(_PROCINFO_GET_PSTAT_AND_CTIME_METHODDEF) */

#ifndef _PROCINFO_GET_BSDINFO_METHODDEF
    #define _PROCINFO_GET_BSDINFO_METHODDEF
#endif /* !defined(_PROCINFO_GET_BSDINFO_METHODDEF) */

#ifndef _PROCINFO_GET_BSDSHORTINFO_METHODDEF
    #define _PROCINFO_GET_BSDSHORTINFO_METHODDEF
#endif /* !defined(_PROCINFO_GET_BSDSHORTINFO_METHODDEF) */

#ifndef _PROCINFO_GET_UNIQIDENTIFIERINFO_METHODDEF
    #define _PROCINFO_GET_UNIQIDENTIFIERINFO_METHODDEF
#endif /* !defined(_PROCINFO_GET_UNIQIDENTIFIERINFO_METHODDEF) */

#ifndef _PROCINFO_KINFO_GETPROC_METHODDEF
    #define _PROCINFO_KINFO_GETPROC_METHODDEF
#endif /* !defined(_PROCINFO_KINFO_GETPROC_METHODDEF) */

#ifndef _PROCINFO_CHECK_PD_CTIMES_MATCH_METHODDEF
    #define _PROCINFO_CHECK_PD_CTIMES_MATCH_METHODDEF
#endif /* !defined(_PROCINFO_CHECK_PD_CTIMES_MATCH_METHODDEF) */

#ifndef _PROCINFO_GET_BOOTTIME_METHODDEF
    #define _PROCINFO_GET_BOOTTIME_METHODDEF
#endif /* !defined(_PROCINFO_GET_BOOTTIME_METHODDEF) */
/*[clinic end generated code: output=3f0cb150f7c21460 input=a9049054013a1b77]*/
