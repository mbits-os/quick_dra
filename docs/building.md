# Working with code

## Prerequisites

To configure the project:

- [Python 3.12](https://www.python.org/) to run all the tools and scripts, including `./flow`.
- [Conan 2](https://conan.io/) to get all the dependencies.
- [CMake 3.28](https://cmake.org/) to configure the build and to pack the binaries.

To build the project:

- On Windows, Visual Studio 2022.
- On Linux, GCC 13 and [Ninja 1.11](https://ninja-build.org/).

## Flow

Easiest way to get the binaries is to use flows. They are used as part of GitHub workflow (and they even inform the GitHub, what actions are there to take).

```sh
./flow <command> <config-to-build>
```

### Commands

Currently, there are few specialized flows: `config`, `build`, `report` and `verify`, plus one general one, named simply `run`. Each performs a predefined set of steps, with `./flow run` being able to specify, which one to actually run. For a list of steps performed by each of the specialized comments, consult `.flow\config.yml` file.

## Building the code

Each of the commands below can substitute `--rel` parameter for `--dbg` to build a debug build.

### Configuration

Run the command below to recreate `build/conan` and `build/release` directories.

```sh
./flow config --rel
```

### Compilation

Run the command below to build the command line tool under `./build/release/bin/qdra`.

```sh
./flow build --rel
```

### Packing archives/installers

Run the command below to create the archives and installers in `./build/artifacts` directory.

```sh
./flow run --rel -s pack,store
```

### Installing

There is no `./flow` step, which would install the code, but CMake's `--install` should also do the trick:

```sh
sudo cmake --install ./build/release [--prefix <install-dir>]
```
