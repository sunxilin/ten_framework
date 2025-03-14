//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "src/ten_runtime/binding/go/interface/ten/common.h"
#include "src/ten_runtime/binding/go/interface/ten/metadata.h"

#define TEN_GO_METADATA_SIGNATURE 0xC3D0D91971B40C4FU

TEN_RUNTIME_PRIVATE_API bool ten_go_metadata_check_integrity(
    ten_go_metadata_t *self);

TEN_RUNTIME_PRIVATE_API ten_go_handle_t
ten_go_metadata_go_handle(ten_go_metadata_t *self);

TEN_RUNTIME_PRIVATE_API ten_metadata_info_t *ten_go_metadata_c_metadata(
    ten_go_metadata_t *self);

TEN_RUNTIME_PRIVATE_API ten_go_metadata_t *ten_go_metadata_wrap(
    ten_metadata_info_t *c_metadata);
