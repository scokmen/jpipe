FROM debian:bullseye-slim

LABEL project="JPipe <> Compile Farm"
LABEL os="debian-11-buster"
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
    && rm -rf /var/lib/apt/lists/*
    
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-11

WORKDIR /compiler

CMD ["bash"]
