//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "ten_utils/value/value.h"

// {
//   "path": "...",
//   "conversion_mode": "fixed_value",
//   "value": "..."
// }
typedef struct ten_msg_conversion_operation_per_property_rule_fixed_value_t {
  ten_value_t *value;
} ten_msg_conversion_operation_per_property_rule_fixed_value_t;

TEN_RUNTIME_PRIVATE_API void
ten_msg_conversion_operation_per_property_rule_fixed_value_deinit(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self);

TEN_RUNTIME_PRIVATE_API bool
ten_msg_conversion_operation_per_property_rule_fixed_value_convert(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self,
    ten_shared_ptr_t *new_cmd, const char *new_msg_property_path,
    ten_error_t *err);

TEN_RUNTIME_PRIVATE_API bool
ten_msg_conversion_operation_per_property_rule_fixed_value_from_json(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self,
    ten_json_t *json, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API bool
ten_msg_conversion_operation_per_property_rule_fixed_value_to_json(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self,
    ten_json_t *json, ten_error_t *er);

TEN_RUNTIME_PRIVATE_API bool
ten_msg_conversion_operation_per_property_rule_fixed_value_from_value(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self,
    ten_value_t *value, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API void
ten_msg_conversion_operation_per_property_rule_fixed_value_to_value(
    ten_msg_conversion_operation_per_property_rule_fixed_value_t *self,
    ten_value_t *value);
