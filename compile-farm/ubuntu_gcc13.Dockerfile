FROM ubuntu:24.04

LABEL project="JPipe <> Compile Farm"
LABEL os="ubuntu-24.04-noble"
LABEL compiler="gcc-13"

ENV DEBIAN_FRONTEND=noninteractive
ENV CC=gcc-13
ENV CXX=g++-13

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-13 \
    g++-13 \
    make \
    cmake \
    ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 130 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-13

WORKDIR /compiler

CMD ["bash"]
