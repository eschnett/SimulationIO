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
        libbz2-dev \
        libgtest-dev \
        libpython3-dev \
        libssl-dev \
        libyaml-cpp-dev \
        pkg-config \
        python3 \
        python3-h5py \
        python3-numpy \
        swig \
        wget \
        zlib1g-dev \
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
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true ) && \
    rm -rf src

# Install silo
RUN mkdir src && \
    ( cd src && \
    wget https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/silo-4.10.2/silo-4.10.2.tar.gz && \
    tar xzf silo-4.10.2.tar.gz && \
    cd silo-4.10.2 && \
    mkdir build && \
    cd build && \
    ../configure && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true ) && \
    rm -rf src

# As documentation
COPY Dockerfile /Dockerfile
