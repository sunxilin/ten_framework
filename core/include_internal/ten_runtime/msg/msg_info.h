//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "include_internal/ten_runtime/common/constant_str.h"
#include "include_internal/ten_runtime/engine/msg_interface/close_app.h"
#include "include_internal/ten_runtime/engine/msg_interface/cmd_result.h"
#include "include_internal/ten_runtime/engine/msg_interface/start_graph.h"
#include "include_internal/ten_runtime/engine/msg_interface/stop_graph.h"
#include "include_internal/ten_runtime/engine/msg_interface/timer.h"
#include "include_internal/ten_runtime/msg/audio_frame/audio_frame.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/close_app/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/custom/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/start_graph/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/stop_graph/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/timeout/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd/timer/cmd.h"
#include "include_internal/ten_runtime/msg/cmd_base/cmd_result/cmd.h"
#include "include_internal/ten_runtime/msg/data/data.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "include_internal/ten_runtime/msg/video_frame/video_frame.h"
#include "ten_runtime/msg/cmd/custom/cmd.h"
#include "ten_runtime/msg/cmd/start_graph/cmd.h"
#include "ten_runtime/msg/msg.h"

typedef void (*ten_msg_engine_handler_func_t)(ten_engine_t *engine,
                                              ten_shared_ptr_t *msg,
                                              ten_error_t *err);

typedef bool (*ten_raw_msg_init_from_json_func_t)(ten_msg_t *msg,
                                                  ten_json_t *json,
                                                  ten_error_t *err);

typedef ten_msg_t *(*ten_raw_msg_create_from_json_func_t)(ten_json_t *json,
                                                          ten_error_t *err);

typedef ten_json_t *(*ten_raw_msg_to_json_func_t)(ten_msg_t *msg,
                                                  ten_error_t *err);

typedef ten_msg_t *(*ten_raw_msg_clone_func_t)(ten_msg_t *msg,
                                               ten_list_t *excluded_field_ids);

typedef void (*ten_raw_msg_destroy_func_t)(ten_msg_t *msg);

typedef bool (*ten_raw_msg_loop_all_fields_func_t)(
    ten_msg_t *msg, ten_raw_msg_process_one_field_func_t cb, void *user_data,
    ten_error_t *err);

typedef bool (*ten_raw_msg_validate_schema_func_t)(
    ten_msg_t *msg, ten_schema_store_t *schema_store, bool is_msg_out,
    ten_error_t *err);

typedef bool (*ten_raw_msg_set_ten_property_func_t)(ten_msg_t *msg,
                                                    ten_list_t *paths,
                                                    ten_value_t *value,
                                                    ten_error_t *err);

typedef ten_value_t *(*ten_raw_msg_peek_ten_property_func_t)(ten_msg_t *msg,
                                                             ten_list_t *paths,
                                                             ten_error_t *err);

typedef bool (*ten_raw_msg_check_type_and_name_func_t)(ten_msg_t *self,
                                                       const char *type_str,
                                                       const char *name_str,
                                                       ten_error_t *err);

typedef struct ten_msg_info_t {
  const char *msg_type_name;
  const char *msg_unique_name;

  bool create_in_path;
  ten_msg_engine_handler_func_t engine_handler;

  ten_raw_msg_clone_func_t clone;
  ten_raw_msg_init_from_json_func_t init_from_json;
  ten_raw_msg_create_from_json_func_t create_from_json;
  ten_raw_msg_to_json_func_t to_json;
  ten_raw_msg_loop_all_fields_func_t loop_all_fields;
  ten_raw_msg_validate_schema_func_t validate_schema;
  ten_raw_msg_set_ten_property_func_t set_ten_property;
  ten_raw_msg_peek_ten_property_func_t peek_ten_property;
  ten_raw_msg_check_type_and_name_func_t check_type_and_name;
} ten_msg_info_t;

#define TEN_MSG_INFO_INIT_VALUES       \
  {                                    \
      NULL,  /* msg_type_name */       \
      NULL,  /* unique_name */         \
      false, /* create_in_path */      \
      NULL,  /* engine_handler */      \
      NULL,  /* clone */               \
      NULL,  /* init_from_json */      \
      NULL,  /* create_from_json */    \
      NULL,  /* to_json */             \
      NULL,  /* loop_all_fields */     \
      NULL,  /* validate_schema */     \
      NULL,  /* set_ten_property */    \
      NULL,  /* peek_ten_property */   \
      NULL,  /* check_type_and_name */ \
  }

