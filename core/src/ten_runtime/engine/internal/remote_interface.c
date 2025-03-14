//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/engine/internal/remote_interface.h"

#include <stdlib.h>

#include "include_internal/ten_runtime/app/app.h"
#include "include_internal/ten_runtime/common/loc.h"
#include "include_internal/ten_runtime/connection/connection.h"
#include "include_internal/ten_runtime/connection/migration.h"
#include "include_internal/ten_runtime/engine/engine.h"
#include "include_internal/ten_runtime/engine/internal/thread.h"
#include "include_internal/ten_runtime/engine/msg_interface/common.h"
#include "include_internal/ten_runtime/engine/msg_interface/start_graph.h"
#include "include_internal/ten_runtime/extension_context/extension_context.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/protocol/protocol.h"
#include "include_internal/ten_runtime/remote/remote.h"
#include "ten_runtime/app/app.h"
#include "ten_runtime/common/status_code.h"
#include "ten_runtime/msg/cmd_result/cmd_result.h"
#include "ten_utils/container/list_node.h"
#include "ten_utils/container/list_node_ptr.h"
#include "ten_utils/container/list_ptr.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/lib/string.h"
#include "ten_utils/log/log.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/macro/field.h"

static bool ten_engine_del_weak_remote(ten_engine_t *self,
                                       ten_remote_t *remote) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  bool success = ten_list_remove_ptr(&self->weak_remotes, remote);

  TEN_LOGV("Delete remote %p from weak list: %s", remote,
           success ? "success." : "failed.");

  return success;
}

static size_t ten_engine_weak_remotes_cnt_in_specified_uri(ten_engine_t *self,
                                                           const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  size_t cnt = ten_list_find_ptr_cnt_custom(&self->weak_remotes, uri,
                                            ten_remote_is_uri_equal_to);

  TEN_LOGV("weak remote cnt for %s: %zu", uri, cnt);

  return cnt;
}

void ten_engine_on_remote_closed(ten_remote_t *remote, void *on_closed_data) {
  TEN_ASSERT(remote && on_closed_data, "Should not happen.");

  ten_engine_t *self = (ten_engine_t *)on_closed_data;
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(ten_engine_weak_remotes_cnt_in_specified_uri(
                 self, ten_string_get_raw_str(&remote->uri)) <= 1,
             "There should be at most 1 weak remote of the specified uri.");

  bool is_weak = ten_engine_del_weak_remote(self, remote);
  if (is_weak) {
    // The closing of a weak remote is a normal case which should not trigger
    // the closing of the engine. Therefore, we just destroy the remote.
    ten_remote_destroy(remote);
  } else {
    ten_hashhandle_t *hh = ten_hashtable_find_string(
        &self->remotes, ten_string_get_raw_str(&remote->uri));
    TEN_ASSERT(hh, "Should not happen.");

    ten_hashtable_del(&self->remotes, hh);
  }

  if (ten_engine_is_closing(self)) {
    // Proceed to close the engine.
    ten_engine_on_close(self);
  } else {
    if (!is_weak && !self->long_running_mode) {
      // The closing of any remote would trigger the closing of the engine. If
      // we don't want this behavior, comment out the following line.
      ten_engine_close_async(self);
    }
  }
}

void ten_engine_add_remote(ten_engine_t *self, ten_remote_t *remote) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  TEN_LOGD("[%s] Add %s (%p) as remote.",
           ten_string_get_raw_str(ten_app_get_uri(self->app)),
           ten_string_get_raw_str(&remote->uri), remote);

  ten_hashtable_add_string(&self->remotes, &remote->hh_in_remote_table,
                           ten_string_get_raw_str(&remote->uri),
                           ten_remote_destroy);
}

void ten_engine_add_weak_remote(ten_engine_t *self, ten_remote_t *remote) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  TEN_LOGD("[%s] Add %s (%p) as weak remote.",
           ten_string_get_raw_str(ten_app_get_uri(self->app)),
           ten_string_get_raw_str(&remote->uri), remote);

  TEN_UNUSED ten_listnode_t *found = ten_list_find_ptr_custom(
      &self->weak_remotes, ten_string_get_raw_str(&remote->uri),
      ten_remote_is_uri_equal_to);
  TEN_ASSERT(!found, "There should be at most 1 weak remote of %s.",
             ten_string_get_raw_str(&remote->uri));

  // Do not set 'ten_remote_destroy' as the destroy function, because we might
  // _move_ a weak remote out of 'weak_remotes' list when we ensure it is not
  // duplicated.
  ten_list_push_ptr_back(&self->weak_remotes, remote, NULL);
}

