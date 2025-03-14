//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/msg/cmd_base/cmd/cmd.h"

typedef struct ten_cmd_stop_graph_t {
  ten_cmd_t cmd_hdr;
  ten_string_t graph_name;  // The target engine to be shut down.
} ten_cmd_stop_graph_t;

TEN_RUNTIME_PRIVATE_API ten_string_t *ten_raw_cmd_stop_graph_get_graph_name(
    ten_cmd_stop_graph_t *self);

TEN_RUNTIME_PRIVATE_API ten_string_t *ten_cmd_stop_graph_get_graph_name(
    ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API void ten_raw_cmd_stop_graph_as_msg_destroy(
    ten_msg_t *self);

TEN_RUNTIME_PRIVATE_API ten_msg_t *
ten_raw_cmd_stop_graph_as_msg_create_from_json(ten_json_t *json,
                                               ten_error_t *err);

TEN_RUNTIME_PRIVATE_API bool ten_raw_cmd_stop_graph_as_msg_init_from_json(
    ten_msg_t *self, ten_json_t *json, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API ten_cmd_stop_graph_t *ten_raw_cmd_stop_graph_create(
    void);

TEN_RUNTIME_PRIVATE_API ten_json_t *ten_raw_cmd_stop_graph_to_json(
    ten_msg_t *self, ten_error_t *err);
