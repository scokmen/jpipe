FROM ubuntu:24.04

LABEL project="JPipe <> Compile Farm"
LABEL os="ubuntu-24.04-noble"
LABEL compiler="gcc-12"

ENV DEBIAN_FRONTEND=noninteractive
ENV CC=gcc-12
ENV CXX=g++-12

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-12 \
    g++-12 \
    make \
    cmake \
    ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 120 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-12

WORKDIR /compiler

CMD ["bash"]
