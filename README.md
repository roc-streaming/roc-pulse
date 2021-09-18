# PulseAudio modules for Roc Toolkit

[![Build](https://github.com/roc-streaming/roc-pulse/workflows/build/badge.svg)](https://github.com/roc-streaming/roc-pulse/actions)

This repo is the new home for PulseAudio modules implementing Roc sender and receiver.

## Build instructions

### Simple build

When using this method, CMake will automatically download and build dependencies (Roc Toolkit, PulseAudio, libtool). Roc will be statically linked into the modules and there is no need to install it into the system.

First install build tools and PulseAudio:

```
sudo apt install -y \
    gcc g++ \
    make \
    libtool intltool m4 autoconf automake \
    cmake \
    scons \
    git \
    wget \
    python \
    pulseaudio
```

Then build PulseAudio modules:

```
make
```

And then install them into the system:

```
sudo make install
```

### Advanced build

You can disable automatic downloading of dependencies and build them manually.

First, download, build and install Roc Toolkit into the system as described on [this page](https://roc-streaming.org/toolkit/docs/building/user_cookbook.html).

Then download and unpack PulseAudio source code [from here](https://freedesktop.org/software/pulseaudio/releases/). There is no need to configure and build it, only source code is needed.

> PulseAudio doesn't provide official API for out-of-tree modules. This is the reason why we need full PulseAudio source code to build modules.

> Note: PulseAudio source code should have **exactly the same version** as the actual PulseAudio daemon which will be used to load modules.

Optionally, download and unpack [libtool](https://gnu.askapache.com/libtool/). As with PulseAudio, there is no need to build it. Alternatively, install it into the system using your package manager.

Then you can build and install modules:

```
mkdir build
cd build
cmake .. \
  -DDOWNLOAD_ROC=OFF \
  -DDOWNLOAD_PULSEAUDIO=OFF \
  -DDOWNLOAD_LIBTOOL=OFF \
  -DPULSEAUDIO_DIR=<...> \
  -DPULSEAUDIO_VERSION=<...>
make VERBOSE=1
sudo make install
```

Don't forget to specify correct `PULSEAUDIO_VERSION`!

If you've installed Roc Toolkit to non-standard directory, you can use `-DROC_INCLUDE_DIR=<...>` and `-DROC_LIB_DIR=<...>`. If you want to use custom libtool instead of the one from system, you can use `-DLIBTOOL_DIR=<...>`.

### Cross-compilation

You can use [standard instructions](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html) for cross-compiling using CMake.

For simple cases, it's also possible to do everything automaticaly by specifying just two options:

```
mkdir build
cd build
cmake .. -DHOST=<...> -DPULSEAUDIO_VERSION=<...>
make VERBOSE=1
ls -l ../bin
```

Commands above will cross-compile PulseAudio modules, as well as download and cross-compile their dependencies. Dependencies will be statically linked into modules.

Here, `HOST` defines toolchain triple of the target system, e.g. `aarch64-linux-gnu`. In this case `aarch64-linux-gnu-gcc` and other tools should be available in `PATH`.

## Authors

See [here](https://github.com/roc-streaming/roc-pulse/graphs/contributors).

## License

PulseAudio modules are licensed under [LGPL 2.1](LICENSE).

For details on Roc Toolkit licensing, see [here](https://roc-streaming.org/toolkit/docs/about_project/licensing.html).
