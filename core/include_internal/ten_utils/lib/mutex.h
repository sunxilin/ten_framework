//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_utils/ten_config.h"

#include "ten_utils/lib/mutex.h"
#include "ten_utils/lib/thread.h"

#define TEN_MUTEX_SIGNATURE 0x0EFAAE3C19611249U

/**
 * @brief Set the owner thread.
 * @param mutex The mutex handle.
 */
TEN_UTILS_PRIVATE_API void ten_mutex_set_owner(ten_mutex_t *mutex,
                                               ten_tid_t owner);
