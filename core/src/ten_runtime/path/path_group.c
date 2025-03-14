//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/path/path_group.h"

#include "include_internal/ten_runtime/extension/extension.h"
#include "include_internal/ten_runtime/extension_thread/extension_thread.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/path/path.h"
#include "ten_runtime/common/status_code.h"
#include "ten_runtime/msg/cmd_result/cmd_result.h"
#include "ten_utils/container/list.h"
#include "ten_utils/container/list_ptr.h"
#include "ten_utils/lib/alloc.h"
#include "ten_utils/lib/smart_ptr.h"
#include "ten_utils/macro/check.h"

bool ten_path_group_check_integrity(ten_path_group_t *self, bool check_thread) {
  TEN_ASSERT(self, "Should not happen.");

  if (ten_signature_get(&self->signature) != TEN_PATH_GROUP_SIGNATURE) {
    return false;
  }

  if (check_thread) {
    // In case the extension thread may be in lock_mode, we utilize
    // extension_thread_check_integrity to handle this scenario.
    if (self->table->attach_to == TEN_PATH_TABLE_ATTACH_TO_EXTENSION) {
      ten_extension_thread_t *extension_thread =
          self->table->attached_target.extension->extension_thread;
      return ten_extension_thread_check_integrity(extension_thread, true);
    }

    return ten_sanitizer_thread_check_do_check(&self->thread_check);
  }

  return true;
}

/**
 * @brief Checks whether a given path is associated with a group or not.
 *
 * @return true if the path is part of a group, otherwise, it returns false.
 */
bool ten_path_is_in_a_group(ten_path_t *self) {
  TEN_ASSERT(self, "Invalid argument.");
  return self->group ? true : false;
}

/**
 * @brief The three functions @name ten_path_group_create, @name
 * ten_path_group_create_master, and @name ten_path_group_create_slave create a
 * new path group, a master group, and a slave group, respectively. They
 * allocate memory for the structure, initialize various components of the path
 * group, and return the group.
 */
static ten_path_group_t *ten_path_group_create(ten_path_table_t *table,
                                               TEN_PATH_GROUP_ROLE role) {
  TEN_ASSERT(role != TEN_PATH_GROUP_ROLE_INVALID, "Invalid argument.");

  ten_path_group_t *self =
      (ten_path_group_t *)TEN_MALLOC(sizeof(ten_path_group_t));
  TEN_ASSERT(self, "Failed to allocate memory.");

  self->table = table;
  self->role = role;

  return self;
}

static ten_path_group_t *ten_path_group_create_master(
    ten_path_table_t *table, TEN_PATH_GROUP_POLICY policy) {
  TEN_ASSERT(policy != TEN_PATH_GROUP_POLICY_INVALID, "Invalid argument.");

  ten_path_group_t *self =
      ten_path_group_create(table, TEN_PATH_GROUP_ROLE_MASTER);

  self->master.policy = policy;
  ten_list_init(&self->master.members);

  return self;
}

static ten_path_group_t *ten_path_group_create_slave(ten_path_table_t *table,
                                                     ten_path_t *master) {
  TEN_ASSERT(master, "Invalid argument.");

  ten_path_group_t *self =
      ten_path_group_create(table, TEN_PATH_GROUP_ROLE_SLAVE);

  self->slave.master = master;

  return self;
}

/**
 * @brief Frees the memory allocated for the path group.
 */
void ten_path_group_destroy(ten_path_group_t *self) {
  TEN_ASSERT(self, "Invalid argument.");

  if (self->role == TEN_PATH_GROUP_ROLE_MASTER) {
    ten_list_clear(&self->master.members);
  }

  TEN_FREE(self);
}

/**
 * @brief Creates a group of paths. It takes a list of paths as an argument and
 * initializes each path with a group. The first path in the list is designated
 * as the master, and the rest are designated as slaves. It also initializes the
 * members of the master group.
 */
void ten_paths_create_group(ten_list_t *paths, TEN_PATH_GROUP_POLICY policy) {
  TEN_ASSERT(paths, "Invalid argument.");
  TEN_ASSERT(ten_list_size(paths) > 1, "Invalid argument.");

  ten_path_t *master = NULL;
  ten_list_foreach (paths, iter) {
    ten_path_t *path = ten_ptr_listnode_get(iter.node);
    TEN_ASSERT(path && ten_path_check_integrity(path, true),
               "Invalid argument.");
    TEN_ASSERT(path->table, "Invalid argument.");

    if (iter.index == 0) {
      // It's a master.
      master = path;
      path->group = ten_path_group_create_master(path->table, policy);
    } else {
      path->group = ten_path_group_create_slave(path->table, master);
    }

    // The 'master' is one of the members, too.
    ten_list_push_ptr_back(&master->group->master.members, path, NULL);
  }
}

