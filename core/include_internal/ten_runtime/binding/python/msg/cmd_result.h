//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/binding/python/common/python_stuff.h"
#include "include_internal/ten_runtime/binding/python/msg/msg.h"

typedef struct ten_py_cmd_result_t {
  ten_py_msg_t msg;
} ten_py_cmd_result_t;

TEN_RUNTIME_PRIVATE_API PyTypeObject *ten_py_cmd_result_py_type(void);

TEN_RUNTIME_PRIVATE_API bool ten_py_cmd_result_init_for_module(
    PyObject *module);

TEN_RUNTIME_PRIVATE_API ten_py_cmd_result_t *ten_py_cmd_result_wrap(
    ten_shared_ptr_t *cmd);

TEN_RUNTIME_PRIVATE_API void ten_py_cmd_result_invalidate(
    ten_py_cmd_result_t *self);

TEN_RUNTIME_PRIVATE_API PyObject *ten_py_cmd_result_create(PyTypeObject *type,
                                                           PyObject *args,
                                                           PyObject *kw);

TEN_RUNTIME_PRIVATE_API void ten_py_cmd_result_destroy(PyObject *self);

TEN_RUNTIME_PRIVATE_API PyObject *ten_py_cmd_result_get_status_code(
    PyObject *self, PyObject *args);

TEN_RUNTIME_PRIVATE_API PyObject *ten_py_cmd_result_set_status_code(
    PyObject *self, PyObject *args);

TEN_RUNTIME_PRIVATE_API PyObject *ten_py_cmd_result_set_is_final(
    PyObject *self, PyObject *args);

TEN_RUNTIME_PRIVATE_API PyObject *ten_py_cmd_result_get_is_final(
    PyObject *self, PyObject *args);
