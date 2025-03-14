//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stddef.h>

#include "src/ten_runtime/binding/go/interface/ten/common.h"
#include "ten_utils/lib/error.h"

#define TEN_GO_STATUS_ERR_MSG_BUF_SIZE 256

TEN_RUNTIME_PRIVATE_API ten_go_handle_array_t *ten_go_handle_array_create(
    size_t size);

TEN_RUNTIME_PRIVATE_API void ten_go_handle_array_destroy(
    ten_go_handle_array_t *self);

TEN_RUNTIME_PRIVATE_API char *ten_go_str_dup(const char *str);

TEN_RUNTIME_PRIVATE_API void ten_go_bridge_destroy_c_part(ten_go_bridge_t *self);

TEN_RUNTIME_PRIVATE_API void ten_go_bridge_destroy_go_part(ten_go_bridge_t *self);

TEN_RUNTIME_PRIVATE_API void ten_go_status_init_with_errno(ten_go_status_t *self,
                                                         ten_errno_t errno);

TEN_RUNTIME_PRIVATE_API void ten_go_status_from_error(ten_go_status_t *self,
                                                    ten_error_t *err);

TEN_RUNTIME_PRIVATE_API void ten_go_status_set_errno(ten_go_status_t *self,
                                                   ten_errno_t errno);

TEN_RUNTIME_PRIVATE_API void ten_go_status_set(ten_go_status_t *self,
                                             ten_errno_t errno,
                                             const char *msg);
