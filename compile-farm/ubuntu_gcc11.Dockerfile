FROM ubuntu:24.04

LABEL project="JPipe <> Compile Farm"
LABEL os="ubuntu-24.04-noble"
LABEL compiler="gcc-11"

ENV DEBIAN_FRONTEND=noninteractive
ENV CC=gcc-11
ENV CXX=g++-11

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-11 \
    g++-11 \
    make \
    cmake \
    ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-11

WORKDIR /compiler

CMD ["bash"]
