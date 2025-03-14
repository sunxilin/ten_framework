//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "include_internal/ten_runtime/msg/cmd_base/cmd_base.h"
#include "ten_runtime/common/status_code.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/lib/smart_ptr.h"

#define TEN_CMD_STATUS_SIGNATURE 0x9EAF798217CDEC8DU

typedef struct ten_msg_t ten_msg_t;
typedef struct ten_schema_store_t ten_schema_store_t;

typedef struct ten_cmd_result_t {
  ten_cmd_base_t cmd_base_hdr;

  ten_signature_t signature;

  TEN_MSG_TYPE original_cmd_type;
  ten_string_t original_cmd_name;

  TEN_STATUS_CODE status_code;

  bool is_final;
} ten_cmd_result_t;

TEN_RUNTIME_PRIVATE_API bool ten_raw_cmd_result_validate_schema(
    ten_msg_t *msg, ten_schema_store_t *schema_store, bool is_msg_out,
    ten_error_t *err);

/**
 * @brief The schema definition of the `status` cmd is defined within the schema
 * of the `cmd`, ex:
 *
 *   "api": {
 *     "cmd_in": [{
 *       "name": "hello",
 *       "result": {
 *         "property": {}
 *       }
 *     }]
 *   }
 *
 * In other words, the schema of the `status` cmd is indexed by the original cmd
 * name. When we want to validate the cmd result, we must get the original cmd
 * name from the path first, and the original cmd name has to be stored in the
 * cmd result for using later. As the msg name of the cmd result is always a
 * fixed value (i.e., ten::result), we can borrow the storage of the msg name to
 * store the original cmd name, and then resume the msg name after the schema
 * validation.
 */
TEN_RUNTIME_PRIVATE_API void ten_cmd_result_set_original_cmd_name(
    ten_shared_ptr_t *self, const char *original_cmd_name);

TEN_RUNTIME_PRIVATE_API void ten_raw_cmd_result_set_status_code(
    ten_cmd_result_t *self, TEN_STATUS_CODE status_code);

TEN_RUNTIME_API void ten_cmd_result_set_status_code(
    ten_shared_ptr_t *self, TEN_STATUS_CODE status_code);

TEN_RUNTIME_PRIVATE_API void ten_raw_cmd_result_set_original_cmd_type(
    ten_cmd_result_t *self, TEN_MSG_TYPE type);

TEN_RUNTIME_PRIVATE_API TEN_MSG_TYPE
ten_raw_cmd_result_get_original_cmd_type(ten_cmd_result_t *self);

TEN_RUNTIME_PRIVATE_API void ten_cmd_result_set_original_cmd_type(
    ten_shared_ptr_t *self, TEN_MSG_TYPE type);

TEN_RUNTIME_PRIVATE_API TEN_MSG_TYPE
ten_cmd_result_get_original_cmd_type(ten_shared_ptr_t *self);

TEN_RUNTIME_PRIVATE_API bool ten_raw_cmd_result_as_msg_init_from_json(
    ten_msg_t *self, ten_json_t *json, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API ten_msg_t *ten_raw_cmd_result_as_msg_create_from_json(
    ten_json_t *json, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API ten_msg_t *ten_raw_cmd_result_as_msg_clone(
    ten_msg_t *self, TEN_UNUSED ten_list_t *excluded_field_ids);

TEN_RUNTIME_PRIVATE_API ten_json_t *ten_raw_cmd_result_as_msg_to_json(
    ten_msg_t *self, ten_error_t *err);

TEN_RUNTIME_PRIVATE_API void ten_raw_cmd_result_destroy(ten_cmd_result_t *self);

TEN_RUNTIME_PRIVATE_API TEN_STATUS_CODE
ten_raw_cmd_result_get_status_code(ten_cmd_result_t *self);

TEN_RUNTIME_API ten_json_t *ten_cmd_result_to_json(ten_shared_ptr_t *self,
                                                   ten_error_t *err);
