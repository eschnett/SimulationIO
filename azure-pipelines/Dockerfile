# How to build this Docker image:
#     docker build . --tag eschnett/simulationio
#     docker push eschnett/simulationio

# Use Ubuntu
FROM ubuntu:19.10

RUN mkdir /work
WORKDIR /work

# Install system packages
RUN apt-get update && \
    apt-get --yes --no-install-recommends install \
        build-essential \
        ca-certificates \
        cmake \
        g++ \
        libgtest-dev \
        libpython3-dev \
        libyaml-cpp-dev \
        pkg-config \
        python3 \
        python3-h5py \
        python3-numpy \
        swig \
        wget \
        && \
    rm -rf /var/lib/apt/lists/*

# Install asdf-cxx
RUN mkdir src && \
    ( cd src && \
    wget https://github.com/eschnett/asdf-cxx/archive/version/7.2.1.tar.gz && \
    tar xzf 7.2.1.tar.gz && \
    cd asdf-cxx-version-7.2.1 && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/work/asdf-cxx-7.2.1 .. && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true ) && \
    rm -rf src

# As documentation
COPY Dockerfile /Dockerfile
