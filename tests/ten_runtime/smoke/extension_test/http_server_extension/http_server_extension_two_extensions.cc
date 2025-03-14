//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include <libwebsockets.h>

#include <cstddef>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "gtest/gtest.h"
#include "include_internal/ten_runtime/binding/cpp/ten.h"
#include "ten_utils/lib/thread.h"
#include "tests/common/client/http.h"

namespace {

class test_extension : public ten::extension_t {
 public:
  explicit test_extension(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    nlohmann::json json = nlohmann::json::parse(cmd->to_json());
    if (json["_ten"]["name"] == "hello_world") {
      auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
      cmd_result->set_property("detail", "hello world, too");
      ten_env.return_result(std::move(cmd_result), std::move(cmd));
    }
  }
};

TEN_CPP_REGISTER_ADDON_AS_EXTENSION(
    http_server_extension_two_extensions__test_extension, test_extension);

class test_app : public ten::app_t {
 public:
  void on_init(ten::ten_env_t &ten_env) final {
    ten::ten_env_internal_accessor_t ten_env_internal_accessor(&ten_env);
    bool rc = ten_env_internal_accessor.init_manifest_from_json(
        // clang-format off
                 R"({
                      "type": "app",
                      "name": "test_app",
                      "version": "0.1.0"
                    })"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    rc = ten_env.init_property_from_json(
        // clang-format off
                 R"({
                      "_ten": {
                        "log_level": 1,
                        "predefined_graphs": [{
                          "name": "0",
                          "auto_start": true,
                          "nodes": [{
                            "type": "extension_group",
                            "name": "test extension group",
                            "addon": "default_extension_group"
                          },{
                            "type": "extension",
                            "name": "simple_http_server_cpp",
                            "addon": "simple_http_server_cpp",
                            "extension_group": "test extension group"
                          },{
                            "type": "extension",
                            "name": "test extension",
                            "addon": "http_server_extension_two_extensions__test_extension",
                            "extension_group": "test extension group"
                          }],
                          "connections": [{
                            "extension_group": "test extension group",
                            "extension": "simple_http_server_cpp",
                            "cmd": [{
                              "name": "hello_world",
                              "dest": [{
                                "extension_group": "test extension group",
                                "extension": "test extension"
                              }]
                            }]
                          }]
                        }]
                      }
                    })"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    ten_env.on_init_done();
  }
};

ten::app_t *test_app_ = nullptr;

void *test_app_thread_main(TEN_UNUSED void *args) {
  test_app_ = new test_app();
  test_app_->run(true);
  test_app_->wait();
  delete test_app_;
  test_app_ = nullptr;

  return nullptr;
}

}  // namespace

TEST(ExtensionTest, HttpServerExtensionTwoExtensions) {  // NOLINT
  auto *app_thread =
      ten_thread_create("app thread", test_app_thread_main, nullptr);

  ten_test_http_client_init();

  ten_string_t resp;
  ten_string_init(&resp);
  ten_test_http_client_post("http://127.0.0.1:8001/",
                            R"({"_ten": {"name": "hello_world"}})", &resp);
  EXPECT_EQ(std::string(ten_string_get_raw_str(&resp)), "\"hello world, too\"");

  ten_string_deinit(&resp);
  ten_test_http_client_deinit();

  if (test_app_ != nullptr) {
    test_app_->close();
  }

  ten_thread_join(app_thread, -1);
}
