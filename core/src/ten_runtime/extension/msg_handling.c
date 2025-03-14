//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/extension/msg_handling.h"

#include <stdbool.h>

#include "include_internal/ten_runtime/extension/extension.h"
#include "include_internal/ten_runtime/extension/extension_info/extension_info.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd_base.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd_result/cmd.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/msg/msg_info.h"
#include "include_internal/ten_runtime/msg_conversion/msg_conversion/msg_and_result_conversion.h"
#include "include_internal/ten_runtime/msg_conversion/msg_conversion/msg_conversion.h"
#include "include_internal/ten_runtime/path/common.h"
#include "include_internal/ten_runtime/path/path.h"
#include "include_internal/ten_runtime/path/path_group.h"
#include "include_internal/ten_runtime/path/path_table.h"
#include "ten_runtime/extension/extension.h"
#include "ten_runtime/msg/cmd_result/cmd_result.h"
#include "ten_runtime/ten_env/internal/return.h"
#include "ten_utils/container/list.h"
#include "ten_utils/container/list_ptr.h"
#include "ten_utils/lib/error.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/log/log.h"
#include "ten_utils/macro/check.h"

static void ten_extension_cache_cmd_result_to_in_path_for_auto_return(
    ten_extension_t *extension, ten_shared_ptr_t *cmd) {
  TEN_ASSERT(extension && ten_extension_check_integrity(extension, true),
             "Invalid argument.");
  TEN_ASSERT(cmd && ten_msg_get_type(cmd) == TEN_MSG_TYPE_CMD_RESULT &&
                 ten_cmd_base_check_integrity(cmd),
             "Should not happen.");

  TEN_LOGV(
      "[%s] Receives a cmd result (%p) for '%s', and will cache it to do some "
      "post processing later.",
      ten_extension_get_name(extension), ten_shared_ptr_get_data(cmd),
      ten_msg_type_to_string(ten_cmd_result_get_original_cmd_type(cmd)));

  ten_msg_dump(cmd, NULL, "The received cmd result: ^m");

  ten_path_t *in_path =
      (ten_path_t *)ten_extension_get_cmd_return_path_from_itself(
          extension, ten_cmd_base_get_cmd_id(cmd));
  if (!in_path) {
    // The cmd result may has returned before.
    TEN_LOGV("[%s] No return path found for cmd result (%p) for '%s'.",
             ten_string_get_raw_str(&extension->name),
             ten_shared_ptr_get_data(cmd),
             ten_msg_type_to_string(ten_cmd_result_get_original_cmd_type(cmd)));
    return;
  }

  ten_path_set_result(in_path, cmd);
}

