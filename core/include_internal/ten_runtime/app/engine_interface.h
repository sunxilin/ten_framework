//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include "ten_utils/lib/smart_ptr.h"

typedef struct ten_engine_t ten_engine_t;
typedef struct ten_app_t ten_app_t;
typedef struct ten_predefined_graph_info_t ten_predefined_graph_info_t;

TEN_RUNTIME_PRIVATE_API ten_engine_t *ten_app_create_engine(
    ten_app_t *self, ten_shared_ptr_t *cmd);

TEN_RUNTIME_PRIVATE_API void ten_app_del_engine(ten_app_t *self,
                                              ten_engine_t *engine);

TEN_RUNTIME_PRIVATE_API ten_predefined_graph_info_t *
ten_app_get_predefined_graph_info_based_on_dest_graph_name_from_msg(
    ten_app_t *self, ten_shared_ptr_t *msg);

TEN_RUNTIME_PRIVATE_API ten_engine_t *
ten_app_get_engine_based_on_dest_graph_name_from_msg(ten_app_t *self,
                                                     ten_shared_ptr_t *msg);
