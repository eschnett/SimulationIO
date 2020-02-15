# How to build this Docker image:
#     docker build . --tag eschnett/simulationio-deps
#     docker push eschnett/simulationio-deps

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

ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"

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

# Install HDF5
# Note: Ubuntu's HDF5 package is missing h5format_convert
RUN mkdir src && \
    (cd src && \
    wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/hdf5-1.10.6.tar.gz && \
    tar xzf hdf5-1.10.6.tar.gz && \
    cd hdf5-1.10.6 && \
    mkdir build && \
    cd build && \
    ../configure --enable-cxx --enable-hl --prefix=/usr/local && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true) && \
    rm -rf src

# Install Silo (requires HDF5)
RUN mkdir src && \
    (cd src && \
    wget https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/silo-4.10.2/silo-4.10.2-bsd.tar.gz && \
    tar xzf silo-4.10.2-bsd.tar.gz && \
    cd silo-4.10.2-bsd && \
    mkdir build && \
    cd build && \
    ../configure --disable-fortran --enable-optimization --with-hdf5=/usr/local/include,/usr/local/lib --prefix=/usr/local && \
    make -j$(nproc) && \
    make -j$(nproc) install && \
    true) && \
    rm -rf src

# As documentation
COPY Dockerfile /Dockerfile
