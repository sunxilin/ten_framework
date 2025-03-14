//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/binding/go/internal/common.h"
#include "include_internal/ten_runtime/binding/go/ten_env/ten_env.h"
#include "include_internal/ten_runtime/binding/go/ten_env/ten_env_internal.h"
#include "ten_runtime/binding/go/interface/ten/common.h"
#include "ten_runtime/binding/go/interface/ten/ten.h"
#include "ten_runtime/common/errno.h"
#include "ten_runtime/ten_env/internal/metadata.h"
#include "ten_runtime/ten_env_proxy/ten_env_proxy.h"
#include "ten_utils/lib/alloc.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/lib/event.h"
#include "ten_utils/lib/string.h"
#include "ten_utils/macro/check.h"

typedef struct ten_notify_set_init_property_info_t {
  ten_string_t value;
  ten_event_t *completed;
} ten_env_notify_init_property_info_t;

static ten_env_notify_init_property_info_t *
ten_env_notify_init_property_info_create(const void *value, int value_len) {
  ten_env_notify_init_property_info_t *info =
      TEN_MALLOC(sizeof(ten_env_notify_init_property_info_t));
  TEN_ASSERT(info, "Failed to allocate memory.");

  ten_string_init_formatted(&info->value, "%.*s", value_len, value);
  info->completed = ten_event_create(0, 1);

  return info;
}

static void ten_env_notify_init_property_info_destroy(
    ten_env_notify_init_property_info_t *self) {
  TEN_ASSERT(self, "Invalid argument.");

  ten_string_deinit(&self->value);
  ten_event_destroy(self->completed);

  TEN_FREE(self);
}

static void ten_env_notify_init_property_from_json(ten_env_t *ten_env,
                                                   void *user_data) {
  TEN_ASSERT(user_data, "Invalid argument.");
  TEN_ASSERT(ten_env && ten_env_check_integrity(ten_env, true),
             "Should not happen.");

  ten_env_notify_init_property_info_t *info = user_data;
  TEN_ASSERT(info, "Should not happen.");

  ten_error_t err;
  ten_error_init(&err);

  ten_env_init_property_from_json(ten_env, ten_string_get_raw_str(&info->value),
                                  &err);

  ten_event_set(info->completed);

  ten_error_deinit(&err);
}

ten_go_status_t ten_go_ten_env_init_property_from_json_bytes(
    uintptr_t bridge_addr, const void *json_str, int json_str_len) {
  ten_go_ten_env_t *self = ten_go_ten_env_reinterpret(bridge_addr);
  TEN_ASSERT(self && ten_go_ten_env_check_integrity(self),
             "Should not happen.");

  ten_go_status_t status;
  ten_go_status_init_with_errno(&status, TEN_ERRNO_OK);

  TEN_GO_TEN_IS_ALIVE_REGION_BEGIN(self, {
    ten_go_status_init_with_errno(&status, TEN_ERRNO_TEN_IS_CLOSED);
    return status;
  });

  ten_error_t err;
  ten_error_init(&err);
  ten_env_notify_init_property_info_t *info =
      ten_env_notify_init_property_info_create(json_str, json_str_len);
  TEN_ASSERT(info, "Should not happen.");

  if (!ten_env_proxy_notify(self->c_ten_proxy,
                            ten_env_notify_init_property_from_json, info, false,
                            &err)) {
    ten_go_status_from_error(&status, &err);
    goto done;
  }

  ten_event_wait(info->completed, -1);

done:
  ten_env_notify_init_property_info_destroy(info);
  ten_error_deinit(&err);
  TEN_GO_TEN_IS_ALIVE_REGION_END(self);

ten_is_close:
  return status;
}
