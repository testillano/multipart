ARG base_tag=latest
FROM ghcr.io/testillano/multipart_builder:${base_tag}
MAINTAINER testillano

LABEL testillano.multipart.description="ert_multipart library image"

WORKDIR /code/build

ARG make_procs=4
ARG build_type=Release

# ert_multipart
COPY . /code/build/multipart/
RUN set -x && \
    cd multipart && cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs} && make install && \
    cd .. && rm -rf multipart && \
    set +x
