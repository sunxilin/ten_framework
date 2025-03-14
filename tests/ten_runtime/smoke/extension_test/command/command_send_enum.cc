//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include <nlohmann/json.hpp>
#include <string>

#include "gtest/gtest.h"
#include "include_internal/ten_runtime/binding/cpp/ten.h"
#include "ten_utils/lib/thread.h"
#include "ten_utils/macro/check.h"
#include "tests/common/client/cpp/msgpack_tcp.h"
#include "tests/ten_runtime/smoke/extension_test/util/binding/cpp/check.h"

namespace {

enum TEST_ENUM { TEST_1, TEST_2 };

class test_extension_1 : public ten::extension_t {
 public:
  explicit test_extension_1(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    nlohmann::json json = nlohmann::json::parse(cmd->to_json());

    if (json["_ten"]["name"] == "hello_world") {
      hello_world_cmd = std::move(cmd);

      auto new_cmd = ten::cmd_t::create("send enum");
      new_cmd->set_property("test data", TEST_1);

      ten_env.send_cmd(
          std::move(new_cmd),
          [this](ten::ten_env_t &ten_env,
                 std::unique_ptr<ten::cmd_result_t> status) {
            nlohmann::json json = nlohmann::json::parse(status->to_json());

            auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
            cmd_result->set_property("detail", json.value("detail", ""));
            ten_env.return_result(std::move(cmd_result),
                                  std::move(hello_world_cmd));
          });

      return;
    }
  }

 private:
  std::unique_ptr<ten::cmd_t> hello_world_cmd;
};

class test_extension_2 : public ten::extension_t {
 public:
  explicit test_extension_2(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    if (std::string(cmd->get_name()) == "send enum") {
      auto u_test_data = cmd->get_property_int32("test data");
      auto test_data = static_cast<TEST_ENUM>(u_test_data);
      if (test_data != TEST_1) {
        TEN_ASSERT(0, "Should not happen.");
      }

      auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
      cmd_result->set_property("detail", "hello world, too");
      ten_env.return_result(std::move(cmd_result), std::move(cmd));
    }
  }
};

class test_app : public ten::app_t {
 public:
  void on_init(ten::ten_env_t &ten_env) override {
    bool rc = ten_env.init_property_from_json(
        // clang-format off
                 R"({
                      "_ten": {
                        "uri": "msgpack://127.0.0.1:8001/",
                        "log_level": 1
                      }
                    })"
        // clang-format on
        ,
        nullptr);
    ASSERT_EQ(rc, true);

    ten_env.on_init_done();
  }
};

void *test_app_thread_main(TEN_UNUSED void *arg) {
  auto *app = new test_app();
  app->run();
  delete app;

  return nullptr;
}

TEN_CPP_REGISTER_ADDON_AS_EXTENSION(command_send_enum__extension_1,
                                    test_extension_1);
TEN_CPP_REGISTER_ADDON_AS_EXTENSION(command_send_enum__extension_2,
                                    test_extension_2);

}  // namespace

TEST(ExtensionTest, CommandSendEnum) {  // NOLINT
  // Start app.
  auto *app_thread =
      ten_thread_create("app thread", test_app_thread_main, nullptr);

  // Create a client and connect to the app.
  auto *client = new ten::msgpack_tcp_client_t("msgpack://127.0.0.1:8001/");

  // Send graph.
  nlohmann::json resp = client->send_json_and_recv_resp_in_json(
      R"({
           "_ten": {
             "type": "start_graph",
             "seq_id": "55",
             "nodes": [{
               "type": "extension_group",
               "name": "command_send_enum__extension_group_1",
               "addon": "default_extension_group",
               "app": "msgpack://127.0.0.1:8001/"
             },{
               "type": "extension_group",
               "name": "command_send_enum__extension_group_2",
               "addon": "default_extension_group",
               "app": "msgpack://127.0.0.1:8001/"
             },{
               "type": "extension",
               "name": "command_send_enum__extension_1",
               "addon": "command_send_enum__extension_1",
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "command_send_enum__extension_group_1"
             },{
               "type": "extension",
               "name": "command_send_enum__extension_2",
               "addon": "command_send_enum__extension_2",
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "command_send_enum__extension_group_2"
             }],
             "connections": [{
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "command_send_enum__extension_group_1",
               "extension": "command_send_enum__extension_1",
               "cmd": [{
                 "name": "send enum",
                 "dest": [{
                   "app": "msgpack://127.0.0.1:8001/",
                   "extension_group": "command_send_enum__extension_group_2",
                   "extension": "command_send_enum__extension_2"
                 }]
               }]
             }]
           }
         })"_json);
  ten_test::check_status_code_is(resp, TEN_STATUS_CODE_OK);

  // Send a user-defined 'hello world' command.
  resp = client->send_json_and_recv_resp_in_json(
      R"({
           "_ten": {
             "name": "hello_world",
             "seq_id": "137",
             "dest": [{
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "command_send_enum__extension_group_1",
               "extension": "command_send_enum__extension_1"
             }]
           }
         })"_json);
  ten_test::check_result_is(resp, "137", TEN_STATUS_CODE_OK,
                            "hello world, too");

  delete client;

  ten_thread_join(app_thread, -1);
}
