//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "ten_utils/lib/error.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/lib/smart_ptr.h"

typedef struct ten_env_t ten_env_t;
typedef struct ten_extension_t ten_extension_t;

typedef void (*ten_env_cmd_result_handler_func_t)(
    ten_extension_t *extension, ten_env_t *ten_env,
    ten_shared_ptr_t *cmd_result, void *cmd_result_handler_data);

TEN_RUNTIME_API bool ten_env_send_cmd(
    ten_env_t *self, ten_shared_ptr_t *cmd,
    ten_env_cmd_result_handler_func_t result_handler, void *result_handler_data,
    ten_error_t *err);

TEN_RUNTIME_API bool ten_env_send_json(
    ten_env_t *self, ten_json_t *json,
    ten_env_cmd_result_handler_func_t result_handler, void *result_handler_data,
    ten_error_t *err);

TEN_RUNTIME_API bool ten_env_send_data(ten_env_t *self, ten_shared_ptr_t *data,
                                       ten_error_t *err);

TEN_RUNTIME_API bool ten_env_send_video_frame(ten_env_t *self,
                                              ten_shared_ptr_t *frame,
                                              ten_error_t *err);

TEN_RUNTIME_API bool ten_env_send_audio_frame(ten_env_t *self,
                                              ten_shared_ptr_t *frame,
                                              ten_error_t *err);
