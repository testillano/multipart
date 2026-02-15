#!/bin/bash
# =============================================================================
# multipart build script
# =============================================================================
# Non-interactive. Reads versions from Dockerfile.
#
# Usage:
#   ./build.sh                              # build everything
#   ./build.sh --builder                    # deps stage only
#   DBUILD_XTRA_OPTS=--no-cache ./build.sh  # force rebuild
# =============================================================================

set -e

SCR="$(readlink -f "$0")"
cd "$(dirname "${SCR}")"

DOCKERFILE=Dockerfile
registry=ghcr.io/testillano

parse_arg() { grep "^ARG ${1}=" "${DOCKERFILE}" | head -1 | cut -d= -f2; }
resolve() {
  local val="${!1}"
  [ -z "${val}" ] && val="$(parse_arg "$1")"
  [ -z "${val}" ] && val="$(eval echo \$${1}__dflt)"
  echo "${val}"
}

image_tag__dflt=latest
make_procs__dflt=$(nproc)

build_bargs() {
  local bargs=""
  bargs+=" --build-arg make_procs=$(resolve make_procs)"
  bargs+=" --build-arg build_type=$(resolve build_type)"
  bargs+=" --build-arg ert_logger_ver=$(resolve ert_logger_ver)"
  echo "${bargs}"
}

build_builder() {
  echo
  echo "=== Build multipart_builder (deps stage) ==="
  echo
  local tag=$(resolve image_tag)
  local bargs=$(build_bargs)
  set -x
  # shellcheck disable=SC2086
  docker build --rm ${DBUILD_XTRA_OPTS} ${bargs} --target deps \
    -t ${registry}/multipart_builder:"${tag}" . || return 1
  set +x
}

build_image() {
  echo
  echo "=== Build multipart image (deps + library) ==="
  echo
  local tag=$(resolve image_tag)
  local bargs=$(build_bargs)
  set -x
  # shellcheck disable=SC2086
  docker build --rm ${DBUILD_XTRA_OPTS} ${bargs} --target build \
    -t ${registry}/multipart:"${tag}" . || return 1
  set +x
}

case "${1:-}" in
  --builder) build_builder ;;
  --image) build_image ;;
  -h|--help) echo "Usage: $0 [--builder|--image]"; echo "  (no args): builds everything." ;;
  "") build_image ;;
  *) echo "Unknown option: $1" && exit 1 ;;
esac

exit $?
