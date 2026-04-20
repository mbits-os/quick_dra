# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import os
import shutil
import subprocess
import sys
from pathlib import Path

if len(sys.argv) < 2:
    print("python tools/ubuntu_alternative.py <compiler> [<version>]", file=sys.stderr)
    raise SystemExit(1)


def __get_compiler(name: str):
    for suite in __compilers:
        for compiler in suite:
            if compiler == name or name.startswith(f"{compiler}-"):
                return suite
    return None


def __gather_execs():
    result = set[str]()
    PATH = (os.environ.get("PATH") or os.path.defpath).split(os.path.pathsep)
    for pathname in PATH:
        for root, dirs, files in Path(pathname).walk():
            dirs[:] = []
            for filename in files:
                try:
                    path = root / filename
                    mode = path.stat().st_mode
                    mode = mode & 0o777
                    if mode & 0o111 == 0:
                        continue
                    result.add(path.name)
                except FileNotFoundError:
                    # This can result from e.g. broken symlink
                    pass

    return result


def __highest_known(prefix: str):
    if not prefix.endswith("-"):
        prefix += "-"

    version = -1
    for name in filter(lambda name: name.startswith(prefix), __gather_execs()):
        suffix = name[len(prefix) :]
        try:
            value = int(suffix)
        except ValueError:
            continue
        if value > version:
            version = value

    return version if version >= 0 else None


def __get_suite(name: str):
    suite = __get_compiler(name)
    if not suite:
        known_compilers = [name for suite in __compilers for name in suite]
        if len(known_compilers) > 1:
            known = ", ".join(known_compilers[:-1])
            known = f"{known} and {known_compilers[-1]}"
        else:
            known = ", ".join(known_compilers)
        print(
            f"error: unknown compiler: {name}; known compilers are: {known}",
            file=sys.stderr,
        )
        raise SystemExit(1)

    version: int | None = None

    if len(sys.argv) > 2:
        try:
            version = int(sys.argv[2])
        except ValueError:
            print(f"error: version is not an int: {sys.argv[2]}", file=sys.stderr)
            raise SystemExit(1)

    if version is None:
        if "-" in __compiler:
            try:
                version = int(__compiler.split("-", 1)[1])
            except ValueError:
                print(f"error: version is not an int: {__compiler}", file=sys.stderr)
                raise SystemExit(1)

    if version is None:
        version = __highest_known(suite[0])

    if version is None:
        print(f"error: cannot find version for {__compiler}", file=sys.stderr)
        raise SystemExit(1)

    print(f"setting alternatives for {suite[0]}-{version}", file=sys.stderr)

    return suite, version


def __get_alternative(name: str):
    proc = subprocess.run(
        ["update-alternatives", "--display", name], shell=False, capture_output=True
    )

    if proc.returncode:
        return None

    deps: dict[str, str] = {}
    installs: list[str] = []
    link: str | None = None
    lines = proc.stdout.decode().rstrip().split("\n")
    in_main = True

    link_prefix = f"  link {name} is "
    for line in lines:
        if line.startswith("/"):
            in_main = False
            installs.append(line.split(" - ")[0])
        if not in_main:
            continue
        if line.startswith("  slave "):
            line = line[len("  slave ") :]
            dep, dep_link = line.split(" is ", 1)
            deps[dep] = dep_link
        elif line.startswith(link_prefix):
            link = line.rstrip()[len(link_prefix) :]
    return link, deps, installs


def __suite_setup(suite: list[str], setup: dict[str, str]):
    mapping: dict[str, tuple[str | None, dict[str, str], list[str]]] = {}
    for app in suite:
        alts = __get_alternative(app)
        if alts:
            mapping[app] = alts

    for app in suite:
        if app not in mapping:
            is_slave = False
            for _, slaves, _ in mapping.values():
                if app in slaves:
                    is_slave = True
                    break
            if not is_slave:
                print(f"error: cannot find alternative for {app}", file=sys.stderr)
                raise SystemExit(1)

    script: list[list[str]] = []

    for key, decl_ in sorted(mapping.items()):
        link, deps, installed = decl_
        try:
            need = setup[key]
        except KeyError:
            continue

        suffix = f"/{need}"
        available = list(filter(lambda path: path.endswith(suffix), installed))
        if available:
            alternative = available[0]
        elif not need.startswith(f"{key}-") and need not in mapping:
            suffix = f"/{need}-{__version}"
            available = list(filter(lambda path: path.endswith(suffix), installed))
            if available:
                alternative = available[0]
            else:
                if link is None:
                    print(
                        f"error: cannot install new alternative: no link for {key}",
                        file=sys.stderr,
                    )
                    raise SystemExit(1)
                alternative = shutil.which(f"{need}-{__version}")
                if alternative is None:
                    print(
                        f"error: cannot find binary for: {need}-{__version}",
                        file=sys.stderr,
                    )
                    raise SystemExit(1)

                script.append(
                    [
                        "--install",
                        link,
                        key,
                        alternative,
                        str(__version * 10),
                    ]
                )
        else:
            if link is None:
                print(
                    f"error: cannot install new alternative: no link for {key}",
                    file=sys.stderr,
                )
                raise SystemExit(1)
            alternative = shutil.which(need)
            if alternative is None:
                print(f"error: cannot find binary for: {need}", file=sys.stderr)
                raise SystemExit(1)

            install = ["--install", link, key, alternative, str(__version * 10)]
            for dep in deps:
                if dep in suite:
                    dep_alt = shutil.which(setup[dep])
                    if dep_alt is None:
                        print(
                            f"error: cannot find binary for: {setup[dep]}",
                            file=sys.stderr,
                        )
                        raise SystemExit(1)
                    install.extend(["--slave", deps[dep], dep, dep_alt])
            script.append(install)

        script.append(["--set", key, alternative])

    return script


__compiler = sys.argv[1]

__compilers = [
    ["g++", "gcc"],
    ["clang++", "clang"],
]

__base_suite = ["c++", "cc"]

__suite, __version = __get_suite(__compiler)

__script = __suite_setup(__suite, {name: f"{name}-{__version}" for name in __suite})
__script.extend(
    __suite_setup(__base_suite, {base: app for base, app in zip(__base_suite, __suite)})
)

try:
    for command in __script:
        cmd = ["update-alternatives", *command]
        print(*cmd)
        proc = subprocess.run(cmd, shell=False, check=False)
        if proc.returncode:
            raise SystemExit(1)
except subprocess.CalledProcessError:
    raise SystemExit(1)
