# =============================================================================
# multipart multi-stage Dockerfile
# =============================================================================
# Stages:
#   deps  - Dependencies (logger)
#   build - Library compilation and installation
#
# Usage:
#   docker build --target deps  -t multipart_builder .
#   docker build --target build -t multipart .
#   docker build -t multipart .  (default: build)
# =============================================================================

FROM ubuntu:24.04 AS deps
LABEL maintainer="testillano"
LABEL testillano.multipart_builder.description="Docker image with all dependencies to build ert_multipart library"

WORKDIR /code/build

# ---------------------------------------------------------------------------
# Dependency versions (single source of truth)
# ---------------------------------------------------------------------------
ARG make_procs=4
ARG build_type=Release
ARG ert_logger_ver=v1.1.1

# ---------------------------------------------------------------------------
# System packages
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    wget tar \
    make cmake g++ \
    doxygen graphviz \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# ===========================================================================
# ERT_LOGGER
# ===========================================================================
RUN set -x && \
    wget https://github.com/testillano/logger/archive/${ert_logger_ver}.tar.gz && \
    tar xvf ${ert_logger_ver}.tar.gz && cd logger-*/ && \
    cmake -DERT_LOGGER_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf * && \
    set +x

# ---------------------------------------------------------------------------
# Builder entrypoint
# ---------------------------------------------------------------------------
COPY deps/build.sh /var/build.sh
RUN chmod a+x /var/build.sh

ENTRYPOINT ["/var/build.sh"]
CMD []

# =============================================================================
# Stage: build (compile and install ert_multipart)
# =============================================================================
FROM deps AS build

ARG make_procs=4
ARG build_type=Release

COPY . /code/build/multipart/
RUN set -x && \
    cd multipart && \
    cmake -DERT_MULTIPART_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf multipart && \
    set +x
