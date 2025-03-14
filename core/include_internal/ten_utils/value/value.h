//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_utils/ten_config.h"

#include <stdbool.h>

#include "ten_utils/value/value.h"  // IWYU pragma: keep
#include "ten_utils/value/value_kv.h"

#define TEN_VALUE_SIGNATURE 0x1F30F97F37E6BC42U

TEN_UTILS_PRIVATE_API bool ten_value_is_equal(ten_value_t *self,
                                              ten_value_t *target);

TEN_UTILS_API ten_value_t *ten_value_create_vstring(const char *fmt, ...);

TEN_UTILS_API ten_value_t *ten_value_create_vastring(const char *fmt,
                                                     va_list ap);

TEN_UTILS_PRIVATE_API void ten_value_reset_to_null(ten_value_t *self);