void ten_engine_upgrade_weak_remote_to_normal_remote(ten_engine_t *self,
                                                     ten_remote_t *remote) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  ten_engine_del_weak_remote(self, remote);
  ten_engine_add_remote(self, remote);
}

// According to the uri of the remote, create the necessary 'protocol' and
// 'connection' for the remote.
static ten_connection_t *ten_engine_create_connection_for_remote(
    ten_engine_t *self, const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);
  TEN_ASSERT(uri, "Should not happen.");

  ten_protocol_t *protocol =
      ten_protocol_create(uri, TEN_PROTOCOL_ROLE_OUT_DEFAULT);
  TEN_ASSERT(protocol, "Should not happen.");

  ten_connection_t *connection = ten_connection_create(protocol);
  TEN_ASSERT(connection, "Should not happen.");

  // This is in the 'connect_to' stage, the 'connection' already attaches to the
  // engine, no migration is needed.
  ten_connection_set_migration_state(connection,
                                     TEN_CONNECTION_MIGRATION_STATE_DONE);

  return connection;
}

void ten_engine_link_connection_to_remote(ten_engine_t *self,
                                          ten_connection_t *connection,
                                          const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(connection, "Invalid argument.");
  TEN_ASSERT(ten_connection_check_integrity(connection, true),
             "Invalid use of engine %p.", connection);

  TEN_ASSERT(uri, "Invalid argument.");

  ten_remote_t *remote = ten_engine_find_remote(self, uri);
  if (remote) {
    TEN_ASSERT(
        remote->connection == NULL,
        "The relationship of remote and connection should be 1-1 mapping.");

    ten_connection_attach_to_remote(connection, remote);
  } else {
    remote = ten_remote_create_for_engine(uri, self, connection);

    ten_engine_add_remote(self, remote);
  }
}

static ten_remote_t *ten_engine_create_remote(ten_engine_t *self,
                                              const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(uri, "Should not happen.");

  ten_connection_t *connection =
      ten_engine_create_connection_for_remote(self, uri);

  ten_remote_t *remote = ten_remote_create_for_engine(uri, self, connection);
  TEN_ASSERT(remote, "Should not happen.");

  return remote;
}

/**
 * @brief The remote is connected successfully, it's time to send out the
 * message which was going to be sent originally.
 */
static void ten_engine_on_graph_remote_connected(ten_remote_t *self,
                                                 ten_shared_ptr_t *cmd) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(self, true),
             "Invalid use of remote %p.", self);

  TEN_ASSERT(self->connection && ten_connection_attach_to(self->connection) ==
                                     TEN_CONNECTION_ATTACH_TO_REMOTE,
             "Should not happen.");

  TEN_ASSERT(self->connection->protocol &&
                 ten_protocol_attach_to(self->connection->protocol) ==
                     TEN_PROTOCOL_ATTACH_TO_CONNECTION,
             "Should not happen.");

  TEN_ASSERT(cmd && ten_msg_check_integrity(cmd), "Invalid argument.");

  ten_protocol_send_msg(self->connection->protocol, cmd);

  ten_shared_ptr_destroy(cmd);
  self->on_server_connected_cmd = NULL;
}

static void ten_engine_on_graph_remote_connect_error(ten_remote_t *self,
                                                     ten_shared_ptr_t *cmd) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(self, true),
             "Invalid use of remote %p.", self);

  TEN_ASSERT(cmd && ten_msg_check_integrity(cmd), "Invalid argument.");

  // Failed to connect to remote, we must to delete (dereference) the message
  // which was going to be sent originally to prevent from memory leakage.
  ten_shared_ptr_destroy(cmd);
  self->on_server_connected_cmd = NULL;

  if (self->engine->original_start_graph_cmd_of_enabling_engine) {
    // Before starting the extension system, the only reason to establish a
    // connection is to handle the 'start_graph' command, and this variable
    // enables us to know it is this case.
    ten_engine_return_error_for_cmd_start_graph(
        self->engine, self->engine->original_start_graph_cmd_of_enabling_engine,
        "Failed to connect to %s", ten_string_get_raw_str(&self->uri));
  } else {
    // The remote fails to connect to the target, so it's time to close it.
    ten_remote_close(self);
  }
}

