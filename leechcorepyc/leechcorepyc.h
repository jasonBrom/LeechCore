//	leechcorepyc.c : Implementation of the LeechCore Python API
//
// (c) Ulf Frisk, 2020-2025
// Author: Ulf Frisk, pcileech@frizk.net
//
#ifndef __LEECHCOREPYC_H__
#define __LEECHCOREPYC_H__

#include <leechcore.h>
#include "oscompatibility.h"

#if !defined(LEECHCOREPYC_HAVE_PYTHON)
#if defined(__has_include)
#if __has_include(<Python.h>)
#define LEECHCOREPYC_HAVE_PYTHON 1
#else
#define LEECHCOREPYC_HAVE_PYTHON 0
#endif
#else
#define LEECHCOREPYC_HAVE_PYTHON 1
#endif
#endif /* !defined(LEECHCOREPYC_HAVE_PYTHON) */

#if LEECHCOREPYC_HAVE_PYTHON
#define PY_SSIZE_T_CLEAN
#define Py_LIMITED_API 0x03060000
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#include <structmember.h>
#define _DEBUG
#else
#include <Python.h>
#include <structmember.h>
#endif
#else
#include <stddef.h>
#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */
typedef ptrdiff_t Py_ssize_t;
typedef struct _fakePyTypeObject PyTypeObject;
typedef struct _fakePyObject {
    Py_ssize_t ob_refcnt;
    PyTypeObject *ob_type;
} PyObject;
struct _fakePyTypeObject {
    int unused;
};
#define PyObject_HEAD Py_ssize_t ob_refcnt; PyTypeObject *ob_type;
#endif

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

extern PyObject *g_pPyType_LeechCore;
extern PyObject *g_pPyType_BarRequest;

typedef struct tdPyObj_LeechCore {
    PyObject_HEAD
    BOOL fValid;
    HANDLE hLC;
    LC_CONFIG cfg;
    PHANDLE phLCkeepalive;
    PyObject *fnBarCB;
    PyObject *fnTlpReadCB;
    PyObject *pyBarDictSingle[6];   // dict of pcie bar info.
    PyObject *pyBarListAll;         // list of pyBarSingle[0..5].
} PyObj_LeechCore;

typedef struct tdPyObj_BarRequest {
    PyObject_HEAD
    BOOL fValid;
    PyObj_LeechCore *pyLC;
    PLC_BAR_REQUEST pReq;
} PyObj_BarRequest;

_Success_(return) BOOL LcPy_BarRequest_InitializeType(PyObject *pModule);

PyObj_BarRequest* LcPy_BarRequest_InitializeInternal(_In_ PyObj_LeechCore *pyLC, _In_ PLC_BAR_REQUEST pReq);

#endif /* __LEECHCOREPYC_H__ */
