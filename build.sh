#!/bin/bash

#############
# VARIABLES #
#############
image_tag__dflt=latest
base_tag__dflt=latest
make_procs__dflt=$(grep processor /proc/cpuinfo -c)
build_type__dflt=Release
ert_logger_ver__dflt=v1.1.0
registry=ghcr.io/testillano

#############
# FUNCTIONS #
#############
usage() {
  cat << EOF

  Usage: $0 [--builder-image|--project|--project-image|--auto]

         --builder-image: builds base image from './Dockerfile.build'.
         --project:       builds the project library using builder image.
         --project-image: builds project image from './Dockerfile'.
         --auto:          builds everything using defaults.

         Environment variables:

         For headless mode you may prepend or export asked/environment variables for the corresponding
         docker procedure:

         --builder-image: image_tag, base_tag (alpine), make_procs, build_type, ert_logger_ver
         --project:       make_procs, build_type, base_tag (multipart_builder)
         --project-image: image_tag, base_tag (multipart_builder), make_procs, build_type
         --auto:          any of the variables above

         Other prepend variables:

         DBUILD_XTRA_OPTS: extra options to docker build.

         Examples:

         build_type=Debug $0 --builder-image
         image_tag=test1 $0 --auto
         DBUILD_XTRA_OPTS=--no-cache $0 --auto

EOF
}

# $1: variable by reference
_read() {
  local -n varname=$1

  local default=$(eval echo \$$1__dflt)
  local s_default="<null>"
  [ -n "${default}" ] && s_default="${default}"
  echo "Input '$1' value [${s_default}]:"

  if [ -n "${varname}" ]
  then
    echo "${varname}"
  else
    read -r varname
    [ -z "${varname}" ] && varname=${default}
  fi
}

build_builder_image() {
  echo
  echo "=== Build multipart_builder image ==="
  echo
  _read image_tag
  _read base_tag
  _read make_procs
  _read build_type
  _read ert_logger_ver

  bargs="--build-arg base_tag=${base_tag}"
  bargs+=" --build-arg make_procs=${make_procs}"
  bargs+=" --build-arg build_type=${build_type}"
  bargs+=" --build-arg ert_logger_ver=${ert_logger_ver}"

  set -x
  rm -f CMakeCache.txt
  # shellcheck disable=SC2086
  docker build --rm ${DBUILD_XTRA_OPTS} ${bargs} -f Dockerfile.build -t ${registry}/multipart_builder:"${image_tag}" . || return 1
  set +x
}

build_project() {
  echo
  echo "=== Format source code ==="
  echo
  sources=$(find . -name "*.hpp" -o -name "*.cpp")
  docker run -i --rm -v $PWD:/data frankwolf/astyle ${sources}

  echo
  echo "=== Build multipart project ==="
  echo
  _read base_tag
  _read make_procs
  _read build_type

  envs="-e MAKE_PROCS=${make_procs} -e BUILD_TYPE=${build_type}"

  set -x
  rm -f CMakeCache.txt
  # shellcheck disable=SC2086
  docker run --rm -it -u "$(id -u):$(id -g)" ${envs} -v "${PWD}":/code -w /code ${registry}/multipart_builder:"${base_tag}" || return 1
  # shellcheck disable=SC2086
  docker run --rm -it -u "$(id -u):$(id -g)" ${envs} -v "${PWD}":/code -w /code ${registry}/multipart_builder:"${base_tag}" "" doc || return 1
  set +x
}

build_project_image() {
  echo
  echo "=== Build multipart image ==="
  echo
  _read image_tag
  _read base_tag
  _read make_procs
  _read build_type

  bargs="--build-arg base_tag=${base_tag}"
  bargs+=" --build-arg make_procs=${make_procs}"
  bargs+=" --build-arg build_type=${build_type}"

  set -x
  rm -f CMakeCache.txt
  # shellcheck disable=SC2086
  docker build --rm ${DBUILD_XTRA_OPTS} ${bargs} -t ${registry}/multipart:"${image_tag}" . || return 1
  set +x
}

build_auto() {
  # export defaults to automate, but allow possible environment values:
  # shellcheck disable=SC1090
  source <(grep -E '^[0a-z_]+__dflt' "$0" | sed 's/^/export /' | sed 's/__dflt//' | sed -e 's/\([0a-z_]*\)=\(.*\)/\1=\${\1:-\2}/')
  build_builder_image && build_project && build_project_image
}

#############
# EXECUTION #
#############
# shellcheck disable=SC2164
cd "$(dirname "$0")"

case "$1" in
  --builder-image) build_builder_image ;;
  --project) build_project ;;
  --project-image) build_project_image ;;
  --auto) build_auto ;;
  *) usage && exit 1 ;;
esac

exit $?

