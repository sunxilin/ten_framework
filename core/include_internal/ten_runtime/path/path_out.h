//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/path/path.h"
#include "ten_runtime/ten_env/internal/send.h"

typedef struct ten_path_out_t {
  ten_path_t base;

  ten_env_cmd_result_handler_func_t result_handler;
  void *result_handler_data;
} ten_path_out_t;

TEN_RUNTIME_PRIVATE_API ten_path_out_t *ten_path_out_create(
    ten_path_table_t *table, const char *cmd_name, const char *parent_cmd_id,
    const char *cmd_id, ten_loc_t *src_loc, ten_loc_t *dest_loc,
    ten_env_cmd_result_handler_func_t result_handler,
    void *result_handler_data);

TEN_RUNTIME_PRIVATE_API void ten_path_out_destroy(ten_path_out_t *self);
