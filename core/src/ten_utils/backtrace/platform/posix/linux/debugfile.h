//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
// This file is modified from
// https://github.com/ianlancetaylor/libbacktrace [BSD license]
//
#include "ten_utils/ten_config.h"

#include "ten_utils/backtrace/backtrace.h"

TEN_UTILS_PRIVATE_API int elf_open_debug_file_by_build_id(
    ten_backtrace_t *self, const char *build_id_data, size_t build_id_size);

TEN_UTILS_PRIVATE_API int elf_open_debug_file_by_debug_link(
    ten_backtrace_t *self, const char *filename, const char *debug_link_name,
    uint32_t debug_link_crc, ten_backtrace_error_func_t error_cb, void *data);
