/* _procinfo module */

// Argument Clinic uses the internal C API
#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "Python.h"
#include "pycore_moduleobject.h"  // _PyModule_GetState()

#include <stdlib.h>

#ifdef HAVE_LIBPROC_H
#include <libproc.h>
#endif

#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_USER_H
#include <sys/user.h>
#endif

//#ifdef HAVE_SYS_PROC_H
#include <sys/proc.h>
//#endif

/*[clinic input]
module _procinfo
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=e43360017d86df77]*/

#include "clinic/_procinfo.c.h"

typedef struct {
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)
    PyObject *ProcPidtShortBsdInfoResultType;
#endif
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)
    PyObject *ProcPidtBsdInfoResultType;
#endif
} _procinfo_state;

static _procinfo_state*
_get_procinfo_state(PyObject *module)
{
    void *state = _PyModule_GetState(module);
    assert(state != NULL);
    return (_procinfo_state*)state;
}

static int
_procinfo_clear(PyObject *module)
{
    _procinfo_state *state = _get_procinfo_state(module);
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)
    Py_CLEAR(state->ProcPidtShortBsdInfoResultType);
#endif
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)
    Py_CLEAR(state->ProcPidtBsdInfoResultType);
#endif
    (void)state;
    return 0;
}

static int
_procinfo_traverse(PyObject *module, visitproc visit, void *arg)
{
    _procinfo_state *state = _get_procinfo_state(module);
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)
    Py_VISIT(state->ProcPidtShortBsdInfoResultType);
#endif
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)
    Py_VISIT(state->ProcPidtBsdInfoResultType);
#endif
    (void)state;
    return 0;
}

static void
_procinfo_free(void *module)
{
   _procinfo_clear((PyObject *)module);
}

#if defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)

#define KINFO_PROC_AVAILABLE 1

#ifdef __NetBSD__
typedef struct kinfo_proc2 kinfo_type;
#define MIB_ENTRY2 KERN_PROC2
#else
typedef struct kinfo_proc kinfo_type;
#define MIB_ENTRY2 KERN_PROC
#endif

/* use syscall() to fill a struct kinfo_proc */
static int
_get_kinfo_proc(int pid, kinfo_type* buf)
{
    int mib[] = {CTL_KERN, MIB_ENTRY2, KERN_PROC_PID, pid,
#ifdef __NetBSD__
        sizeof(struct kinfo_type), 1
#endif
        };
    const int cnt = sizeof(mib)/sizeof(mib[0]);

    size_t len = sizeof(*buf);
    int ret = -1;

    Py_BEGIN_ALLOW_THREADS
    ret = sysctl(mib, cnt, buf, &len, NULL, 0);
    Py_END_ALLOW_THREADS

    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return -1;
    }

    if (len == 0) {
        errno = ESRCH; //FIXME ugly
        PyErr_SetFromErrno(PyExc_OSError);
        return -1;
    }
    return 0;
}
#endif

#if defined(__APPLE__) && defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)

