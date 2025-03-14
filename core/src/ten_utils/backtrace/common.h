//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_utils/ten_config.h"

#include "ten_utils/backtrace/backtrace.h"
#include "ten_utils/log/log.h"

typedef struct ten_backtrace_common_t {
  ten_log_t *log;  // The log used to dump the backtrace.

  ten_backtrace_dump_file_line_func_t dump_cb;
  ten_backtrace_error_func_t error_cb;
  void *cb_data;  // The user-defined argument to the above callback functions.
} ten_backtrace_common_t;

TEN_UTILS_PRIVATE_API ten_backtrace_t *g_ten_backtrace;

TEN_UTILS_PRIVATE_API void ten_backtrace_common_init(
    ten_backtrace_common_t *self, ten_backtrace_dump_file_line_func_t dump_cb,
    ten_backtrace_error_func_t error_cb);

TEN_UTILS_PRIVATE_API void ten_backtrace_common_deinit(ten_backtrace_t *self);

TEN_UTILS_PRIVATE_API int ten_backtrace_default_dump_cb(
    ten_backtrace_t *self, uintptr_t pc, const char *filename, int lineno,
    const char *function, void *data);

TEN_UTILS_PRIVATE_API void ten_backtrace_default_error_cb(ten_backtrace_t *self,
                                                          const char *msg,
                                                          int errnum,
                                                          void *data);
