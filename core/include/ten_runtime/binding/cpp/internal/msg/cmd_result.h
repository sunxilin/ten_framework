//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include "ten_runtime/ten_config.h"

#include <memory>

#include "ten_runtime/binding/cpp/internal/msg/msg.h"
#include "ten_runtime/common/status_code.h"
#include "ten_runtime/msg/cmd_result/cmd_result.h"
#include "ten_utils/lang/cpp/lib/value.h"

namespace ten {

class extension_t;

class cmd_result_t : public msg_t {
 private:
  friend extension_t;
  friend ten_env_t;

  // Passkey Idiom.
  struct ctor_passkey_t {
   private:
    friend cmd_result_t;

    explicit ctor_passkey_t() = default;
  };

 public:
  static std::unique_ptr<cmd_result_t> create(TEN_STATUS_CODE status_code,
                                              error_t *err = nullptr) {
    return std::make_unique<cmd_result_t>(status_code, ctor_passkey_t());
  }

  explicit cmd_result_t(TEN_STATUS_CODE status_code, ctor_passkey_t /*unused*/)
      : msg_t(ten_cmd_result_create(status_code)) {}
  explicit cmd_result_t(ten_shared_ptr_t *cmd, ctor_passkey_t /*unused*/)
      : msg_t(cmd) {};

  ~cmd_result_t() override = default;

  TEN_STATUS_CODE get_status_code(error_t *err = nullptr) const {
    return ten_cmd_result_get_status_code(c_msg_);
  }

  bool get_is_final(error_t *err = nullptr) const {
    return ten_cmd_result_get_is_final(
        c_msg_, err != nullptr ? err->get_internal_representation() : nullptr);
  }

  bool set_is_final(bool is_final, error_t *err = nullptr) {
    return ten_cmd_result_set_is_final(
        c_msg_, is_final,
        err != nullptr ? err->get_internal_representation() : nullptr);
  }

  // @{
  cmd_result_t(cmd_result_t &other) = delete;
  cmd_result_t(cmd_result_t &&other) = delete;
  cmd_result_t &operator=(const cmd_result_t &cmd) = delete;
  cmd_result_t &operator=(cmd_result_t &&cmd) = delete;
  // @}

 private:
  static std::unique_ptr<cmd_result_t> create(ten_shared_ptr_t *cmd,
                                              error_t *err = nullptr) {
    return std::make_unique<cmd_result_t>(cmd, ctor_passkey_t());
  }
};

}  // namespace ten
