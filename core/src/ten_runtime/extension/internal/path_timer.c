//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/extension/path_timer.h"

#include "include_internal/ten_runtime/extension/close.h"
#include "include_internal/ten_runtime/extension/extension.h"
#include "include_internal/ten_runtime/extension/msg_handling.h"
#include "include_internal/ten_runtime/extension_thread/extension_thread.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd_base.h"
#include "include_internal/ten_runtime/path/path.h"
#include "include_internal/ten_runtime/path/path_table.h"
#include "include_internal/ten_runtime/timer/timer.h"
#include "ten_runtime/msg/cmd_result/cmd_result.h"
#include "ten_runtime/timer/timer.h"
#include "ten_utils/container/list.h"
#include "ten_utils/lib/string.h"
#include "ten_utils/lib/time.h"

static void ten_extension_in_path_timer_on_triggered(ten_timer_t *self,
                                                     void *on_trigger_data) {
  ten_extension_t *extension = (ten_extension_t *)on_trigger_data;
  TEN_ASSERT(extension && ten_extension_check_integrity(extension, true) &&
                 self && ten_timer_check_integrity(self, true),
             "Should not happen.");

  ten_path_table_t *path_table = extension->path_table;
  TEN_ASSERT(path_table, "Should not happen.");

  ten_list_t *in_paths = &path_table->in_paths;
  TEN_ASSERT(in_paths && ten_list_check_integrity(in_paths),
             "Should not happen.");

  int64_t current_time_us = ten_current_time_us();

  // Remove all the expired paths in the IN path table.
  ten_list_foreach (in_paths, iter) {
    ten_path_t *path = (ten_path_t *)ten_ptr_listnode_get(iter.node);
    TEN_ASSERT(path && ten_path_check_integrity(path, true),
               "Should not happen.");

    if (current_time_us >= path->expired_time_us) {
      ten_list_remove_node(in_paths, iter.node);
    }
  }
}

static void ten_extension_out_path_timer_on_triggered(ten_timer_t *self,
                                                      void *on_trigger_data) {
  ten_extension_t *extension = (ten_extension_t *)on_trigger_data;
  TEN_ASSERT(extension && ten_extension_check_integrity(extension, true) &&
                 self && ten_timer_check_integrity(self, true),
             "Should not happen.");

  ten_path_table_t *path_table = extension->path_table;
  TEN_ASSERT(path_table, "Should not happen.");

  ten_list_t *out_paths = &path_table->out_paths;
  TEN_ASSERT(out_paths && ten_list_check_integrity(out_paths),
             "Should not happen.");

  int64_t current_time_us = ten_current_time_us();

  // Remove all the expired paths in the OUT path table.
  size_t timeout_paths_cnt = 0;
  ten_list_foreach (out_paths, iter) {
    ten_path_t *path = (ten_path_t *)ten_ptr_listnode_get(iter.node);
    TEN_ASSERT(path && ten_path_check_integrity(path, true),
               "Should not happen.");

    if (current_time_us >= path->expired_time_us) {
      ++timeout_paths_cnt;
      ten_extension_terminate_out_path_prematurely(extension, path,
                                                   "Path timeout.");
    }
  }

  if (timeout_paths_cnt) {
    TEN_LOGE("[%s] %zu paths timeout.", ten_extension_get_name(extension),
             timeout_paths_cnt);
  }
}

ten_timer_t *ten_extension_create_timer_for_in_path(ten_extension_t *self) {
  TEN_ASSERT(self && ten_extension_check_integrity(self, true),
             "Should not happen.");

  ten_extension_thread_t *extension_thread = self->extension_thread;
  TEN_ASSERT(extension_thread &&
                 ten_extension_thread_check_integrity(extension_thread, true),
             "Should not happen.");

  ten_timer_t *timer = ten_timer_create(
      ten_extension_thread_get_attached_runloop(extension_thread),
      self->path_timeout_info.check_interval, TEN_TIMER_INFINITE);

  ten_timer_set_on_triggered(timer, ten_extension_in_path_timer_on_triggered,
                             self);
  ten_timer_set_on_closed(timer, ten_extension_on_timer_closed, self);

  return timer;
}

ten_timer_t *ten_extension_create_timer_for_out_path(ten_extension_t *self) {
  TEN_ASSERT(self && ten_extension_check_integrity(self, true),
             "Should not happen.");

  ten_extension_thread_t *extension_thread = self->extension_thread;
  TEN_ASSERT(extension_thread &&
                 ten_extension_thread_check_integrity(extension_thread, true),
             "Should not happen.");

  ten_timer_t *timer = ten_timer_create(
      ten_extension_thread_get_attached_runloop(extension_thread),
      self->path_timeout_info.check_interval, TEN_TIMER_INFINITE);

  ten_timer_set_on_triggered(timer, ten_extension_out_path_timer_on_triggered,
                             self);
  ten_timer_set_on_closed(timer, ten_extension_on_timer_closed, self);

  return timer;
}

void ten_extension_terminate_out_path_prematurely(ten_extension_t *self,
                                                  ten_path_t *path,
                                                  const char *msg) {
  TEN_ASSERT(self && ten_extension_check_integrity(self, true) && path &&
                 ten_path_check_integrity(path, true),
             "Should not happen.");

  ten_shared_ptr_t *cmd_result = ten_cmd_result_create(TEN_STATUS_CODE_ERROR);
  ten_msg_set_property(cmd_result, "detail", ten_value_create_string(msg),
                       NULL);
  TEN_ASSERT(cmd_result && ten_cmd_base_check_integrity(cmd_result),
             "Should not happen.");

  ten_cmd_base_set_cmd_id(cmd_result, ten_string_get_raw_str(&path->cmd_id));

  ten_extension_handle_in_msg(self, cmd_result);
  ten_shared_ptr_destroy(cmd_result);
}
