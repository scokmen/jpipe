FROM ubuntu:24.04

LABEL project="JPipe <> Compile Farm"
LABEL os="ubuntu-24.04-noble"
LABEL compiler="gcc-10"

ENV DEBIAN_FRONTEND=noninteractive
ENV CC=gcc-10
ENV CXX=g++-10

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-10 \
    g++-10 \
    make \
    cmake \
    ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-10

WORKDIR /compiler

CMD ["bash"]