//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <cstddef>
#include <memory>

#include "ten_runtime/binding/cpp/internal/msg/msg.h"
#include "ten_runtime/msg/audio_frame/audio_frame.h"
#include "ten_utils/lib/smart_ptr.h"

namespace ten {

class ten_env_t;
class extension_t;

class audio_frame_t : public msg_t {
 private:
  friend extension_t;

  // Passkey Idiom.
  struct ctor_passkey_t {
   private:
    friend audio_frame_t;

    explicit ctor_passkey_t() = default;
  };

 public:
  static std::unique_ptr<audio_frame_t> create(const char *audio_frame_name,
                                               error_t *err = nullptr) {
    if (audio_frame_name == nullptr || strlen(audio_frame_name) == 0) {
      if (err != nullptr && err->get_internal_representation() != nullptr) {
        ten_error_set(err->get_internal_representation(),
                      TEN_ERRNO_INVALID_ARGUMENT,
                      "audio frame name cannot be empty.");
      }
      return nullptr;
    }

    auto *c_frame = ten_audio_frame_create();
    ten_msg_set_name(
        c_frame, audio_frame_name,
        err != nullptr ? err->get_internal_representation() : nullptr);

    return std::make_unique<audio_frame_t>(c_frame, ctor_passkey_t());
  }

  static std::unique_ptr<audio_frame_t> create_from_json(const char *json_str,
                                                         error_t *err = nullptr)
      __attribute__((warning("This method may access the '_ten' field. Use "
                             "caution if '_ten' is provided."))) {
    ten_shared_ptr_t *c_audio_frame = ten_audio_frame_create_from_json_string(
        json_str,
        err != nullptr ? err->get_internal_representation() : nullptr);

    return std::make_unique<audio_frame_t>(c_audio_frame, ctor_passkey_t());
  }

  explicit audio_frame_t(ten_shared_ptr_t *audio_frame,
                         ctor_passkey_t /*unused*/)
      : msg_t(audio_frame) {}

  ~audio_frame_t() override = default;

  int64_t get_timestamp(error_t *err = nullptr) const {
    return ten_audio_frame_get_timestamp(c_msg_);
  }
  bool set_timestamp(int64_t timestamp, error_t *err = nullptr) {
    return ten_audio_frame_set_timestamp(c_msg_, timestamp);
  }

  int32_t get_sample_rate(error_t *err = nullptr) const {
    return ten_audio_frame_get_sample_rate(c_msg_);
  }
  bool set_sample_rate(int32_t sample_rate, error_t *err = nullptr) {
    return ten_audio_frame_set_sample_rate(c_msg_, sample_rate);
  }

  uint64_t get_channel_layout(error_t *err = nullptr) const {
    return ten_audio_frame_get_channel_layout(c_msg_);
  }
  bool set_channel_layout(uint64_t channel_layout, error_t *err = nullptr) {
    return ten_audio_frame_set_channel_layout(c_msg_, channel_layout);
  }

  int32_t get_samples_per_channel(error_t *err = nullptr) const {
    return ten_audio_frame_get_samples_per_channel(c_msg_);
  }
  bool set_samples_per_channel(int32_t samples_per_channel,
                               error_t *err = nullptr) {
    return ten_audio_frame_set_samples_per_channel(c_msg_, samples_per_channel);
  }

  int32_t get_bytes_per_sample(error_t *err = nullptr) const {
    return ten_audio_frame_get_bytes_per_sample(c_msg_);
  }
  bool set_bytes_per_sample(int32_t size, error_t *err = nullptr) {
    return ten_audio_frame_set_bytes_per_sample(c_msg_, size);
  }

  int32_t get_number_of_channels(error_t *err = nullptr) const {
    return ten_audio_frame_get_number_of_channel(c_msg_);
  }
  bool set_number_of_channels(int32_t number, error_t *err = nullptr) {
    return ten_audio_frame_set_number_of_channel(c_msg_, number);
  }

  TEN_AUDIO_FRAME_DATA_FMT get_data_fmt(error_t *err = nullptr) const {
    return ten_audio_frame_get_data_fmt(c_msg_);
  }
  bool set_data_fmt(TEN_AUDIO_FRAME_DATA_FMT format, error_t *err = nullptr) {
    return ten_audio_frame_set_data_fmt(c_msg_, format);
  }

  int32_t get_line_size(error_t *err = nullptr) const {
    return ten_audio_frame_get_line_size(c_msg_);
  }
  bool set_line_size(int32_t line_size, error_t *err = nullptr) {
    return ten_audio_frame_set_line_size(c_msg_, line_size);
  }

  bool is_eof(error_t *err = nullptr) const {
    return ten_audio_frame_is_eof(c_msg_);
  }
  bool set_is_eof(bool is_eof, error_t *err = nullptr) {
    return ten_audio_frame_set_is_eof(c_msg_, is_eof);
  }

  bool alloc_buf(size_t size, error_t *err = nullptr) {
    return ten_audio_frame_alloc_data(c_msg_, size) != nullptr;
  }

  buf_t lock_buf(error_t *err = nullptr) const {
    ten_buf_t *data = ten_audio_frame_peek_data(c_msg_);

    if (!ten_msg_add_locked_res_buf(
            c_msg_, data->data,
            err != nullptr ? err->get_internal_representation() : nullptr)) {
      return buf_t{};
    }

    buf_t result{data->data, data->size};

    return result;
  }

  bool unlock_buf(buf_t &buf, error_t *err = nullptr) {
    const uint8_t *data = buf.data();
    if (!ten_msg_remove_locked_res_buf(
            c_msg_, data,
            err != nullptr ? err->get_internal_representation() : nullptr)) {
      return false;
    }

    // Since the `buf` has already been given back, clearing the contents of the
    // `buf` itself not only notifies developers that this `buf` can no longer
    // be used, but also prevents it from being used incorrectly again.
    ten_buf_init_with_owned_data(&buf.buf, 0);

    return true;
  }

  // @{
  audio_frame_t(const audio_frame_t &other) = delete;
  audio_frame_t(audio_frame_t &&other) noexcept = delete;
  audio_frame_t &operator=(const audio_frame_t &other) = delete;
  audio_frame_t &operator=(audio_frame_t &&other) noexcept = delete;
  // @}

  // @{
  // Internal use only. This function is called in 'extension_t' to create C++
  // message from C message.
  explicit audio_frame_t(ten_shared_ptr_t *frame) : msg_t(frame) {}
  // @}

 private:
  friend class extension_t;
};

}  // namespace ten