static PyObject*
_procinfo_fill_kp_proc(const struct extern_proc* ep)
{
    PyObject* out = PyDict_New();
    if (out == NULL) {
        return PyErr_NoMemory();
    }

/*[python input]

for s in """

p_flag:PyLong_FromLong
p_pid:PyLong_FromLong
p_oppid:PyLong_FromLong
p_dupfd:PyLong_FromLong
p_stat:PyLong_FromLong
p_xstat:PyLong_FromUnsignedLong
p_acflag:PyLong_FromUnsignedLong
p_priority:PyLong_FromUnsignedLong
p_usrpri:PyLong_FromUnsignedLong
p_nice:PyLong_FromLong
p_comm:PyUnicode_FromString
p_starttime.tv_sec:PyLong_FromLongLong
p_starttime.tv_usec:PyLong_FromLong

""".strip().split():
    field, conv_fn = s.split(":")
    key=field.replace(".", "_")
    print(f'    PyDict_SetItemString(out, "{key}", {conv_fn}(ep->{field}));')
[python start generated code]*/
    PyDict_SetItemString(out, "p_flag", PyLong_FromLong(ep->p_flag));
    PyDict_SetItemString(out, "p_pid", PyLong_FromLong(ep->p_pid));
    PyDict_SetItemString(out, "p_oppid", PyLong_FromLong(ep->p_oppid));
    PyDict_SetItemString(out, "p_dupfd", PyLong_FromLong(ep->p_dupfd));
    PyDict_SetItemString(out, "p_stat", PyLong_FromLong(ep->p_stat));
    PyDict_SetItemString(out, "p_xstat", PyLong_FromUnsignedLong(ep->p_xstat));
    PyDict_SetItemString(out, "p_acflag", PyLong_FromUnsignedLong(ep->p_acflag));
    PyDict_SetItemString(out, "p_priority", PyLong_FromUnsignedLong(ep->p_priority));
    PyDict_SetItemString(out, "p_usrpri", PyLong_FromUnsignedLong(ep->p_usrpri));
    PyDict_SetItemString(out, "p_nice", PyLong_FromLong(ep->p_nice));
    PyDict_SetItemString(out, "p_comm", PyUnicode_FromString(ep->p_comm));
    PyDict_SetItemString(out, "p_starttime_tv_sec", PyLong_FromLongLong(ep->p_starttime.tv_sec));
    PyDict_SetItemString(out, "p_starttime_tv_usec", PyLong_FromLong(ep->p_starttime.tv_usec));
/*[python end generated code: output=3082a6572610e947 input=e2ab3b2161f51846]*/

    if (PyErr_Occurred()) {
        Py_CLEAR(out);
    }
    return out;
}


/*[clinic input]
_procinfo.get_kinfo_proc
  pid: int

[clinic start generated code]*/

static PyObject *
_procinfo_get_kinfo_proc_impl(PyObject *module, int pid)
/*[clinic end generated code: output=07c812e4cc75a388 input=f965fedc11e5f731]*/
{
    kinfo_type buf;
    if (_get_kinfo_proc(pid, &buf) < 0) {
        // _get_kinfo_proc() sets an exception
        return NULL;
    }

    PyObject* out = PyDict_New();
    if (out == NULL) {
        return PyErr_NoMemory();
    }

    PyObject *kp = _procinfo_fill_kp_proc(&buf.kp_proc);
    if (kp == NULL) {
        Py_CLEAR(out);
        return NULL;
    }

    PyDict_SetItemString(out, "kp_proc", kp);
    Py_DECREF(kp);

    return out;
}

#endif


#if (defined(__APPLE__) || defined(__FreeBSD__)) || defined(__NetBSD__)
/*[clinic input]
_procinfo.get_pstat_and_ctime
  pid: int

Get a process pstat and create time.

This is a simplified version of get_kinfo_proc() that only returns pstat and
create time. This avoids building a nested structure and all the work that
goes into this. This method is used regularly by the process_handle module
to detect PID reuse, so it should be reasonably cheap to call.

Returns (pstat, ctime.tv_src, ctime.tv_usec)

[clinic start generated code]*/

static PyObject *
_procinfo_get_pstat_and_ctime_impl(PyObject *module, int pid)
/*[clinic end generated code: output=55d007322cddd72b input=14da5c4c5e5a2db9]*/
{
    kinfo_type buf;
    if (_get_kinfo_proc(pid, &buf) < 0) {
        return NULL;
    }

#if defined(__APPLE__)
    return Py_BuildValue("ili",
        buf.kp_proc.p_stat,
        buf.kp_proc.p_starttime.tv_sec,
        buf.kp_proc.p_starttime.tv_usec);
#elif defined(__FreeBSD__)
    return Py_BuildValue("iLl",
            buf.ki_stat, buf.ki_start.tv_sec, buf.ki_start.tv_usec);
#elif defined(__NetBSD__)
// FIXME: this seems to be returning a junk pstat value
    return Py_BuildValue("iLl",
            buf.p_stat, buf.p_uctime_sec, buf.p_uctime_usec);
#endif
}

#endif

#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)

PyDoc_STRVAR(proc_pidtbsdinfo_result__doc__,
    "result struct from proc_info(..., PROC_PIDTBSDINFO, ...)\n\n\
    see _procinfo.get_bsdinfo() for more info\n");

