# How to build this Docker image:
#     docker build . --build-arg commit=<commit>

FROM eschnett/simulationio-deps

WORKDIR /work

ARG commit

RUN git clone -n https://github.com/eschnett/SimulationIO.git && \
    cd SimulationIO && \
    git checkout "${commit}"

WORKDIR /work/SimulationIO

RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_MPI=OFF -DENABLE_RNPL=OFF -DENABLE_TILEDB=OFF -DCMAKE_INSTALL_PREFIX=/work/SimulationIO ..

WORKDIR /work/SimulationIO/build

RUN make -j$(nproc)

RUN make -j$(nproc) test CTEST_OUTPUT_ON_FAILURE=1

RUN make -j$(nproc) install
