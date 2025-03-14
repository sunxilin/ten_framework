//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <cstddef>
#include <cstring>
#include <memory>

#include "ten_runtime/binding/cpp/internal/msg/msg.h"
#include "ten_runtime/msg/data/data.h"
#include "ten_utils/lang/cpp/lib/buf.h"
#include "ten_utils/lib/smart_ptr.h"

namespace ten {

class ten_env_t;
class extension_t;

class data_t : public msg_t {
 private:
  friend extension_t;

  // Passkey Idiom.
  struct ctor_passkey_t {
   private:
    friend data_t;

    explicit ctor_passkey_t() = default;
  };

 public:
  static std::unique_ptr<data_t> create(const char *data_name,
                                        error_t *err = nullptr) {
    if (data_name == nullptr || strlen(data_name) == 0) {
      if (err != nullptr && err->get_internal_representation() != nullptr) {
        ten_error_set(err->get_internal_representation(),
                      TEN_ERRNO_INVALID_ARGUMENT, "Data name cannot be empty.");
      }
      return nullptr;
    }

    auto *c_data = ten_data_create();
    ten_msg_set_name(
        c_data, data_name,
        err != nullptr ? err->get_internal_representation() : nullptr);

    return std::make_unique<data_t>(c_data, ctor_passkey_t());
  }

  static std::unique_ptr<data_t> create_from_json(const char *json_str,
                                                  error_t *err = nullptr)
      __attribute__((warning("This method may access the '_ten' field. Use "
                             "caution if '_ten' is provided."))) {
    ten_shared_ptr_t *c_data = ten_data_create_from_json_string(
        json_str,
        err != nullptr ? err->get_internal_representation() : nullptr);

    return std::make_unique<data_t>(c_data, ctor_passkey_t());
  }

  explicit data_t(ten_shared_ptr_t *data, ctor_passkey_t /*unused*/)
      : msg_t(data) {}

  ~data_t() override = default;

  bool alloc_buf(size_t size, error_t *err = nullptr) {
    return ten_data_alloc_buf(c_msg_, size) != nullptr;
  }

  buf_t lock_buf(error_t *err = nullptr) {
    if (!ten_msg_add_locked_res_buf(
            c_msg_, ten_data_peek_buf(c_msg_)->data,
            err != nullptr ? err->get_internal_representation() : nullptr)) {
      return {};
    }

    buf_t result{ten_data_peek_buf(c_msg_)->data,
                 ten_data_peek_buf(c_msg_)->size};

    return result;
  }

  bool unlock_buf(buf_t &buf, error_t *err = nullptr) {
    if (!ten_msg_remove_locked_res_buf(
            c_msg_, buf.data(),
            err != nullptr ? err->get_internal_representation() : nullptr)) {
      return false;
    }

    // Since the `buf` has already been given back, clearing the contents of the
    // `buf` itself not only notifies developers that this `buf` can no longer
    // be used, but also prevents it from being used incorrectly again.
    ten_buf_init_with_owned_data(&buf.buf, 0);

    return true;
  }

  // Pay attention to its copy semantics.
  buf_t get_buf(error_t *err = nullptr) const {
    size_t data_size = ten_data_peek_buf(c_msg_)->size;

    buf_t buf{data_size};
    if (data_size != 0) {
      memcpy(buf.data(), ten_data_peek_buf(c_msg_)->data, data_size);
    }
    return buf;
  }

  // @{
  data_t(data_t &other) = delete;
  data_t(data_t &&other) = delete;
  data_t &operator=(data_t &other) = delete;
  data_t &operator=(data_t &&other) = delete;
  // @}

  // @{
  // Internal use only. This function is called in 'extension_t' to create C++
  // message from C message.
  explicit data_t(ten_shared_ptr_t *data) : msg_t(data) {}
  // @}
};

}  // namespace ten