static PyStructSequence_Field proc_pidtbsdinfo_result__fields__[] = {
    {"pbi_flags", "64bit; emulated etc"},
    {"pbi_status",},
    {"pbi_xstatus",},
    {"pbi_pid",},
    {"pbi_ppid",},
    {"pbi_uid",},
    {"pbi_gid",},
    {"pbi_ruid",},
    {"pbi_rgid",},
    {"pbi_svuid",},
    {"pbi_svgid",},
    {"pbi_comm",},
    {"pbi_name", "empty if no name is registered"},
    {"pbi_nfiles",},
    {"pbi_pgid",},
    {"pbi_pjobc",},
    {"e_tdev", "controlling tty dev"},
    {"e_tpgid", "tty process group id"},
    {"pbi_nice",},
    {"pbi_start_tvsec",},
    {"pbi_start_tvusec",},
    {0}
};

static PyStructSequence_Desc pidtbsdinfo_result_desc = {
    "_procinfo.pidtbsdinfo_result",
    proc_pidtbsdinfo_result__doc__,
    proc_pidtbsdinfo_result__fields__,
    21
};

/*[clinic input]
_procinfo.get_bsdinfo
  pid: int


[clinic start generated code]*/

static PyObject *
_procinfo_get_bsdinfo_impl(PyObject *module, int pid)
/*[clinic end generated code: output=6f735c0165e7760b input=5d9784b73778f5c7]*/
{
    struct proc_bsdinfo buf = {0};
    int n = -1;

    Py_BEGIN_ALLOW_THREADS
    n = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &buf, sizeof(buf));
    Py_END_ALLOW_THREADS

    if (n < 0 || n != sizeof(buf)) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    PyObject* ty = _get_procinfo_state(module)->ProcPidtBsdInfoResultType;
    PyObject* out = PyStructSequence_New((PyTypeObject *)ty);
    if (!out) {
        return PyErr_NoMemory();
    }

    int pos = 0;

/*[python input]

for s in """

	pbi_flags:PyLong_FromUnsignedLong
	pbi_status:PyLong_FromUnsignedLong
	pbi_xstatus:PyLong_FromUnsignedLong
	pbi_pid:PyLong_FromUnsignedLong
	pbi_ppid:PyLong_FromUnsignedLong
	pbi_uid:PyLong_FromLong
	pbi_gid:PyLong_FromLong
	pbi_ruid:PyLong_FromLong
	pbi_rgid:PyLong_FromLong
	pbi_svuid:PyLong_FromLong
	pbi_svgid:PyLong_FromLong
	pbi_comm:PyUnicode_FromString
	pbi_name:PyUnicode_FromString
	pbi_nfiles:PyLong_FromUnsignedLong
	pbi_pgid:PyLong_FromUnsignedLong
	pbi_pjobc:PyLong_FromUnsignedLong
	e_tdev:PyLong_FromUnsignedLong
	e_tpgid:PyLong_FromUnsignedLong
	pbi_nice:PyLong_FromLong
	pbi_start_tvsec:PyLong_FromUnsignedLongLong
	pbi_start_tvusec:PyLong_FromUnsignedLongLong

""".strip().split():
    field, conv_fn = s.split(":")
    print(f"    PyStructSequence_SetItem(out, pos++, {conv_fn}(buf.{field}));")
[python start generated code]*/
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_flags));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_status));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_xstatus));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_pid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_ppid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_uid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_gid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_ruid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_rgid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_svuid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_svgid));
    PyStructSequence_SetItem(out, pos++, PyUnicode_FromString(buf.pbi_comm));
    PyStructSequence_SetItem(out, pos++, PyUnicode_FromString(buf.pbi_name));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_nfiles));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_pgid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbi_pjobc));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.e_tdev));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.e_tpgid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbi_nice));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLongLong(buf.pbi_start_tvsec));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLongLong(buf.pbi_start_tvusec));
/*[python end generated code: output=e57948c2c64ee255 input=c38f555ee7520650]*/

    if (PyErr_Occurred()) {
        Py_CLEAR(out);
    }
    return out;
}

#endif

#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)

