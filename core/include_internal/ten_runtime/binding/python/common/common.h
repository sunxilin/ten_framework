//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "include_internal/ten_runtime/binding/python/common/python_stuff.h"
#include "ten_utils/container/list.h"

TEN_RUNTIME_PRIVATE_API void ten_py_initialize_with_config(
    const char *program, ten_list_t *module_search_path);

TEN_RUNTIME_PRIVATE_API void ten_py_set_program_name(const char *program_name);

TEN_RUNTIME_PRIVATE_API void ten_py_set_argv(int argc, char **argv);

TEN_RUNTIME_PRIVATE_API void ten_py_run_file(const char *file_path);

// This function is meant to ensure that the current thread is ready to call
// the Python C API. The return value is a 'handle' to the current thread state
// which should be passed to ten_py_gil_state_release() when the current thread
// is ensured that it will no longer call any Python C API.
//
// When the function returns, the current thread will hold the GIL and can call
// Python C API functions including evaluation of Python code.
//
// This function can be called as many times as desired by a thread as long as
// each call is matched with a call to ten_py_gil_state_release().
TEN_RUNTIME_PRIVATE_API PyGILState_STATE ten_py_gil_state_ensure(void);

TEN_RUNTIME_PRIVATE_API void ten_py_gil_state_release(PyGILState_STATE state);

TEN_RUNTIME_PRIVATE_API bool ten_py_is_holding_gil(void);

TEN_RUNTIME_PRIVATE_API PyThreadState *ten_py_gil_state_get_this_thread_state(
    void);
