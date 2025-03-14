//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/msg/audio_frame/field/samples_per_channel.h"

#include "include_internal/ten_runtime/common/constant_str.h"
#include "include_internal/ten_runtime/msg/audio_frame/audio_frame.h"
#include "include_internal/ten_runtime/msg/msg.h"
#include "ten_runtime/msg/audio_frame/audio_frame.h"
#include "ten_runtime/msg/msg.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/macro/check.h"

bool ten_audio_frame_put_samples_per_channel_to_json(ten_msg_t *self,
                                                     ten_json_t *json,
                                                     ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");

  ten_json_t *ten_json =
      ten_json_object_peek_object_forcibly(json, TEN_STR_UNDERLINE_TEN);
  TEN_ASSERT(ten_json, "Should not happen.");

  ten_json_object_set_new(
      ten_json, TEN_STR_SAMPLES_PER_CHANNEL,
      ten_json_create_integer(ten_raw_audio_frame_get_samples_per_channel(
          (ten_audio_frame_t *)self)));

  return true;
}

bool ten_audio_frame_get_samples_per_channel_from_json(
    ten_msg_t *self, ten_json_t *json, TEN_UNUSED ten_error_t *err) {
  TEN_ASSERT(self && ten_raw_msg_check_integrity(self) &&
                 ten_raw_msg_get_type(self) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");
  TEN_ASSERT(json && ten_json_check_integrity(json), "Should not happen.");

  ten_json_t *ten_json =
      ten_json_object_peek_object(json, TEN_STR_UNDERLINE_TEN);
  if (!ten_json) {
    return true;
  }

  int64_t samples_per_channel =
      ten_json_object_get_integer(ten_json, TEN_STR_SAMPLES_PER_CHANNEL);
  ten_raw_audio_frame_set_samples_per_channel((ten_audio_frame_t *)self,
                                              (int32_t)samples_per_channel);

  return true;
}

void ten_audio_frame_copy_samples_per_channel(
    ten_msg_t *self, ten_msg_t *src,
    TEN_UNUSED ten_list_t *excluded_field_ids) {
  TEN_ASSERT(self && src && ten_raw_msg_check_integrity(src) &&
                 ten_raw_msg_get_type(src) == TEN_MSG_TYPE_AUDIO_FRAME,
             "Should not happen.");

  ten_raw_audio_frame_set_samples_per_channel(
      (ten_audio_frame_t *)self,
      ten_raw_audio_frame_get_samples_per_channel((ten_audio_frame_t *)src));
}