PyDoc_STRVAR(proc_pidt_shortbsdinfo_result__doc__,
    "result struct from proc_info(..., PROC_PIDT_SHORTBSDINFO, ...)\n\n\
    see _procinfo.get_bsdshortinfo() for more info\n");

static PyStructSequence_Field proc_pidt_shortbsdinfo_result__fields__[] = {
    {"pbsi_pid", "process id"},
    {"pbsi_ppid", "process parent id"},
    {"pbsi_pgid", "process pgrp id"},   /* header says "perp id" */
    {"pbsi_status", "p_stat value, SZOMB, SRUN, etc"},
    {"pbsi_comm", "upto 16 characters of process name"},
    {"pbsi_flags", "64bit; emulated etc"},
    {"pbsi_uid", "current uid on process"},
    {"pbsi_gid", "current gid on process"},
    {"pbsi_ruid", "current ruid on process"},
    {"pbsi_rgid", "current tgid on process"},
    {"pbsi_svuid", "current svuid on process"},
    {"pbsi_svgid", "current svgid on process"},
    {0}
};

static PyStructSequence_Desc pidt_shortbsdinfo_result_desc = {
    "_procinfo.pidt_shortbsdinfo_result",
    proc_pidt_shortbsdinfo_result__doc__,
    proc_pidt_shortbsdinfo_result__fields__,
    12
};

/*[clinic input]
_procinfo.get_bsdshortinfo
  pid: int

Calls proc_info() giving basic info about the process.


[clinic start generated code]*/

static PyObject *
_procinfo_get_bsdshortinfo_impl(PyObject *module, int pid)
/*[clinic end generated code: output=e2ac541a4e540b5c input=7470591e1494112c]*/
{
    struct proc_bsdshortinfo buf = {0};
    int n = -1;

    Py_BEGIN_ALLOW_THREADS
    n = proc_pidinfo(pid, PROC_PIDT_SHORTBSDINFO, 0, &buf, sizeof(buf));
    Py_END_ALLOW_THREADS

    if (n < 0 || n != sizeof(buf)) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    PyObject* ty = _get_procinfo_state(module)->ProcPidtShortBsdInfoResultType;
    PyObject* out = PyStructSequence_New((PyTypeObject *)ty);
    if (!out) {
        return PyErr_NoMemory();
    }

    int pos = 0;

/*[python input]

for s in """

    pbsi_pid:PyLong_FromUnsignedLong
    pbsi_ppid:PyLong_FromUnsignedLong
    pbsi_pgid:PyLong_FromUnsignedLong
    pbsi_status:PyLong_FromUnsignedLong
    pbsi_comm:PyUnicode_FromString
    pbsi_flags:PyLong_FromUnsignedLong
    pbsi_uid:PyLong_FromLong
    pbsi_gid:PyLong_FromLong
    pbsi_ruid:PyLong_FromLong
    pbsi_rgid:PyLong_FromLong
    pbsi_svuid:PyLong_FromLong
    pbsi_svgid:PyLong_FromLong

""".strip().split():
    field, conv_fn = s.split(":")
    print(f"    PyStructSequence_SetItem(out, pos++, {conv_fn}(buf.{field}));")
[python start generated code]*/
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbsi_pid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbsi_ppid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbsi_pgid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbsi_status));
    PyStructSequence_SetItem(out, pos++, PyUnicode_FromString(buf.pbsi_comm));
    PyStructSequence_SetItem(out, pos++, PyLong_FromUnsignedLong(buf.pbsi_flags));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_uid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_gid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_ruid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_rgid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_svuid));
    PyStructSequence_SetItem(out, pos++, PyLong_FromLong(buf.pbsi_svgid));
/*[python end generated code: output=bc06a3b532058c78 input=0d11dcff75315261]*/

    if (PyErr_Occurred()) {
        Py_CLEAR(out);
    }
    return out;
}

#endif

#if defined(__APPLE__) && defined(HAVE_PROC_PIDINFO)

