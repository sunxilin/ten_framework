//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/msg/cmd_base/cmd/cmd.h"

typedef struct ten_cmd_start_graph_t {
  ten_cmd_t cmd_hdr;

  bool long_running_mode;
  ten_string_t predefined_graph_name;

  // ten_shared_ptr_t of ten_extension_group_info_t
  ten_list_t extension_groups_info;
  // ten_shared_ptr_t of ten_extension_info_t
  ten_list_t extensions_info;
} ten_cmd_start_graph_t;

TEN_RUNTIME_PRIVATE_API void ten_cmd_start_graph_get_next_list(
    ten_shared_ptr_t *self, ten_app_t *app, ten_list_t *next);

TEN_RUNTIME_PRIVATE_API ten_list_t *ten_raw_cmd_start_graph_get_extensions_info(
    ten_cmd_start_graph_t *self);

TEN_RUNTIME_PRIVATE_API ten_list_t *ten_cmd_start_graph_get_extensions_info(
    ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API ten_list_t *
ten_raw_cmd_start_graph_get_extension_groups_info(ten_cmd_start_graph_t *self);

TEN_RUNTIME_PRIVATE_API ten_list_t *
ten_cmd_start_graph_get_extension_groups_info(ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API void
ten_cmd_start_graph_add_missing_extension_group_node(ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API bool ten_raw_cmd_start_graph_get_long_running_mode(
    ten_cmd_start_graph_t *self);

TEN_RUNTIME_PRIVATE_API bool ten_cmd_start_graph_get_long_running_mode(
    ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API ten_string_t *
ten_raw_cmd_start_graph_get_predefined_graph_name(ten_cmd_start_graph_t *self);

TEN_RUNTIME_PRIVATE_API ten_string_t *
ten_cmd_start_graph_get_predefined_graph_name(ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API ten_list_t
ten_cmd_start_graph_get_requested_extension_names(ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API void ten_cmd_start_graph_fill_loc_info(
    ten_shared_ptr_t *self, const char *app_uri, const char *graph_name);

TEN_RUNTIME_PRIVATE_API bool ten_raw_cmd_start_graph_as_msg_init_from_json(
    ten_msg_t *self, ten_json_t *json, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API void ten_raw_cmd_start_graph_as_msg_destroy(
    ten_msg_t *self);

TEN_RUNTIME_PRIVATE_API ten_msg_t *
ten_raw_cmd_start_graph_as_msg_create_from_json(ten_json_t *json,
                                                ten_error_t *err);

TEN_RUNTIME_PRIVATE_API ten_list_t
ten_cmd_start_graph_get_extension_addon_and_instance_name_pairs_of_specified_extension_group(
    ten_shared_ptr_t *self, const char *app_uri, const char *graph_name,
    const char *extension_group_name);

TEN_RUNTIME_PRIVATE_API ten_json_t *ten_raw_cmd_start_graph_to_json(
    ten_msg_t *self, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API ten_cmd_start_graph_t *ten_raw_cmd_start_graph_create(
    void);

TEN_RUNTIME_PRIVATE_API ten_msg_t *ten_raw_cmd_start_graph_as_msg_clone(
    ten_msg_t *self, ten_list_t *excluded_field_ids);
