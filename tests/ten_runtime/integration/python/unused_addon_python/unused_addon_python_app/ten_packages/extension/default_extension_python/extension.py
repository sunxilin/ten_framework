#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
from ten import (
    Extension,
    TenEnv,
    Cmd,
    StatusCode,
    CmdResult,
)


class DefaultExtension(Extension):
    # The purpose of this case is to make sure the extension should not be
    # loaded if the extension is not declared in the graph.
    assert False

    def on_init(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_init")

        ten_env.init_property_from_json('{"testKey": "testValue"}')
        ten_env.on_init_done()

    def on_start(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_start")

        ten_env.set_property_from_json("testKey2", '"testValue2"')
        testValue = ten_env.get_property_to_json("testKey")
        testValue2 = ten_env.get_property_to_json("testKey2")
        print("testValue: ", testValue, " testValue2: ", testValue2)

        ten_env.on_start_done()

    def on_stop(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_stop")
        ten_env.on_stop_done()

    def on_deinit(self, ten_env: TenEnv) -> None:
        print("DefaultExtension on_deinit")
        ten_env.on_deinit_done()

    def check_greeting(
        self, ten_env: TenEnv, result: CmdResult, receivedCmd: Cmd
    ):
        statusCode = result.get_status_code()
        detail = result.get_property_string("detail")
        print(
            "DefaultExtension check_greeting: status:"
            + str(statusCode)
            + " detail:"
            + detail
        )

        respCmd = CmdResult.create(StatusCode.OK)
        respCmd.set_property_string("detail", detail + " nbnb")
        print("DefaultExtension create respCmd")

        ten_env.return_result(respCmd, receivedCmd)

    def check_hello(self, ten_env: TenEnv, result: CmdResult, receivedCmd: Cmd):
        statusCode = result.get_status_code()
        detail = result.get_property_string("detail")
        print(
            "DefaultExtension check_hello: status:"
            + str(statusCode)
            + " detail:"
            + detail
        )

        # Send a command to go extension.
        new_cmd = Cmd.create("greeting")
        ten_env.send_cmd(
            new_cmd,
            lambda ten, result: self.check_greeting(ten, result, receivedCmd),
        )

    def on_cmd(self, ten_env: TenEnv, cmd: Cmd) -> None:
        print("DefaultExtension on_cmd")

        cmd_json = cmd.to_json()
        print("DefaultExtension on_cmd json: " + cmd_json)

        new_cmd = Cmd.create("hello")
        new_cmd.set_property_from_json("test", '"testValue2"')
        test_value = new_cmd.get_property_to_json("test")
        print("DefaultExtension on_cmd test_value: " + test_value)

        # Send command to a cpp extension.
        ten_env.send_cmd(
            new_cmd, lambda ten, result: self.check_hello(ten, result, cmd)
        )