/**
 * @brief Takes a list of paths and a path type as arguments and checks the
 * status of each path in the group. If any of the paths fail, it returns that
 * path, otherwise, it returns the first or last path in the list, depending on
 * the parameter `return_last`. This function is used to resolve the status of a
 * group of paths.
 */
static ten_path_t *ten_path_group_resolve_in_one_fail_and_all_ok_return(
    ten_list_t *members, TEN_UNUSED TEN_PATH_TYPE path_type, bool return_last) {
  TEN_ASSERT(members && ten_list_check_integrity(members), "Invalid argument.");

  bool has_received_all_cmd_results_in_group = true;

  ten_list_foreach (members, iter) {
    ten_path_t *path = ten_ptr_listnode_get(iter.node);
    TEN_ASSERT(path && ten_path_check_integrity(path, true),
               "Invalid argument.");

    ten_shared_ptr_t *cmd_result = path->cached_cmd_result;
    if (cmd_result) {
      TEN_ASSERT(ten_msg_get_type(cmd_result) == TEN_MSG_TYPE_CMD_RESULT,
                 "Invalid argument.");
    }

    if (cmd_result) {
      if (ten_cmd_result_get_status_code(cmd_result) != TEN_STATUS_CODE_OK) {
        // Receive a fail result, return it.
        return path;
      }
    } else {
      has_received_all_cmd_results_in_group = false;
    }
  }

  if (has_received_all_cmd_results_in_group) {
    // Receive all cmd results, the purpose of the group is completed, too.
    // Therefore, return the specified cmd result.
    ten_path_t *path = ten_ptr_listnode_get(
        return_last ? ten_list_back(members) : ten_list_front(members));
    TEN_ASSERT(path && ten_path_check_integrity(path, true),
               "Invalid argument.");
    TEN_ASSERT(path->cached_cmd_result, "Should not happen.");

    return path;
  }

  return NULL;
}

/**
 * @brief Takes a path as an argument and returns the master path of the group
 * that the path belongs to.
 */
ten_path_t *ten_path_group_get_master(ten_path_t *path) {
  TEN_ASSERT(path && ten_path_check_integrity(path, true), "Invalid argument.");
  TEN_ASSERT(ten_path_is_in_a_group(path), "Invalid argument.");

  ten_path_t *master = NULL;
  switch (path->group->role) {
    case TEN_PATH_GROUP_ROLE_MASTER:
      master = path;
      break;
    case TEN_PATH_GROUP_ROLE_SLAVE:
      master = path->group->slave.master;
      break;
    default:
      TEN_ASSERT(0, "Should not happen.");
      break;
  }

  return master;
}

/**
 * @brief Takes a path as an argument and returns a list of all the paths that
 * belong to the same group as the given path.
 */
ten_list_t *ten_path_group_get_members(ten_path_t *path) {
  TEN_ASSERT(path && ten_path_check_integrity(path, true), "Invalid argument.");
  TEN_ASSERT(ten_path_is_in_a_group(path), "Invalid argument.");

  ten_path_t *master = ten_path_group_get_master(path);
  TEN_ASSERT(master, "Should not happen.");

  ten_list_t *members = &master->group->master.members;
  TEN_ASSERT(members && ten_list_check_integrity(members),
             "Should not happen.");

  return members;
}

/**
 * @brief Resolve the path group, where @a path belongs to, to a final cmd
 * result.
 *
 * This function takes a path and a path type as arguments and resolves the path
 * group to a final cmd result. It checks the policy of the path group and calls
 * the appropriate function to resolve the path group status.
 *
 * Policy `TEN_PATH_GROUP_POLICY_ONE_FAIL_RETURN_AND_ALL_OK_RETURN_FIRST`
 * returns the first path in the list if all paths have succeeded, and returns
 * the first path that has failed otherwise.
 */
ten_path_t *ten_path_group_resolve(ten_path_t *path, TEN_PATH_TYPE type) {
  TEN_ASSERT(path && ten_path_check_integrity(path, true), "Invalid argument.");
  TEN_ASSERT(ten_path_is_in_a_group(path), "Invalid argument.");

  ten_path_t *master = ten_path_group_get_master(path);
  TEN_ASSERT(master, "Should not happen.");

  ten_list_t *members = &(master->group->master.members);
  TEN_ASSERT(members && ten_list_check_integrity(members),
             "Should not happen.");

  switch (master->group->master.policy) {
    case TEN_PATH_GROUP_POLICY_ONE_FAIL_RETURN_AND_ALL_OK_RETURN_FIRST:
      return ten_path_group_resolve_in_one_fail_and_all_ok_return(members, type,
                                                                  false);
    case TEN_PATH_GROUP_POLICY_ONE_FAIL_RETURN_AND_ALL_OK_RETURN_LAST:
      return ten_path_group_resolve_in_one_fail_and_all_ok_return(members, type,
                                                                  true);
    default:
      TEN_ASSERT(0, "Should not happen.");
      return NULL;
  }

  TEN_ASSERT(0, "Should not happen.");
}
