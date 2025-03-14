//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "ten_runtime/ten_env/ten_env.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/lib/smart_ptr.h"

TEN_RUNTIME_API bool ten_env_return_result(ten_env_t *self,
                                         ten_shared_ptr_t *result,
                                         ten_shared_ptr_t *target_cmd,
                                         ten_error_t *err);

TEN_RUNTIME_API bool ten_env_return_result_directly(ten_env_t *self,
                                                  ten_shared_ptr_t *cmd,
                                                  ten_error_t *err);
