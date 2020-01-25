# docker build . --tag eschnett/simulationio
# docker push eschnett/simulationio

FROM ubuntu:19.10

ENV DEBIAN_FRONTEND=noninteractive 

RUN mkdir /sio
WORKDIR /sio

# Install system packages
RUN apt-get update && \
    apt-get --yes --no-install-recommends install \
        build-essential \
        ca-certificates \
        cmake \
        git \
        libbz2-dev \
        libpython-dev \
        libssl-dev \
        libyaml-cpp-dev \
        openssl \
        pkg-config \
        python-h5py \
        python-numpy \
        swig \
        wget \
        zlib1g-dev \
        && \
    rm -rf /var/lib/apt/lists/*

# Install asdf-cxx
RUN ( \
        mkdir -p src/asdf-cxx-7.0.0 && \
        cd src/asdf-cxx-7.0.0 && \
        wget https://github.com/eschnett/asdf-cxx/archive/version/7.0.0.tar.gz && \
        tar xzf 7.0.0.tar.gz && \
        cd asdf-cxx-version-7.0.0 && \
        mkdir build && \
        cd build && \
        cmake \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_INSTALL_PREFIX=/sio/asdf-cxx-7.0.0 \
            .. && \
        make -j$(nproc) && \
        make -j$(nproc) install && \
    true ) && \
    rm -rf src/asdf-cxx-7.0.0

# Install HDF5
RUN ( \
        mkdir -p src/hdf5-1.10.6 && \
        cd src/hdf5-1.10.6 && \
        wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/hdf5-1.10.6.tar.gz && \
        tar xzf hdf5-1.10.6.tar.gz && \
        cd hdf5-1.10.6 && \
        mkdir build && \
        cd build && \
        ../configure --enable-cxx --enable-hl --prefix=/sio/hdf5-1.10.6 && \
        make -j$(nproc) && \
        make -j$(nproc) install && \
    true ) && \
    rm -rf src/hdf5-1.10.6

# Install Silo
RUN ( \
        mkdir -p src/silo-4.10.2-bsd && \
        cd src/silo-4.10.2-bsd && \
        wget https://wci.llnl.gov/content/assets/docs/simulation/computer-codes/silo/silo-4.10.2/silo-4.10.2-bsd.tar.gz && \
        tar xzf silo-4.10.2-bsd.tar.gz && \
        cd silo-4.10.2-bsd && \
        mkdir build && \
        cd build && \
        ../configure --disable-fortran --enable-optimization --with-hdf5=/sio/hdf5-1.10.6/include,/sio/hdf5-1.10.6/lib --prefix=/sio/silo-4.10.2-bsd && \
        make -j$(nproc) && \
        make -j$(nproc) install && \
    true ) && \
    rm -rf src/silo-4.10.2-bsd

# Install SimulationIO
RUN export PATH="/sio/asdf-cxx-7.0.0/bin:/sio/hdf5-1.10.6/bin:/sio/silo-4.10.2-bsd/bin:$PATH" && \
    mkdir -p src/simulationio-git && \
    cd src/simulationio-git && \
    git clone -b eschnett/silo https://github.com/eschnett/SimulationIO.git && \
    cd SimulationIO && \
    mkdir build && \
    cd build && \
    cmake \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DENABLE_ASDF_CXX=ON \
        -DENABLE_HDF5=ON \
        -DENABLE_MPI=OFF \
        -DENABLE_RNPL=OFF \
        -DENABLE_SILO=ON \
        -DENABLE_TILEDB=OFF \
        -DCMAKE_PREFIX_PATH='/sio/asdf-cxx-7.0.0;/sio/hdf5-1.10.6;/sio/silo-4.10.2-bsd' \
        -DCMAKE_INSTALL_PREFIX=/sio/simulationio-git \
        .. && \
    make -j$(nproc) && \
    make -j$(nproc) test CTEST_OUTPUT_ON_FAILURE=1 && \
    make -j$(nproc) install
