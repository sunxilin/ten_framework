//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/path/path.h"

typedef struct ten_path_in_t {
  ten_path_t base;
} ten_path_in_t;

TEN_RUNTIME_PRIVATE_API ten_path_in_t *ten_path_in_create(
    ten_path_table_t *table, const char *cmd_name, const char *parent_cmd_id,
    const char *cmd_id, ten_loc_t *src_loc, ten_loc_t *dest_loc,
    ten_msg_conversion_operation_t *result_conversion);

TEN_RUNTIME_PRIVATE_API void ten_path_in_destroy(ten_path_in_t *self);
