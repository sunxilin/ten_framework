//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "include_internal/ten_runtime/addon/addon.h"

typedef struct ten_env_t ten_env_t;

typedef struct
    ten_extension_context_on_addon_create_extension_group_done_info_t {
  ten_extension_group_t *extension_group;
  ten_addon_context_t *addon_context;
} ten_extension_context_on_addon_create_extension_group_done_info_t;

ten_extension_context_on_addon_create_extension_group_done_info_t *
ten_extension_context_on_addon_create_extension_group_done_info_create(void);

void ten_extension_context_on_addon_create_extension_group_done_info_destroy(
    ten_extension_context_on_addon_create_extension_group_done_info_t *self);

TEN_RUNTIME_PRIVATE_API void
ten_extension_context_on_addon_create_extension_group_done(
    ten_env_t *self, void *instance, ten_addon_context_t *addon_context);

TEN_RUNTIME_PRIVATE_API void
ten_extension_context_on_addon_destroy_extension_group_done(
    ten_env_t *self, ten_addon_context_t *addon_context);