#define PROC_PIDUNIQIDENTIFIERINFO      17
struct proc_uniqidentifierinfo {
	uint8_t                 p_uuid[16];             /* UUID of the main executable */
	uint64_t                p_uniqueid;             /* 64 bit unique identifier for process */
	uint64_t                p_puniqueid;            /* unique identifier for process's parent */
	int32_t                 p_idversion;            /* pid version */
	uint32_t                p_reserve2;             /* reserved for future use */
	uint64_t                p_reserve3;             /* reserved for future use */
	uint64_t                p_reserve4;             /* reserved for future use */
};

/*[clinic input]
_procinfo.get_uniqidentifierinfo
  pid: int

[clinic start generated code]*/

static PyObject *
_procinfo_get_uniqidentifierinfo_impl(PyObject *module, int pid)
/*[clinic end generated code: output=ad6102647d8f69ea input=648a3d97671957a4]*/
{
	struct proc_uniqidentifierinfo buf = {0};

    int n = -1;
    Py_BEGIN_ALLOW_THREADS
	n = proc_pidinfo(pid, PROC_PIDUNIQIDENTIFIERINFO, 0, &buf, sizeof(buf));
    Py_END_ALLOW_THREADS

    if (n < 0 || n != sizeof(buf)) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    PyObject* out = PyDict_New();
    if (out == NULL) {
        return PyErr_NoMemory();
    }

#if 0
	uint8_t                 p_uuid[16];             /* UUID of the main executable */
#endif

    PyDict_SetItemString(out, "p_uniqueid", PyLong_FromUnsignedLongLong(buf.p_uniqueid));
    PyDict_SetItemString(out, "p_puniqueid", PyLong_FromUnsignedLongLong(buf.p_puniqueid));
    PyDict_SetItemString(out, "p_idversion", PyLong_FromLong(buf.p_idversion));

    if (PyErr_Occurred()) {
        Py_CLEAR(out);
    }

    return out;

}
#endif

#if HAVE_KINFO_GETPROC
/*[clinic input]
_procinfo.kinfo_getproc
  pid: int


Kinfo_gerproc (FreeBSD)

[clinic start generated code]*/

