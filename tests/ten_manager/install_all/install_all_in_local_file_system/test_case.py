#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
import os
import sys
from .common import cmd_exec


def get_installed_extensions_count(app_dir: str):
    extension_dir = os.path.join(app_dir, "ten_packages/extension/")
    extensions = os.listdir(extension_dir)

    return len(extensions)


def test_tman_dependency_resolve():
    base_path = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.join(base_path, "../../../../")

    if sys.platform == "win32":
        os.environ["PATH"] = (
            os.path.join(root_dir, "ten_manager/lib") + ";" + os.getenv("PATH")
        )
        tman_bin = os.path.join(root_dir, "ten_manager/bin/tman.exe")
    else:
        tman_bin = os.path.join(root_dir, "ten_manager/bin/tman")

    app_dir = os.path.join(base_path, "test_app")

    config_file = os.path.join(
        root_dir,
        "tests/local_registry/config.json",
    )

    # The extensions in the dependencies of this app are not in the mock server
    # (i.e., they are not published). Execute 'install_all' in the app, all the
    # dependencies should be installed.
    #
    # We also add 'ext_1' which is in the mock server in the dependencies of
    # this app, to check if tman works well.
    returncode, output_text = cmd_exec.run_cmd_realtime(
        [
            tman_bin,
            f"--config-file={config_file}",
            "install",
        ],
        cwd=app_dir,
    )
    if returncode != 0:
        print(output_text)
        assert 0

    assert get_installed_extensions_count(app_dir) == 3


if __name__ == "__main__":
    test_tman_dependency_resolve()
