//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <stdbool.h>

#include "include_internal/ten_runtime/engine/internal/close.h"
#include "include_internal/ten_runtime/path/path_table.h"
#include "ten_utils/container/hash_table.h"
#include "ten_utils/container/list.h"
#include "ten_utils/io/runloop.h"
#include "ten_utils/lib/atomic.h"
#include "ten_utils/lib/event.h"
#include "ten_utils/lib/mutex.h"
#include "ten_utils/lib/signature.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/lib/string.h"

#define TEN_ENGINE_SIGNATURE 0x68E43695C0DB905AU

#define CMD_ID_COUNTER_MIN_VALUE 0
#define CMD_ID_COUNTER_MAX_VALUE 4095

typedef struct ten_extension_context_t ten_extension_context_t;
typedef struct ten_app_t ten_app_t;

struct ten_engine_t {
  ten_signature_t signature;
  ten_sanitizer_thread_check_t thread_check;

  ten_atomic_t is_closing;

  ten_engine_on_closed_func_t on_closed;
  void *on_closed_data;

  ten_app_t *app;
  ten_extension_context_t *extension_context;

  // This means that the engine can start to handle messages, i.e. all the
  // extension threads are started successfully.
  bool is_ready_to_handle_msg;

  // When app creates an engine, it will create a randomized graph ID for the
  // engine.
  ten_string_t graph_name;

  ten_path_table_t *path_table;

  // Save the original received 'start_graph' command so that after we
  // successfully started the engine, we can return a correct cmd result back
  // according to this saved 'start_graph' command.
  ten_shared_ptr_t *original_start_graph_cmd_of_enabling_engine;

  ten_list_t timers;

  // @{
  ten_hashtable_t remotes;  // ten_remote_t
  ten_list_t weak_remotes;
  // @}

  // @{
  ten_mutex_t *extension_msgs_lock;
  ten_list_t extension_msgs;
  // @}

  // @{
  // Used to send messages to the engine.
  ten_mutex_t *in_msgs_lock;
  ten_list_t in_msgs;
  // @}

  // @{
  // The following members are used for engines which have its own event loop.
  bool has_own_loop;
  ten_runloop_t *loop;
  ten_event_t *belonging_thread_is_set;
  ten_event_t *engine_thread_ready_for_migration;
  // @}

  bool long_running_mode;

  // Store the stop_graph command that will shut down this engine temporarily,
  // so that after the engine has completely closed, the cmd_result can be
  // returned based on this.
  ten_shared_ptr_t *cmd_stop_graph;
};

TEN_RUNTIME_PRIVATE_API bool ten_engine_check_integrity(ten_engine_t *self,
                                                        bool check_thread);

TEN_RUNTIME_PRIVATE_API ten_engine_t *ten_engine_create(ten_app_t *app,
                                                        ten_shared_ptr_t *cmd);

TEN_RUNTIME_PRIVATE_API void ten_engine_destroy(ten_engine_t *self);

TEN_RUNTIME_PRIVATE_API ten_runloop_t *ten_engine_get_attached_runloop(
    ten_engine_t *self);

TEN_RUNTIME_PRIVATE_API bool ten_engine_is_ready_to_handle_msg(
    ten_engine_t *self);

TEN_RUNTIME_PRIVATE_API const char *ten_engine_get_name(ten_engine_t *self);