static PyObject *
_procinfo_kinfo_getproc_impl(PyObject *module, int pid)
/*[clinic end generated code: output=9e1942a47bd0f1e0 input=2e6804f2944f32fd]*/
{
    struct kinfo_proc *p = NULL;

    Py_BEGIN_ALLOW_THREADS
    p = kinfo_getproc(getpid());
    Py_END_ALLOW_THREADS

    if (p == NULL) {
        /* no real info on why it failed, it seems */
        PyErr_SetString(PyExc_RuntimeError, "kinfo_getproc returned NULL");
        return NULL;
    }

    PyObject* out = PyDict_New();
    if (out == NULL) {
        free(p);
        return PyErr_NoMemory();
    } else {
#if 0
	struct	pargs *ki_args;		/* address of command arguments */
	struct	proc *ki_paddr;		/* address of proc */
	struct	user *ki_addr;		/* kernel virtual addr of u-area */
	struct	vnode *ki_tracep;	/* pointer to trace file */
	struct	vnode *ki_textvp;	/* pointer to executable file */
	struct	filedesc *ki_fd;	/* pointer to open file info */
	struct	vmspace *ki_vmspace;	/* pointer to kernel vmspace struct */
	const void *ki_wchan;		/* sleep address */
#endif

    PyDict_SetItemString(out, "ki_pid", PyLong_FromLong(p->ki_pid));
    PyDict_SetItemString(out, "ki_ppid", PyLong_FromLong(p->ki_ppid));
    PyDict_SetItemString(out, "ki_pgid", PyLong_FromLong(p->ki_pgid));
    PyDict_SetItemString(out, "ki_tpgid", PyLong_FromLong(p->ki_tpgid));
    PyDict_SetItemString(out, "ki_sid", PyLong_FromLong(p->ki_sid));
    PyDict_SetItemString(out, "ki_tsid", PyLong_FromLong(p->ki_tsid));
    PyDict_SetItemString(out, "ki_jobc", PyLong_FromLong(p->ki_jobc));


#if 0
	uint32_t ki_tdev_freebsd11;	/* controlling tty dev */
	sigset_t ki_siglist;		/* Signals arrived but not delivered */
	sigset_t ki_sigmask;		/* Current signal mask */
	sigset_t ki_sigignore;		/* Signals being ignored */
	sigset_t ki_sigcatch;		/* Signals being caught by user */
#endif

    PyDict_SetItemString(out, "ki_uid", PyLong_FromUnsignedLong(p->ki_uid));
    PyDict_SetItemString(out, "ki_ruid", PyLong_FromUnsignedLong(p->ki_ruid));
    PyDict_SetItemString(out, "ki_svuid", PyLong_FromUnsignedLong(p->ki_svuid));
    PyDict_SetItemString(out, "ki_rgid", PyLong_FromUnsignedLong(p->ki_rgid));
    PyDict_SetItemString(out, "ki_svgid", PyLong_FromUnsignedLong(p->ki_svgid));
#if 0
	short	ki_ngroups;		/* number of groups */
	gid_t	ki_groups[KI_NGROUPS];	/* groups */
	vm_size_t ki_size;		/* virtual size */
	segsz_t ki_rssize;		/* current resident set size in pages */
	segsz_t ki_swrss;		/* resident set size before last swap */
	segsz_t ki_tsize;		/* text size (pages) XXX */
	segsz_t ki_dsize;		/* data size (pages) XXX */
	segsz_t ki_ssize;		/* stack size (pages) */
	u_short	ki_xstat;		/* Exit status for wait & stop signal */
	u_short	ki_acflag;		/* Accounting flags */
	fixpt_t	ki_pctcpu;	 	/* %cpu for process during ki_swtime */
	u_int	ki_estcpu;	 	/* Time averaged value of ki_cpticks */
	u_int	ki_slptime;	 	/* Time since last blocked */
	u_int	ki_swtime;	 	/* Time swapped in or out */
	u_int	ki_cow;			/* number of copy-on-write faults */
	u_int64_t ki_runtime;		/* Real time in microsec */
#endif
    PyDict_SetItemString(out, "ki_start_tvsec", PyLong_FromLongLong(p->ki_start.tv_sec));
    PyDict_SetItemString(out, "ki_start_tv_usec", PyLong_FromLong(p->ki_start.tv_usec));
#if 0
	struct	timeval ki_childtime;	/* time used by process children */
	long	ki_flag;		/* P_* flags */
	long	ki_kiflag;		/* KI_* flags (below) */
	int	ki_traceflag;		/* Kernel trace points */
#endif
    PyDict_SetItemString(out, "ki_stat", PyLong_FromLong(p->ki_stat));
#if 0
	signed char ki_nice;		/* Process "nice" value */
	char	ki_lock;		/* Process lock (prevent swap) count */
	char	ki_rqindex;		/* Run queue index */
	u_char	ki_oncpu_old;		/* Which cpu we are on (legacy) */
	u_char	ki_lastcpu_old;		/* Last cpu we were on (legacy) */
	char	ki_tdname[TDNAMLEN+1];	/* thread name */
	char	ki_wmesg[WMESGLEN+1];	/* wchan message */
	char	ki_login[LOGNAMELEN+1];	/* setlogin name */
	char	ki_lockname[LOCKNAMELEN+1]; /* lock name */
	char	ki_comm[COMMLEN+1];	/* command name */
	char	ki_emul[KI_EMULNAMELEN+1];  /* emulation name */
	char	ki_loginclass[LOGINCLASSLEN+1]; /* login class */
	char	ki_moretdname[MAXCOMLEN-TDNAMLEN+1];	/* more thread name */
	uint64_t ki_tdev;		/* controlling tty dev */
	int	ki_oncpu;		/* Which cpu we are on */
	int	ki_lastcpu;		/* Last cpu we were on */
	int	ki_tracer;		/* Pid of tracing process */
	int	ki_flag2;		/* P2_* flags */
	int	ki_fibnum;		/* Default FIB number */
	u_int	ki_cr_flags;		/* Credential flags */
	int	ki_jid;			/* Process jail ID */
	int	ki_numthreads;		/* XXXKSE number of threads in total */
	lwpid_t	ki_tid;			/* XXXKSE thread id */
	struct	priority ki_pri;	/* process priority */
	struct	rusage ki_rusage;	/* process rusage statistics */
	/* XXX - most fields in ki_rusage_ch are not (yet) filled in */
	struct	rusage ki_rusage_ch;	/* rusage of children processes */
	struct	pcb *ki_pcb;		/* kernel virtual addr of pcb */
	void	*ki_kstack;		/* kernel virtual addr of stack */
	void	*ki_udata;		/* User convenience pointer */
	struct	thread *ki_tdaddr;	/* address of thread */
	struct	pwddesc *ki_pd;	/* pointer to process paths info */
	void	*ki_uerrmsg;		/* address of the ext err msg place */
	long	ki_sflag;		/* PS_* flags */
	long	ki_tdflags;		/* XXXKSE kthread flag */
#endif
    }

    if (PyErr_Occurred()) {
        Py_CLEAR(out);
    }

    free(p);
    return out;
}