void ten_engine_connect_to_graph_remote(ten_engine_t *self, const char *uri,
                                        ten_shared_ptr_t *cmd) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(uri, "Invalid argument.");

  TEN_ASSERT(cmd && ten_msg_get_type(cmd) == TEN_MSG_TYPE_CMD_START_GRAPH,
             "Should not happen.");

  TEN_LOGD("Trying to connect to %s inside graph.", uri);

  ten_remote_t *remote = ten_engine_create_remote(self, uri);
  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  // This channel might be duplicated with other channels between this TEN app
  // and the remote TEN app. This situation may appear in a graph which
  // contains loops.
  //
  //                   ------->
  //  ----> TEN app 1            TEN app 2 <-----
  //                   <-------
  //
  // If it's this case, this 'channel' would be destroyed later, so we put
  // this 'channel' (i.e., the 'remote' here) to a tmp list (weak remotes)
  // first to prevent any messages from flowing through this channel.
  ten_engine_add_weak_remote(self, remote);

  ten_remote_connect_to(remote, ten_engine_on_graph_remote_connected, cmd,
                        ten_engine_on_graph_remote_connect_error);
}

void ten_engine_route_msg_to_remote(ten_engine_t *self, ten_shared_ptr_t *msg) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(
      msg && ten_msg_check_integrity(msg) && ten_msg_get_dest_cnt(msg) == 1,
      "Should not happen.");

  const char *dest_uri = ten_msg_get_first_dest_uri(msg);
  ten_remote_t *remote = ten_engine_find_remote(self, dest_uri);

  if (remote) {
    ten_remote_send_msg(remote, msg);
  } else {
    TEN_LOGW("Could not find suitable remote based on uri: %s", dest_uri);
  }

  // It's unnecessary to search weak remotes, because weak remotes are not
  // ready to transfer messages.
}

ten_remote_t *ten_engine_check_remote_is_existed(ten_engine_t *self,
                                                 const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(uri, "Should not happen.");

  ten_remote_t *remote = NULL;

  // 1. Check if the remote is in the 'remotes' list.
  ten_hashhandle_t *hh = ten_hashtable_find_string(&self->remotes, uri);
  if (hh) {
#if defined(_DEBUG)
    size_t weak_remote_cnt = ten_list_find_ptr_cnt_custom(
        &self->weak_remotes, uri, ten_remote_is_uri_equal_to);
    TEN_ASSERT(
        weak_remote_cnt <= 1,
        "A remote might appear in 'remote' and 'weak_remote' once when the "
        "graph contains a loop. This is the case of 'duplicate' connection.");
#endif

    remote = CONTAINER_OF_FROM_FIELD(hh, ten_remote_t, hh_in_remote_table);
    TEN_ASSERT(remote, "Invalid argument.");
    TEN_ASSERT(ten_remote_check_integrity(remote, true),
               "Invalid use of remote %p.", remote);

    TEN_LOGD("remote %p for uri '%s' is found in 'remotes' list.", remote, uri);

    return remote;
  }

  // 2. Check if the remote is in the 'weak_remotes' list.
  ten_listnode_t *found = ten_list_find_ptr_custom(&self->weak_remotes, uri,
                                                   ten_remote_is_uri_equal_to);

  if (found) {
    remote = ten_ptr_listnode_get(found);
    TEN_ASSERT(remote, "Invalid argument.");
    TEN_ASSERT(ten_remote_check_integrity(remote, true),
               "Invalid use of remote %p.", remote);
  }

  TEN_LOGD("remote %p for uri '%s' is%s in 'weak_remotes' list.", remote, uri,
           remote ? "" : " not");

  return remote;
}

