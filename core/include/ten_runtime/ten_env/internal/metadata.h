//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "ten_runtime/ten_env/ten_env.h"
#include "ten_utils/value/value.h"

typedef struct ten_env_t ten_env_t;
typedef struct ten_error_t ten_error_t;

typedef void (*ten_env_peek_property_async_cb_t)(ten_env_t *ten_env,
                                                 ten_value_t *value,
                                                 void *cb_data,
                                                 ten_error_t *err);

typedef void (*ten_env_set_property_async_cb_t)(ten_env_t *ten_env, bool res,
                                                void *cb_data,
                                                ten_error_t *err);

/**
 * @brief Note that the ownership of @a value would be transferred into the
 * TEN runtime, so the caller of this function could _not_ consider the
 * value instance is still valid.
 */
TEN_RUNTIME_API bool ten_env_set_property(ten_env_t *self, const char *path,
                                          ten_value_t *value, ten_error_t *err);

// This function is used to set prop on any threads.
TEN_RUNTIME_API bool ten_env_set_property_async(
    ten_env_t *self, const char *path, ten_value_t *value,
    ten_env_set_property_async_cb_t cb, void *cb_data, ten_error_t *err);

TEN_RUNTIME_API ten_value_t *ten_env_peek_property(ten_env_t *self,
                                                   const char *path,
                                                   ten_error_t *err);

TEN_RUNTIME_API bool ten_env_peek_property_async(
    ten_env_t *self, const char *path, ten_env_peek_property_async_cb_t cb,
    void *cb_data, ten_error_t *err);

TEN_RUNTIME_API bool ten_env_is_property_exist(ten_env_t *self,
                                               const char *path,
                                               ten_error_t *err);

TEN_RUNTIME_API bool ten_env_init_property_from_json(ten_env_t *self,
                                                     const char *json_str,
                                                     ten_error_t *err);