#endif

/* make clinic doesn't seem to see KINFO_PROC_AVAILABLE for some reason */
#if defined(__FreeBSD__) /* && defined(KINFO_PROC_AVAILABLE) */

/*[clinic input]
_procinfo.check_pd_ctimes_match -> bool
  pid: int
  pd: int

Check that the ctime of the pid matches the ctime of the process descriptor.

[clinic start generated code]*/

static int
_procinfo_check_pd_ctimes_match_impl(PyObject *module, int pid, int pd)
/*[clinic end generated code: output=529caf74bb0bba17 input=81088095e1370624]*/
{
    struct kinfo_proc buf;
    if (_get_kinfo_proc(pid, &buf) < 0) {
        return -1;
    }

    struct stat sb;
    if(fstat(pd, &sb) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return -1;
    }

#if 0
    printf("check_ctimes_match %ld %ld %ld %ld\n",
            sb.st_ctim.tv_sec, buf.ki_start.tv_sec,
            sb.st_ctim.tv_nsec, buf.ki_start.tv_usec
          );
#endif

    if (sb.st_ctim.tv_sec != buf.ki_start.tv_sec) {
        return 0;
    }

    /* Sigh. One of these is a timespec, the other a timeval...
     * Though in my testing it seems sb.st.ctim only has usec resolution. */
    return (sb.st_ctim.tv_nsec/1000) == buf.ki_start.tv_usec;
}
#endif

#if defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTL)

/*[clinic input]
_procinfo.get_boottime

Check that the ctime of the pid matches the ctime of the process descriptor.

[clinic start generated code]*/

