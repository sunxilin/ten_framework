#
# This file is part of the TEN Framework project.
# See https://github.com/TEN-framework/ten_framework/LICENSE for license
# information.
#
import argparse
import sys
import os
from build.scripts import cmd_exec


class ArgumentInfo(argparse.Namespace):
    def __init__(self):
        super().__init__()
        self.project_path: str
        self.manifest_path: str
        self.build_type: str
        self.target: str
        self.target_path: str
        self.env: list[str]
        self.log_level: int


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--project-path", type=str, required=True)
    parser.add_argument("--manifest-path", type=str, required=False)
    parser.add_argument("--build-type", type=str, required=True)
    parser.add_argument("--target", type=str, required=True)
    parser.add_argument("--target-path", type=str, required=True)
    parser.add_argument("--env", type=str, action="append", default=[])
    parser.add_argument("--log-level", type=int, required=True)

    arg_info = ArgumentInfo()
    args = parser.parse_args(namespace=arg_info)

    # Setup environment variables.
    for env in args.env:
        split_key_index = str(env).find("=")
        if split_key_index == -1:
            sys.exit(1)
        else:
            os.environ[(str(env)[:split_key_index])] = str(env)[
                split_key_index + len("=") :
            ]

    origin_wd = os.getcwd()

    try:

        os.chdir(args.project_path)

        cmd = [
            "cargo",
            "build",
            "--target-dir",
            args.target_path,
            "--target",
            args.target,
        ]

        if args.manifest_path != "":
            cmd.append(
                "--manifest-path",
            )
            cmd.append(
                args.manifest_path,
            )

        if args.build_type == "release":
            cmd.append("--release")

        returncode, _ = cmd_exec.run_cmd(cmd, args.log_level)
        if returncode:
            raise Exception("Failed to run cargo build.")

    except Exception as exc:
        print(exc)

    finally:
        os.chdir(origin_wd)
        sys.exit(-1 if returncode != 0 else 0)