void ten_extension_handle_in_msg(ten_extension_t *self, ten_shared_ptr_t *msg) {
  TEN_ASSERT(self && ten_extension_check_integrity(self, true),
             "Invalid argument.");
  TEN_ASSERT(msg && ten_msg_check_integrity(msg), "Invalid argument.");

  ten_error_t err;
  ten_error_init(&err);

  bool delete_msg = false;

  // - During on_init() and on_deinit(), there should be no interaction with
  //   other extensions.
  // - During on_start() and on_stop(), it is possible to send messages to other
  //   extensions and receive cmd_result for that message.
  //
  // The messages, from other extensions, sent to this extension will be
  // delivered to this extension only after its on_start_done(). Therefore, all
  // messages sent to this extension before on_start_done() will be queued until
  // on_start_done() is triggered. On the other hand, the cmd result of the
  // command sent by this extension in on_start() can be delivered to this
  // extension before its on_start_done().

  if (!(self->state == TEN_EXTENSION_STATE_STARTED) &&
      !(ten_msg_is_cmd_result(msg) &&
        ten_msg_get_type(msg) == TEN_MSG_TYPE_CMD_RESULT)) {
    // The extension is not started, and the msg is not a cmd result, so
    // cache the msg to the pending list.
    ten_list_push_smart_ptr_back(&self->pending_msgs, msg);
    goto done;
  }

  bool msg_is_cmd_result = false;

  // Because 'commands' has 'results', TEN will perform some bookkeeping for
  // 'commands' before sending it to the extension.

  if (ten_msg_get_type(msg) == TEN_MSG_TYPE_CMD_RESULT) {
    msg_is_cmd_result = true;

    // Set the cmd result to the corresponding OUT path to indicate that
    // there has been a cmd result flow through that OUT path.
    ten_path_t *out_path =
        ten_path_table_set_result(self->path_table, TEN_PATH_OUT, msg);
    if (!out_path) {
      // The OUT path is gone, it means the current cmd result should be
      // discarded (not sending it to the extension).
      msg = NULL;
    } else {
      TEN_ASSERT(ten_path_check_integrity(out_path, true), "Invalid argument.");

      // The path will be removed from the path table if the cmd result is
      // determined. It's fine here, as we are in the target extension (the
      // consumer of the cmd result), and the producer extension (the
      // extension calls return_xxx()) can not get any information (ex: the
      // result of conversion or schema validation) from this extension. The
      // producer extension has no opportunity to retry even if something
      // fails, so the path can be removed.
      msg = ten_path_table_determine_actual_cmd_result(
          self->path_table, TEN_PATH_OUT, out_path,
          ten_cmd_result_get_is_final(msg, NULL));
      if (msg) {
        // The cmd result should be sent to the extension.

        // Because the cmd result might not be handled in the extension, in
        // other words, there might not be 'return_xxx' in the extension for
        // this cmd result, so we cache the cmd result to the
        // corresponding IN path (if exists), and will handle this status
        // command (if the IN path still exists) when on_cmd_done().
        //
        // TODO(Xilin): Currently, there is no mechanism for auto return, so the
        // relevant logic code should be able to be disabled.
        ten_extension_cache_cmd_result_to_in_path_for_auto_return(self, msg);

        delete_msg = true;
      }
    }
  }

  if (!msg) {
    goto done;
  }

  bool should_this_msg_do_msg_conversion =
      // The builtin cmds (e.g., 'status', 'timeout') could not be converted.
      (ten_msg_get_type(msg) == TEN_MSG_TYPE_CMD ||
       ten_msg_get_type(msg) == TEN_MSG_TYPE_DATA ||
       ten_msg_get_type(msg) == TEN_MSG_TYPE_VIDEO_FRAME ||
       ten_msg_get_type(msg) == TEN_MSG_TYPE_AUDIO_FRAME) &&
      // If there is no message conversions exist, put the original command to
      // the list directly.
      self->extension_info &&
      ten_list_size(&self->extension_info->msg_conversions) > 0;

  // The type of elements is 'ten_msg_and_result_conversion_t'.
  ten_list_t converted_msgs = TEN_LIST_INIT_VAL;

  if (should_this_msg_do_msg_conversion) {
    // Perform the command conversion, and get the actual commands.
    ten_error_t err;
    ten_error_init(&err);
    if (!ten_extension_convert_msg(self, msg, &converted_msgs, &err)) {
      TEN_LOGE("[%s] Failed to convert msg %s: %s",
               ten_extension_get_name(self), ten_msg_get_name(msg),
               ten_error_errmsg(&err));
    }
    ten_error_deinit(&err);
  } else {
    ten_list_push_ptr_back(
        &converted_msgs, ten_msg_and_result_conversion_create(msg, NULL),
        (ten_ptr_listnode_destroy_func_t)ten_msg_and_result_conversion_destroy);
  }

  // Get the actual messages which should be sent to the extension. Handle those
  // messages now.

  if (!msg_is_cmd_result) {
    // Create the corresponding IN paths for the input commands.

    ten_list_t in_paths = TEN_LIST_INIT_VAL;

    ten_list_foreach (&converted_msgs, iter) {
      ten_msg_and_result_conversion_t *msg_and_result_conversion =
          ten_ptr_listnode_get(iter.node);
      TEN_ASSERT(msg_and_result_conversion, "Invalid argument.");

      ten_shared_ptr_t *actual_cmd = msg_and_result_conversion->msg;
      TEN_ASSERT(actual_cmd && ten_msg_check_integrity(actual_cmd),
                 "Should not happen.");

      if (ten_msg_is_cmd(actual_cmd)) {
        if (ten_msg_info[ten_msg_get_type(actual_cmd)].create_in_path) {
          // Add a path entry to the IN path table of this extension.
          ten_path_t *in_path = (ten_path_t *)ten_path_table_add_in_path(
              self->path_table, actual_cmd,
              msg_and_result_conversion->operation);

          ten_list_push_ptr_back(&in_paths, in_path, NULL);
        }
      }
    }

    if (ten_list_size(&in_paths) > 1) {
      // Create a path group in this case.

      ten_paths_create_group(
          &in_paths,
          TEN_PATH_GROUP_POLICY_ONE_FAIL_RETURN_AND_ALL_OK_RETURN_LAST);
    }

    ten_list_clear(&in_paths);
  }

  // The path table processing is completed, it's time to check the schema. And
  // the schema validation must be happened after conversions, as the schemas of
  // the msgs are defined in the extension, and the msg structure is expected to
  // be matched with the schema. The correctness of the msg structure is
  // guaranteed by the conversions.
  bool pass_schema_check = true;
  ten_list_foreach (&converted_msgs, iter) {
    ten_msg_and_result_conversion_t *msg_and_result_conversion =
        ten_ptr_listnode_get(iter.node);
    TEN_ASSERT(msg_and_result_conversion, "Invalid argument.");

    ten_shared_ptr_t *actual_msg = msg_and_result_conversion->msg;
    TEN_ASSERT(actual_msg && ten_msg_check_integrity(actual_msg),
               "Should not happen.");

    if (!ten_extension_validate_msg_schema(self, actual_msg, false, &err)) {
      pass_schema_check = false;
      break;
    }
  }

  if (pass_schema_check) {
    // The schema checking is pass, it's time to start sending the commands to
    // the extension.

    ten_list_foreach (&converted_msgs, iter) {
      ten_msg_and_result_conversion_t *msg_and_result_conversion =
          ten_ptr_listnode_get(iter.node);
      TEN_ASSERT(msg_and_result_conversion, "Invalid argument.");

      ten_shared_ptr_t *actual_msg = msg_and_result_conversion->msg;
      TEN_ASSERT(actual_msg && ten_msg_check_integrity(actual_msg),
                 "Should not happen.");

      // Clear destination before sending to Extension, so that when Extension
      // sends msg back to the TEN core, we can check if the destination is not
      // empty to determine if TEN core needs to determine the destinations
      // according to the graph.
      ten_msg_clear_dest(actual_msg);

      if (ten_msg_get_type(actual_msg) == TEN_MSG_TYPE_CMD_RESULT) {
        ten_env_cmd_result_handler_func_t result_handler =
            ten_cmd_base_get_raw_cmd_base(actual_msg)->result_handler;
        if (result_handler) {
          result_handler(
              self, self->ten_env, actual_msg,
              ten_cmd_base_get_raw_cmd_base(actual_msg)->result_handler_data);
        } else {
          // If the cmd result does not have an associated result handler,
          // TEN runtime will return the cmd result to the upstream extension
          // (if existed) automatically. For example:
          //
          //              cmdA                 cmdA
          // ExtensionA --------> ExtensionB ---------> ExtensionC
          //    ^                   |    ^                |
          //    |                   |    |                |
          //    |                   v    |                v
          //     -------------------      ----------------
          //       cmdA's result         cmdA's result
          //
          // ExtensionB only needs to send the received cmdA to ExtensionC and
          // does not need to handle the result of cmdA. The TEN runtime will
          // help ExtensionB to return the result of cmdA to ExtensionA.
          ten_env_return_result_directly(self->ten_env, actual_msg, NULL);
        }
      } else {
        switch (ten_msg_get_type(msg)) {
          case TEN_MSG_TYPE_CMD:
          case TEN_MSG_TYPE_CMD_TIMEOUT:
            ten_extension_on_cmd(self, actual_msg);
            break;
          case TEN_MSG_TYPE_DATA:
            ten_extension_on_data(self, actual_msg);
            break;
          case TEN_MSG_TYPE_AUDIO_FRAME:
            ten_extension_on_audio_frame(self, actual_msg);
            break;
          case TEN_MSG_TYPE_VIDEO_FRAME:
            ten_extension_on_video_frame(self, actual_msg);
            break;
          default:
            TEN_ASSERT(0 && "Should handle more types.", "Should not happen.");
            break;
        }
      }
    }
  }

  ten_list_clear(&converted_msgs);

done:
  if (delete_msg && msg) {
    ten_shared_ptr_destroy(msg);
  }
  ten_error_deinit(&err);
}