static PyObject *
_procinfo_get_boottime_impl(PyObject *module)
/*[clinic end generated code: output=5a5c22ca98921543 input=2f5545affcb325ac]*/
{
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    struct timeval boottime;

    size_t len = sizeof(boottime);

    errno = 0;
    int n = -1;

    Py_BEGIN_ALLOW_THREADS
    n = sysctl(mib, 2, &boottime, &len, NULL, 0);
    Py_END_ALLOW_THREADS

    if (n < 0) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    if (len != sizeof(boottime)) {
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    return Py_BuildValue("li", boottime.tv_sec, boottime.tv_usec);
}
#endif

/*[clinic input]
_procinfo.pstat_is_zombie -> bool
    p_stat: int

Return whether this pstat value indicates a zombie process


[clinic start generated code]*/

static int
_procinfo_pstat_is_zombie_impl(PyObject *module, int p_stat)
/*[clinic end generated code: output=bf3e85d659b76599 input=07d52d5d940f324a]*/
{
#ifdef __NetBSD__
    // from sys/proc.h's P_ZOMBIE...
    // FIXME: given we're not actually using the macro, probably should
    // move this to python code to avoid calling into c code for no reason
    return (p_stat == SZOMB || p_stat == SDYING || p_stat == SDEAD);
#else
    return p_stat == SZOMB;
#endif
}


/* Module initialisation */
/* List of functions */

static PyMethodDef _procinfo_methods[] = {
    _PROCINFO_GET_BSDINFO_METHODDEF
        _PROCINFO_GET_BSDSHORTINFO_METHODDEF
        _PROCINFO_GET_UNIQIDENTIFIERINFO_METHODDEF
        _PROCINFO_GET_KINFO_PROC_METHODDEF
        _PROCINFO_GET_PSTAT_AND_CTIME_METHODDEF
        _PROCINFO_GET_BOOTTIME_METHODDEF
        _PROCINFO_KINFO_GETPROC_METHODDEF
        _PROCINFO_CHECK_PD_CTIMES_MATCH_METHODDEF
        _PROCINFO_PSTAT_IS_ZOMBIE_METHODDEF

        {NULL, NULL}  /* sentinel */
};


PyDoc_STRVAR(module_doc,
        "");

    static int
all_ins(PyObject* m)
{

/*[python input]

for s in """

SIDL
SRUN
SSLEEP
SSTOP
SZOMB
SWAIT
SLOCK
SIDL
SACTIVE
SDYING
SDEAD

""".strip().split():
    print(f"#ifdef {s}")
    print(f"    if (PyModule_AddIntMacro(m, {s}) < 0)")
    print(f"        return -1;")
    print(f"#endif")
[python start generated code]*/
#ifdef SIDL
    if (PyModule_AddIntMacro(m, SIDL) < 0)
        return -1;
#endif
#ifdef SRUN
    if (PyModule_AddIntMacro(m, SRUN) < 0)
        return -1;
#endif
#ifdef SSLEEP
    if (PyModule_AddIntMacro(m, SSLEEP) < 0)
        return -1;
#endif
#ifdef SSTOP
    if (PyModule_AddIntMacro(m, SSTOP) < 0)
        return -1;
#endif
#ifdef SZOMB
    if (PyModule_AddIntMacro(m, SZOMB) < 0)
        return -1;
#endif
#ifdef SWAIT
    if (PyModule_AddIntMacro(m, SWAIT) < 0)
        return -1;
#endif
#ifdef SLOCK
    if (PyModule_AddIntMacro(m, SLOCK) < 0)
        return -1;
#endif
#ifdef SIDL
    if (PyModule_AddIntMacro(m, SIDL) < 0)
        return -1;
#endif
#ifdef SACTIVE
    if (PyModule_AddIntMacro(m, SACTIVE) < 0)
        return -1;
#endif
#ifdef SDYING
    if (PyModule_AddIntMacro(m, SDYING) < 0)
        return -1;
#endif
#ifdef SDEAD
    if (PyModule_AddIntMacro(m, SDEAD) < 0)
        return -1;
#endif
/*[python end generated code: output=ec4fab514b82ab7f input=556012d5d4b93376]*/

    return 0;
}


static int
_procinfo_exec(PyObject *module)
{
    if (all_ins(module) < 0) {
        return -1;
    }

    _procinfo_state* state = _get_procinfo_state(module);
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDT_SHORTBSDINFO)
    state->ProcPidtShortBsdInfoResultType =
        (PyObject *)PyStructSequence_NewType(&pidt_shortbsdinfo_result_desc);

    if (PyModule_AddObjectRef(module, "proc_pidt_shortbsdinfo_result",
                state->ProcPidtShortBsdInfoResultType) < 0) {
        return -1;
    }
#endif
#if defined(HAVE_PROC_PIDINFO) && defined(PROC_PIDTBSDINFO)
    state->ProcPidtBsdInfoResultType =
        (PyObject *)PyStructSequence_NewType(&pidtbsdinfo_result_desc);

    if (PyModule_AddObjectRef(module, "proc_pidtbsdinfo_result",
                state->ProcPidtBsdInfoResultType) < 0) {
        return -1;
    }
#endif
    (void)state;

    return 0;
}

static PyModuleDef_Slot _procinfo_slots[] = {
    _Py_ABI_SLOT,
    {Py_mod_exec, _procinfo_exec},
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
    {Py_mod_gil, Py_MOD_GIL_NOT_USED},
    {0, NULL}
};

static struct PyModuleDef _procinfo_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_procinfo",
    .m_doc = module_doc,
    .m_size = sizeof(_procinfo_state),
    .m_methods = _procinfo_methods,
    .m_slots = _procinfo_slots,
    .m_traverse = _procinfo_traverse,
    .m_clear = _procinfo_clear,
    .m_free = _procinfo_free,
};

PyMODINIT_FUNC
PyInit__procinfo(void)
{
    return PyModuleDef_Init(&_procinfo_module);
}