TEN_UNUSED static const ten_msg_info_t ten_msg_info[] = {
    [TEN_MSG_TYPE_INVALID] = TEN_MSG_INFO_INIT_VALUES,
    [TEN_MSG_TYPE_CMD] =
        {
            TEN_STR_CMD, /* msg_type_name */

            // General cmd do not have a special name.
            NULL, /* msg_unique_name */

            true, /* create_in_path */

            // The engine does not need to handle general cmd.
            NULL, /* engine_handler */

            ten_raw_cmd_custom_as_msg_clone,            /* clone */
            ten_raw_cmd_custom_as_msg_init_from_json,   /* init_from_json */
            ten_raw_cmd_custom_as_msg_create_from_json, /* create_from_json */
            ten_raw_cmd_custom_to_json,                 /* to_json*/
            NULL,                                       /* loop_all_fields */
            ten_raw_msg_validate_schema,                /* validate_schema */
            ten_raw_cmd_custom_set_ten_property,        /* set_ten_property */
            NULL,                                       /* peek_ten_property */
            ten_raw_cmd_custom_check_type_and_name, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_STOP_GRAPH] =
        {
            TEN_STR_STOP_GRAPH,                           /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_STOP_GRAPH,              /* msg_unique_name */
            false,                                        /* create_in_path */
            ten_engine_handle_cmd_stop_graph,             /* engine_handler */
            NULL,                                         /* clone */
            ten_raw_cmd_stop_graph_as_msg_init_from_json, /* init_from_json */
            ten_raw_cmd_stop_graph_as_msg_create_from_json, /* create_from_json
                                                             */
            ten_raw_cmd_stop_graph_to_json,                 /* to_json */
            NULL, /* loop_all_fields */
            NULL, /* validate_schema */
            NULL, /* set_ten_property */
            NULL, /* peek_ten_property */
            NULL, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_START_GRAPH] =
        {
            TEN_STR_START_GRAPH,                           /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_START_GRAPH,              /* msg_unique_name */
            true,                                          /* create_in_path */
            ten_engine_handle_cmd_start_graph,             /* engine_handler */
            ten_raw_cmd_start_graph_as_msg_clone,          /* clone */
            ten_raw_cmd_start_graph_as_msg_init_from_json, /* init_from_json */
            ten_raw_cmd_start_graph_as_msg_create_from_json, /* create_from_json
                                                              */
            ten_raw_cmd_start_graph_to_json,                 /* to_json */
            NULL, /* loop_all_fields */
            NULL, /* validate_schema */
            NULL, /* set_ten_property */
            NULL, /* peek_ten_property */
            NULL, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_RESULT] =
        {
            TEN_STR_RESULT,                             /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_RESULT,                /* msg_unique_name */
            false,                                      /* create_in_path */
            ten_engine_handle_cmd_result,               /* engine_handler */
            ten_raw_cmd_result_as_msg_clone,            /* clone */
            ten_raw_cmd_result_as_msg_init_from_json,   /* init_from_json */
            ten_raw_cmd_result_as_msg_create_from_json, /* create_from_json */
            ten_raw_cmd_result_as_msg_to_json,          /* to_json */
            NULL,                                       /* loop_all_fields */
            ten_raw_cmd_result_validate_schema,         /* validate_schema */
            NULL,                                       /* set_ten_property */
            NULL,                                       /* peek_ten_property */
            NULL, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_CLOSE_APP] =
        {
            TEN_STR_CLOSE_APP,                             /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_CLOSE_APP,                /* msg_unique_name */
            false,                                         /* create_in_path */
            ten_engine_handle_cmd_close_app,               /* engine_handler */
            NULL,                                          /* clone */
            ten_raw_cmd_close_app_as_msg_init_from_json,   /* init_from_json*/
            ten_raw_cmd_close_app_as_msg_create_from_json, /* create_from_json*/
            ten_raw_cmd_close_app_to_json,                 /* to_json */
            NULL,                                          /* loop_all_fields */
            NULL,                                          /* validate_schema */
            NULL, /* set_ten_property */
            NULL, /* peek_ten_property */
            NULL, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_TIMEOUT] =
        {
            TEN_STR_TIMEOUT,                             /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_TIMEOUT,                /* msg_unique_name */
            false,                                       /* create_in_path */
            NULL,                                        /* engine_handler */
            NULL,                                        /* clone */
            ten_raw_cmd_timeout_as_msg_init_from_json,   /* init_from_json */
            ten_raw_cmd_timeout_as_msg_create_from_json, /* create_from_json */
            ten_raw_cmd_timeout_as_msg_to_json,          /* to_json */
            NULL,                                        /* loop_all_fields */
            NULL,                                        /* validate_schema */
            NULL,                                        /* set_ten_property */
            NULL,                                        /* peek_ten_property */
            NULL, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_CMD_TIMER] =
        {
            TEN_STR_TIMER,                             /* msg_type_name */
            TEN_STR_MSG_NAME_TEN_TIMER,                /* msg_unique_name */
            true,                                      /* create_in_path */
            ten_engine_handle_cmd_timer,               /* engine_handler */
            NULL,                                      /* clone */
            ten_raw_cmd_timer_as_msg_init_from_json,   /* init_from_json */
            ten_raw_cmd_timer_as_msg_create_from_json, /* create_from_json */
            ten_raw_cmd_timer_as_msg_to_json,          /* to_json */
            NULL,                                      /* loop_all_fields */
            NULL,                                      /* validate_schema */
            ten_raw_cmd_timer_set_ten_property,        /* set_ten_property */
            NULL,                                      /* peek_ten_property */
            ten_raw_cmd_timer_check_type_and_name,     /* check_type_and_name */
        },
    [TEN_MSG_TYPE_DATA] =
        {
            TEN_STR_DATA, /* msg_type_name */

            // General data do not have a special name.
            NULL, /* msg_unique_name */

            false, /* create_in_path */

            // The engine does not need to handle general data.
            NULL, /* engine_handler */

            ten_raw_data_as_msg_clone,            /* clone */
            ten_raw_data_as_msg_init_from_json,   /* init_from_json */
            ten_raw_data_as_msg_create_from_json, /* create_from_json */
            ten_raw_data_as_msg_to_json,          /* to_json */
            ten_raw_data_loop_all_fields,         /* loop_all_fields */
            ten_raw_msg_validate_schema,          /* validate_schema */
            ten_raw_data_like_set_ten_property,   /* set_ten_property */
            NULL,                                 /* peek_ten_property */
            ten_raw_data_check_type_and_name,     /* check_type_and_name */
        },
    [TEN_MSG_TYPE_AUDIO_FRAME] =
        {
            TEN_STR_AUDIO_FRAME, /* msg_type_name */

            // General audio frame do not have a special name.
            NULL, /* msg_unique_name */

            false, /* create_in_path */

            // The engine does not need to handle general audio frame.
            NULL, /* engine_handler */

            ten_raw_audio_frame_as_msg_clone,            /* clone */
            ten_raw_audio_frame_as_msg_init_from_json,   /* init_from_json */
            ten_raw_audio_frame_as_msg_create_from_json, /* create_from_json */
            ten_raw_audio_frame_as_msg_to_json,          /* to_json */
            NULL,                                        /* loop_all_fields */
            ten_raw_msg_validate_schema,                 /* validate_schema */
            ten_raw_data_like_set_ten_property,          /* set_ten_property */
            ten_raw_audio_frame_peek_ten_property,       /* peek_ten_property */
            ten_raw_audio_frame_check_type_and_name, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_VIDEO_FRAME] =
        {
            TEN_STR_VIDEO_FRAME, /* msg_type_name */

            // General video frame do not have a special name.
            NULL, /* msg_unique_name */

            false, /* create_in_path */

            // The engine does not need to handle general video frame.
            NULL, /* engine_handler */

            ten_raw_video_frame_as_msg_clone,            /* clone */
            ten_raw_video_frame_as_msg_init_from_json,   /* init_from_json */
            ten_raw_video_frame_as_msg_create_from_json, /* create_from_json */
            ten_raw_video_frame_as_msg_to_json,          /* to_json */
            NULL,                                        /* loop_all_fields */
            ten_raw_msg_validate_schema,                 /* validate_schema */
            ten_raw_video_frame_set_ten_property,        /* set_ten_property */
            ten_raw_video_frame_peek_ten_property,       /* peek_ten_property */
            ten_raw_video_frame_check_type_and_name, /* check_type_and_name */
        },
    [TEN_MSG_TYPE_LAST] = TEN_MSG_INFO_INIT_VALUES,
};

static const size_t ten_msg_info_size =
    sizeof(ten_msg_info) / sizeof(ten_msg_info[0]);
