//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "ten_runtime/ten_env/internal/send.h"

#include <stdbool.h>

#include "include_internal/ten_runtime/common/loc.h"
#include "include_internal/ten_runtime/extension/extension.h"
#include "include_internal/ten_runtime/extension_context/extension_context.h"
#include "include_internal/ten_runtime/extension_thread/extension_thread.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd_base.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/ten_env/ten_env.h"
#include "ten_runtime/common/errno.h"
#include "ten_runtime/msg/msg.h"
#include "ten_runtime/ten_env/ten_env.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/log/log.h"
#include "ten_utils/macro/check.h"

/**
 * @brief All message sending code flows will eventually fall into this
 * function.
 */
static bool ten_send_msg_internal(
    ten_env_t *self, ten_shared_ptr_t *msg,
    ten_env_cmd_result_handler_func_t result_handler, void *result_handler_data,
    ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(msg && ten_msg_check_integrity(msg), "Should not happen.");

  bool err_new_created = false;
  if (!err) {
    err = ten_error_create();
    err_new_created = true;
  }

  bool result = true;

  if (!msg) {
    ten_error_set(err, TEN_ERRNO_INVALID_ARGUMENT, "Msg is empty.");
    result = false;
    goto done;
  }

  if (ten_msg_get_type(msg) == TEN_MSG_TYPE_CMD_RESULT) {
    // The only way to send out the 'status' command from extension is through
    // various 'return_xxx' functions.
    TEN_LOGE("Result commands should be delivered through 'returning'.");
    ten_error_set(err, TEN_ERRNO_GENERIC,
                  "Result commands should be delivered through 'returning'.");
    result = false;
    goto done;
  }

  if (ten_msg_has_locked_res(msg)) {
    ten_error_set(err, TEN_ERRNO_GENERIC,
                  "Locked resources are not allowed in messages sent from an "
                  "extension.");
    result = false;
    goto done;
  }

  const bool msg_is_cmd = ten_msg_is_cmd(msg);

  if (msg_is_cmd) {
    // 'command' has the mechanism of 'result'.
    ten_cmd_base_set_result_handler(msg, result_handler, result_handler_data);

    // @{
    // All commands sent from an extension will eventually go to this function,
    // and command ID plays a critical role in the TEN runtime, therefore, when
    // a command is sent from an extension, and doesn't contain a command ID,
    // TEN runtime must assign it a command ID _before_ sending it to the TEN
    // runtime deeper.
    const char *cmd_id = ten_cmd_base_get_cmd_id(msg);
    if (!strlen(cmd_id)) {
      cmd_id = ten_cmd_base_gen_new_cmd_id_forcibly(msg);
    }
    // @}
  }

  ten_extension_t *extension = ten_env_get_attached_extension(self);
  TEN_ASSERT(extension, "Invalid argument.");

  if (extension->state < TEN_EXTENSION_STATE_INITED) {
    TEN_LOGE("Cannot send messages before on_init_done.");
    ten_error_set(err, TEN_ERRNO_GENERIC,
                  "Cannot send messages before on_init_done.");
    result = false;
    goto done;
  }

  if (extension->state >= TEN_EXTENSION_STATE_CLOSING) {
    TEN_LOGE("Cannot send messages after on_stop_done.");
    ten_error_set(err, TEN_ERRNO_GENERIC,
                  "Cannot send messages after on_stop_done.");
    result = false;
    goto done;
  }

  result = ten_extension_handle_out_msg(extension, msg, err);

done:
  if (err_new_created) {
    ten_error_destroy(err);
  }

  return result;
}

bool ten_env_send_cmd(ten_env_t *self, ten_shared_ptr_t *cmd,
                      ten_env_cmd_result_handler_func_t result_handler,
                      void *result_handler_data, ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(cmd, "Should not happen.");

  return ten_send_msg_internal(self, cmd, result_handler, result_handler_data,
                               err);
}

static bool ten_env_send_json_internal(
    ten_env_t *self, ten_json_t *json,
    ten_env_cmd_result_handler_func_t result_handler, void *result_handler_data,
    ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(json, "Should not happen.");

  bool success = false;

  ten_shared_ptr_t *cmd = ten_msg_create_from_json(json, err);
  if (cmd) {
    success = ten_send_msg_internal(self, cmd, result_handler,
                                    result_handler_data, err);
    ten_shared_ptr_destroy(cmd);
  }

  return success;
}

bool ten_env_send_json(ten_env_t *self, ten_json_t *json,
                       ten_env_cmd_result_handler_func_t result_handler,
                       void *result_handler_data, ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(json, "Should not happen.");

  return ten_env_send_json_internal(self, json, result_handler,
                                    result_handler_data, err);
}

bool ten_env_send_data(ten_env_t *self, ten_shared_ptr_t *data,
                       ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(data, "Should not happen.");

  return ten_send_msg_internal(self, data, NULL, NULL, err);
}

bool ten_env_send_video_frame(ten_env_t *self, ten_shared_ptr_t *frame,
                              ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(frame, "Should not happen.");

  return ten_send_msg_internal(self, frame, NULL, NULL, err);
}

bool ten_env_send_audio_frame(ten_env_t *self, ten_shared_ptr_t *frame,
                              ten_error_t *err) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_env_check_integrity(self, true), "Invalid use of ten_env %p.",
             self);
  TEN_ASSERT(frame, "Should not happen.");

  return ten_send_msg_internal(self, frame, NULL, NULL, err);
}
