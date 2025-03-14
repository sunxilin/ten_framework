//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "ten_utils/lib/error.h"

typedef struct ten_env_t ten_env_t;
typedef struct ten_metadata_info_t ten_metadata_info_t;

TEN_RUNTIME_API bool ten_env_on_init_done(ten_env_t *self, ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_deinit_done(ten_env_t *self, ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_start_done(ten_env_t *self, ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_stop_done(ten_env_t *self, ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_create_extensions_done(ten_env_t *self,
                                                       ten_list_t *extensions,
                                                       ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_destroy_extensions_done(ten_env_t *self,
                                                        ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_create_instance_done(ten_env_t *self,
                                                     void *instance,
                                                     void *context,
                                                     ten_error_t *err);

TEN_RUNTIME_API bool ten_env_on_destroy_instance_done(ten_env_t *self,
                                                      void *context,
                                                      ten_error_t *err);
