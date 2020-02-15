# How to build this Docker image:
#     docker build . --tag eschnett/simulationio
#     docker push eschnett/simulationio

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
        git \
        hdf5-tools \
        libbz2-dev \
        libgtest-dev \
        libhdf5-dev \
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
    (cd src && \
    wget https://github.com/eschnett/asdf-cxx/archive/version/7.2.1.tar.gz && \
    tar xzf 7.2.1.tar.gz && \
    cd asdf-cxx-version-7.2.1 && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true) && \
    rm -rf src

# Install silo
RUN mkdir src && \
    (cd src && \
    wget https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/silo-4.10.2/silo-4.10.2-bsd.tar.gz && \
    tar xzf silo-4.10.2-bsd.tar.gz && \
    cd silo-4.10.2-bsd && \
    mkdir build && \
    cd build && \
    ../configure --disable-fortran --enable-optimization --with-hdf5=/usr/lib/x86_64-linux-gnu/hdf5/serial/include,/usr/lib/x86_64-linux-gnu/hdf5/serial/lib --prefix=/usr/local && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true) && \
    rm -rf src

RUN apt-get update && \
    apt-get --yes --no-install-recommends install git

# As documentation
COPY Dockerfile /Dockerfile

ARG commit=

RUN if [ -n "${commit}" ]; then \
        mkdir src && \
        (cd src && \
            git clone -n https://github.com/eschnett/SimulationIO.git && \
            cd SimulationIO && \
            git checkout "${commit}" && \
            mkdir build && \
            cd build && \
            cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_MPI=OFF -DENABLE_RNPL=OFF -DENABLE_TILEDB=OFF -DCMAKE_INSTALL_PREFIX=/work/SimulationIO .. && \
            make -j$(nproc) && \
            make -j$(nproc) test CTEST_OUTPUT_ON_FAILURE=1 && \
            make -j$(nproc) install && \
        true) && \
        rm -rf src /work/SimulationIO; \
    fi