// This function is used to solve the connection duplication problem. If there
// are two physical connections between two TEN apps, the connection which
// connects a TEN app with a smaller URI to a TEN app with a larger URI would be
// keep, and the other connection would be dropped.
//
//                   ------->
//  ----> TEN app 1            TEN app 2 <----
//                   <-------
bool ten_engine_check_remote_is_duplicated(ten_engine_t *self,
                                           const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(uri, "Should not happen.");

  ten_remote_t *remote = ten_engine_check_remote_is_existed(self, uri);
  if (remote) {
    TEN_LOGW("Found a remote %s (%p), checking duplication...", uri, remote);

    if (ten_c_string_is_equal_or_smaller(
            uri, ten_string_get_raw_str(ten_app_get_uri(self->app)))) {
      TEN_LOGW(" > Remote %s (%p) is smaller, this channel is duplicated.", uri,
               remote);
      return true;
    } else {
      TEN_LOGW(" > Remote %s (%p) is larger, keep this channel.", uri, remote);
    }
  }

  return false;
}

bool ten_engine_check_remote_is_weak(ten_engine_t *self, ten_remote_t *remote) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(remote, "Invalid argument.");
  TEN_ASSERT(ten_remote_check_integrity(remote, true),
             "Invalid use of remote %p.", remote);

  ten_listnode_t *found = ten_list_find_ptr(&self->weak_remotes, remote);

  TEN_LOGD("remote %p is%s weak.", remote, found ? "" : " not");

  return found != NULL;
}

bool ten_engine_receive_msg_from_remote(ten_remote_t *remote,
                                        ten_shared_ptr_t *msg,
                                        TEN_UNUSED void *user_data) {
  TEN_ASSERT(remote && ten_remote_check_integrity(remote, true),
             "Should not happen.");

  ten_engine_t *engine = remote->engine;
  TEN_ASSERT(engine, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(engine, true),
             "Invalid use of engine %p.", engine);

  // Assign the current engine as the message _source_ if there is none, so
  // that if this message traverse to another graph, the result could find the
  // way home.
  ten_msg_set_src_engine_if_unspecified(msg, engine);

  if (!ten_loc_is_empty(&remote->explicit_dest_loc)) {
    // If TEN runtime has explicitly setup the destination location where all
    // the messages coming from this remote should go, adjust the destination of
    // the message according to this.

    ten_msg_clear_and_set_dest_to_loc(msg, &remote->explicit_dest_loc);
  } else {
    // The default destination engine would be the engine where this remote
    // attached to, if the message doesn't specify one.
    ten_msg_set_dest_engine_if_unspecified_or_predefined_graph_name(
        msg, engine, &engine->app->predefined_graph_infos);
  }

  if (ten_engine_is_ready_to_handle_msg(engine)) {
    ten_engine_dispatch_msg(engine, msg);
  } else {
    switch (ten_msg_get_type(msg)) {
      case TEN_MSG_TYPE_CMD_START_GRAPH: {
        // The 'start_graph' command could only be handled once in a graph.
        // Therefore, if we receive a new 'start_graph' command after the graph
        // has been established, just ignore this 'start_graph' command.

        ten_shared_ptr_t *cmd_result =
            ten_cmd_result_create_from_cmd(TEN_STATUS_CODE_ERROR, msg);
        ten_msg_set_property(
            cmd_result, "detail",
            ten_value_create_string(
                "Receive a start_graph cmd after graph is built."),
            NULL);
        ten_connection_send_msg(remote->connection, cmd_result);
        ten_shared_ptr_destroy(cmd_result);
        break;
      }

      case TEN_MSG_TYPE_CMD_RESULT:
        ten_engine_dispatch_msg(engine, msg);
        break;

      default:
        ten_engine_append_to_in_msgs_queue(engine, msg);
        break;
    }
  }

  return true;
}

ten_remote_t *ten_engine_find_remote(ten_engine_t *self, const char *uri) {
  TEN_ASSERT(self, "Invalid argument.");
  TEN_ASSERT(ten_engine_check_integrity(self, true),
             "Invalid use of engine %p.", self);

  TEN_ASSERT(uri, "Should not happen.");

  ten_hashhandle_t *hh = ten_hashtable_find_string(&self->remotes, uri);
  if (hh) {
    return CONTAINER_OF_FROM_FIELD(hh, ten_remote_t, hh_in_remote_table);
  }

  return NULL;
}
