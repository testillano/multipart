# C++ multipart wrapper library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation](https://codedocs.xyz/testillano/multipart.svg)](https://codedocs.xyz/testillano/multipart/index.html)
[![Ask Me Anything !](https://img.shields.io/badge/Ask%20me-anything-1abc9c.svg)](https://github.com/testillano)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/testillano/multipart/graphs/commit-activity)
[![Main project workflow](https://github.com/testillano/multipart/actions/workflows/ci.yml/badge.svg)](https://github.com/testillano/multipart/actions/workflows/ci.yml)
[![Container](https://img.shields.io/badge/Container-ghcr.io-blue.svg)](https://github.com/testillano/multipart/pkgs/container/multipart)

Multipart content parser library based on @iafonov multipart-parser-c (https://github.com/iafonov/multipart-parser-c). Used by [h2agent](https://github.com/testillano/h2agent) for multipart request/response handling.

## Build with Docker

Single multi-stage `Dockerfile` with all dependencies from `ubuntu:24.04`.

```bash
$ ./build.sh                              # build everything
$ ./build.sh --builder                    # deps stage only
$ DBUILD_XTRA_OPTS=--no-cache ./build.sh  # force rebuild
```

Or directly:

```bash
$ docker build -t multipart .
```

### Pulling pre-built images

```bash
$ docker pull ghcr.io/testillano/multipart:<tag>
```

## Build natively

```bash
$ cmake . && make -j$(nproc)
```

### Requirements

All dependencies are documented in the `Dockerfile` (ARG declarations + RUN steps).

### Install

```bash
$ sudo make install
```

### Documentation

```bash
$ make doc
```

## Integration

### CMake

#### FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(ert_multipart
  GIT_REPOSITORY https://github.com/testillano/multipart.git
  GIT_TAG vx.y.z)

FetchContent_GetProperties(ert_multipart)
if(NOT ert_multipart_POPULATED)
  FetchContent_Populate(ert_multipart)
  add_subdirectory(${ert_multipart_SOURCE_DIR} ${ert_multipart_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(foo PRIVATE ert_multipart::ert_multipart)
```

## Contributing

```bash
$ sources=$(find . -name "*.hpp" -o -name "*.cpp")
$ docker run -i --rm -v $PWD:/data frankwolf/astyle ${sources}
```
