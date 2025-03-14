#
# This file is part of the TEN Framework project.
# Licensed under the Apache License, Version 2.0.
# See the LICENSE file for more information.
#
from ten import App, TenEnv


class DefaultApp(App):

    def on_init(self, ten_env: TenEnv):
        print("app on_init")
        ten_env.on_init_done()

    def on_deinit(self, ten_env: TenEnv) -> None:
        print("app on_deinit")
        ten_env.on_deinit_done()


if __name__ == "__main__":

    app = DefaultApp()
    print("app created.")

    app.run(False)
    print("app run completed.")
