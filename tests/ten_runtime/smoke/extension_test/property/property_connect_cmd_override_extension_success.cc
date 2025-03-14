//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "include_internal/ten_runtime/binding/cpp/ten.h"
#include "ten_utils/lang/cpp/lib/value.h"
#include "ten_utils/lib/thread.h"
#include "ten_utils/macro/macros.h"
#include "tests/common/client/cpp/msgpack_tcp.h"
#include "tests/ten_runtime/smoke/extension_test/util/binding/cpp/check.h"

#define PROP_NAME "test_prop"
#define PROP_OLD_VAL 62422
#define PROP_NEW_VAL 892734

namespace {

class test_extension : public ten::extension_t {
 public:
  explicit test_extension(const std::string &name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    nlohmann::json json = nlohmann::json::parse(cmd->to_json());
    if (json["_ten"]["name"] == "hello_world") {
      auto prop_value = ten_env.get_property_int64(PROP_NAME);
      if (prop_value == PROP_NEW_VAL) {
        auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK);
        cmd_result->set_property("detail", "hello world, too");
        ten_env.return_result(std::move(cmd_result), std::move(cmd));
      }
    }
  }
};

class test_extension_group : public ten::extension_group_t {
 public:
  explicit test_extension_group(const std::string &name)
      : ten::extension_group_t(name) {}

  void on_create_extensions(ten::ten_env_t &ten_env) override {
    ten_env.addon_create_extension_async(
        "property_start_graph_cmd_override_extension_success__extension",
        "test extension",
        [](ten::ten_env_t &ten_env, ten::extension_t &extension) {
          std::vector<ten::extension_t *> extensions;
          extensions.push_back(&extension);
          ten_env.on_create_extensions_done(extensions);
        });
  }

  void on_destroy_extensions(
      ten::ten_env_t &ten_env,
      const std::vector<ten::extension_t *> &extensions) override {
    for (auto iter = extensions.begin(); iter != extensions.end(); ++iter) {
      ten_env.addon_destroy_extension_async(*iter, [](ten::ten_env_t &ten_env) {
        ten_env.on_destroy_extensions_done();
      });
    }
  }
};

class test_app : public ten::app_t {
 public:
  void on_init(ten::ten_env_t &ten_env) override {
    bool rc = ten_env.init_property_from_json(
        // clang-format off
                 "{\
                     \"_ten\": {\
                       \"uri\": \"msgpack://127.0.0.1:8001/\"\
                     }\
                   }"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    ten_env.on_init_done();
  }
};

void *test_app_thread_main(TEN_UNUSED void *args) {
  auto *app = new test_app();
  app->run();
  delete app;

  return nullptr;
}

TEN_CPP_REGISTER_ADDON_AS_EXTENSION(
    property_start_graph_cmd_override_extension_success__extension,
    test_extension);

TEN_CPP_REGISTER_ADDON_AS_EXTENSION_GROUP(
    property_start_graph_cmd_override_extension_success__extension_group,
    test_extension_group);

}  // namespace

TEST(ExtensionTest, PropertyConnectCmdOverrideExtensionSuccess) {  // NOLINT
  // Start app.
  auto *app_thread =
      ten_thread_create("app thread", test_app_thread_main, nullptr);

  // Create a client and connect to the app.
  auto *client = new ten::msgpack_tcp_client_t("msgpack://127.0.0.1:8001/");

  // Send graph.
  nlohmann::json command =
      R"({
           "_ten": {
             "type": "start_graph",
             "seq_id": "55",
             "nodes": [{
               "type": "extension_group",
               "name": "property_start_graph_cmd_override_extension_success__extension_group",
               "addon": "property_start_graph_cmd_override_extension_success__extension_group",
               "app": "msgpack://127.0.0.1:8001/"
             },{
               "type": "extension",
               "name": "test extension",
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "property_start_graph_cmd_override_extension_success__extension_group",
               "property": {}
             }]
           }
         })"_json;
  command["_ten"]["nodes"][1]["property"]["test_prop"] = PROP_NEW_VAL;

  nlohmann::json resp = client->send_json_and_recv_resp_in_json(command);
  ten_test::check_status_code_is(resp, TEN_STATUS_CODE_OK);

  // Send a user-defined 'hello world' command.
  resp = client->send_json_and_recv_resp_in_json(
      R"({
           "_ten": {
             "name": "hello_world",
             "seq_id": "137",
             "dest":[{
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "property_start_graph_cmd_override_extension_success__extension_group",
               "extension": "test extension"
             }]
           }
         })"_json);
  ten_test::check_result_is(resp, "137", TEN_STATUS_CODE_OK,
                            "hello world, too");

  delete client;

  ten_thread_join(app_thread, -1);
}
