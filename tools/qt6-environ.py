# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import argparse
import os
import platform
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

DEFAULT_QT_VERSION = "6.8.3"

TOOLS_CHECK = {"moc", "rcc", "uic"}
COMPONENTS_CHECK = {"Core", "Widgets", "Gui", "Svg"}


@dataclass
class Checker:
    tool: str
    app: str
    cmake: str
    dyn: str
    static: str | None
    debug: str | None
    apps: set[str] = field(default_factory=set)

    @staticmethod
    def __check_one(
        installation: Path,
        template: str,
        version_major: str,
        kind: str,
        name: str,
    ):
        filename = template.format(version_major=version_major, name=name)
        if not (installation / filename).exists():
            print(
                f"{kind} {name} not found in {installation} ({(installation / filename)} missing)"
            )
            return False
        print(f"Found {Path(filename)}")
        return True

    @staticmethod
    def __check_list(
        installation: Path,
        template: str,
        version_major: str,
        kind: str,
        names: Iterable[str],
    ):
        for name in names:
            if not Checker.__check_one(
                installation, template, version_major, kind, name
            ):
                return False
        return True

    def check(self, installation: Path, version_major: str) -> bool:
        if not installation.exists():
            print(f"Directory {installation} does not exist")
            return False

        if not Checker.__check_list(
            installation, self.tool, version_major, "Tool", TOOLS_CHECK
        ):
            return False

        if not Checker.__check_list(
            installation, self.app, version_major, "Application", self.apps
        ):
            return False

        for template in (self.cmake, self.dyn, self.static, self.debug):
            if template is None:
                continue
            if not Checker.__check_list(
                installation, template, version_major, "Component", COMPONENTS_CHECK
            ):
                return False

        print(f"Installation {installation} looks good")
        return True


WINDOWS_CHECKER = Checker(
    tool="bin/{name}.exe",
    app="bin/{name}.exe",
    cmake="lib/cmake/Qt{version_major}{name}/Qt{version_major}{name}Config.cmake",
    dyn="bin/Qt{version_major}{name}.dll",
    static="lib/Qt{version_major}{name}.lib",
    debug="lib/Qt{version_major}{name}d.lib",
    apps={"qtpaths", "windeployqt"},
)

LINUX_CHECKER = Checker(
    tool="libexec/{name}",
    app="bin/{name}",
    cmake="lib/cmake/Qt{version_major}{name}/Qt{version_major}{name}Config.cmake",
    dyn="lib/libQt{version_major}{name}.so",
    static=None,
    debug=None,
    apps={"qtpaths"},
)


def parse_args():
    default_host: str | None = None
    if sys.platform.startswith("win32"):
        default_host = (
            "windows_arm64" if platform.machine().upper() == "ARM64" else "windows"
        )
    elif sys.platform.startswith("darwin"):
        default_host = "mac"
    else:
        default_host = (
            "linux_arm64" if platform.machine().upper() == "ARM64" else "linux"
        )

    parser = argparse.ArgumentParser(
        description="Downloads and installs Qt libraries",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--host",
        help="platform name of the Qt to download",
        choices=["windows", "windows_arm64", "mac", "linux", "linux_arm64", "all_os"],
        default=default_host,
        required=False,
    )

    parser.add_argument(
        "--version",
        help="version of the Qt to download",
        default=DEFAULT_QT_VERSION,
        required=False,
    )

    parser.add_argument(
        "--dest",
        help="destination directory for the Qt to download",
        type=Path,
        default=Path(__file__).parent.parent / "build" / "qt",
        required=False,
    )

    parser.add_argument(
        "--no-shell",
        help="do not stay in a shell at the end of installation",
        action="store_true",
        default=False,
    )

    args = parser.parse_args()
    args.target = "desktop"
    args.shell = not args.no_shell

    if args.host == "windows":
        args.arch = "win64_msvc2022_64"
    elif args.host == "windows_arm64":
        args.arch = "win64_msvc2022_arm64"
    else:
        args.arch = "linux_gcc_64"

    if args.arch.startswith("win64_"):
        args.subdir = args.arch[len("win64_") :]
    elif args.arch.startswith("linux_"):
        args.subdir = args.arch[len("linux_") :]
    else:
        args.subdir = args.arch

    return args


def install(host: str, target: str, version: str, arch: str, dest: Path):
    args = [
        "aqt",
        "install-qt",
        host,
        target,
        version,
        arch,
        "-O",
        dest.as_posix(),
    ]
    print(" ".join(shlex.quote(arg) for arg in args))
    exec_path = shutil.which(args[0])
    if not exec_path:
        print(f"Cannot find executable {args[0]} in PATH. Exiting.", file=sys.stderr)
        package = {"aqt": "aqtinstall"}.get(args[0])
        if package:
            print(
                f"Install package with\n\n    pip install {package}\n", file=sys.stderr
            )
        sys.exit(1)
    args[0] = exec_path
    result = subprocess.run(args)
    if result.returncode != 0:
        sys.exit(1)


if __name__ == "__main__":
    args = parse_args()
    installation = args.dest / args.version / args.subdir
    checker = WINDOWS_CHECKER if args.host == "windows" else LINUX_CHECKER
    version_major = args.version.split(".", 1)[0]
    if not checker.check(installation, version_major):
        args.dest.mkdir(parents=True, exist_ok=True)
        install(args.host, args.target, args.version, args.arch, args.dest)

        if not checker.check(installation, version_major):
            print(f"Installation of Qt {args.version} failed", file=sys.stderr)
            sys.exit(1)

    Qt_DIR = (installation / "lib" / "cmake").as_posix()

    if os.environ.get("GITHUB_ENV"):
        with Path(os.environ.get("GITHUB_ENV", "")).open("a") as GITHUB_ENV:
            print(f"Qt{version_major}_DIR={Qt_DIR}", file=GITHUB_ENV)

    if args.shell:
        os.environ[f"Qt{version_major}_DIR"] = Qt_DIR
        pwsh = shutil.which("pwsh")
        shell = os.environ.get("SHELL") or pwsh or os.environ.get("COMSPEC")
        if shell:
            print(
                "CMake environment variables have been set up. Call\n\n    ./flow config --both\n\nin this new shell:"
            )
            subprocess.run([shell], check=True)
