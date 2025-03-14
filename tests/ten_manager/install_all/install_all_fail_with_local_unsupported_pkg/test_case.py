#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
import os
import sys
from .common import cmd_exec


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

    returncode, output_text = cmd_exec.run_cmd_realtime(
        [tman_bin, f"--config-file={config_file}", "install", "--os=linux"],
        cwd=app_dir,
    )

    assert returncode != 0
    # As the local package 'heiheihei_1' is not compatible with linux os, and
    # the package cannot be found in the registry, so no candidates can be found
    # and the installation fails.
    assert output_text.find("Failed to find candidates for extension") != -1


if __name__ == "__main__":
    test_tman_dependency_resolve()
