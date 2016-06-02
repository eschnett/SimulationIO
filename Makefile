CPPFLAGS = $(GTEST_CPPFLAGS) $(HDF5_CPPFLAGS) $(MPI_CPPFLAGS)
CXXFLAGS = $(GTEST_CXXFLAGS) $(HDF5_CXXFLAGS) $(MPI_CXXFLAGS) $(CXX_FLAGS_BASE)
LDFLAGS = $(GTEST_LDFLAGS) $(HDF5_LDFLAGS) $(MPI_LDFLAGS)
LIBS = $(GTEST_LIBS) $(HDF5_LIBS) $(MPI_LIBS)
CXX_FLAGS_BASE = -Wall -std=c++0x -fPIC

DEFAULT_CXX = g++
DEFAULT_MPI_DIR = /usr/
DEFAULT_HDF5_DIR = /usr/local/hdf5
DEFAULT_MPI_NAME = mpich
DEFAULT_MPI_LIBS= -lmpichcxx -lmpich
DEFAULT_PREFIX = /usr/local
DEFAULT_BIN_DIR = bin

CXX ?= $(DEFAULT_CXX)
PY_PACKAGE_DIR=pysimulationio

ifneq ($(COVERAGE),)
CXXFLAGS += --coverage
endif

RC_SRCS =
SIO_SRCS = \
	Basis.cpp \
	BasisVector.cpp \
	Configuration.cpp \
	CoordinateField.cpp \
	CoordinateSystem.cpp \
	DiscreteField.cpp \
	DiscreteFieldBlock.cpp \
	DiscreteFieldBlockComponent.cpp \
	Discretization.cpp \
	DiscretizationBlock.cpp \
	Field.cpp \
	Manifold.cpp \
	Parameter.cpp \
	ParameterValue.cpp \
	Project.cpp \
	SubDiscretization.cpp \
	TangentSpace.cpp \
	TensorComponent.cpp \
	TensorType.cpp
ALL_SRCS = \
	$(SIO_SRCS) \
	$(RC_SRCS) \
	benchmark.cpp \
	convert-carpet-output.cpp \
	example.cpp \
	list.cpp \
	test_RegionCalculus.cpp \
	test_SimulationIO.cpp
PYTHON_EXE = _H5.so _RegionCalculus.so _SimulationIO.so
ALL_EXE = \
	sio-benchmark sio-convert-carpet-output sio-list sio-example \
	test_RegionCalculus test_SimulationIO

HDF5_DIR ?= $(DEFAULT_HDF5_DIR)
HDF5_INCLUDE ?= $(HDF5_DIR)/include
HDF5_CPPFLAGS ?= -I$(HDF5_INCLUDE)
HDF5_CXXFLAGS ?=
HDF5_LINK ?= $(HDF5_DIR)/lib
HDF5_LDFLAGS ?= -L$(HDF5_LINK) -Wl,-rpath,$(HDF5_LINK)
HDF5_LIBS ?= -lhdf5_cpp -lhdf5

MPI_NAME ?= $(DEFAULT_MPI_NAME)
MPI_DIR ?= $(DEFAULT_MPI_DIR)
MPI_INCLUDE ?= $(MPI_DIR)/include/$(MPI_NAME)
MPI_CPPFLAGS ?= -I$(MPI_INCLUDE)
MPI_CXXFLAGS ?=
MPI_LINK ?= $(MPI_DIR)/lib/x86_64-gnu
MPI_LDFLAGS ?= -L$(MPI_LINK) -Wl,-rpath,$(MPI_LINK)
MPI_LIBS ?= $(DEFAULT_MPI_LIBS)
PREFIX ?= $(DEFAULT_PREFIX)
BIN_DIR ?= $(DEFAULT_BIN_DIR)

GTEST_VERSION = release-1.7.0
GTEST_DIR = googletest-${GTEST_VERSION}
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_CXXFLAGS = -pthread
GTEST_LIBS =

os = $(shell uname)
ifeq ($(os), Linux)
make-dynamiclib = -shared
else ifeq ($(os), Darwin)
make-dynamiclib = -dynamiclib
else
make-dynamiclib = -shared
endif

all: $(ALL_EXE) meta

meta: includes.txt links.txt libs.txt compiler.txt flags.txt lib_sources.txt

includes.txt: $(ALL_SRC)
	@echo $(HDF5_INCLUDE) > includes.txt
	@echo $(MPI_INCLUDE) >> includes.txt

links.txt: $(ALL_SRC)
	@echo $(HDF5_LINK) > links.txt
	@echo $(MPI_LINK) >> links.txt

libs.txt: $(ALL_SRC)
	@echo $(HDF5_LIBS) > libs.txt
	@echo $(MPI_LIBS) >> libs.txt

compiler.txt: $(ALL_SRC)
	@echo $(CXX) > compiler.txt

flags.txt: $(ALL_SRC)
	@echo $(CXX_FLAGS_BASE) > flags.txt

lib_sources.txt: $(ALL_SRC)
	@echo $(SIO_SRCS) > lib_sources.txt

gtest:
	$(RM) $@
	curl -Lv -o $(GTEST_DIR).tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
	$(RM) -r $(GTEST_DIR)
	tar xzf $(GTEST_DIR).tar.gz
	:> $@
gtest-all.o: gtest
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

test_RegionCalculus.o: gtest
test_SimulationIO.o: gtest
test_RegionCalculus: $(RC_SRCS:%.cpp=%.o) test_RegionCalculus.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
test_SimulationIO: $(SIO_SRCS:%.cpp=%.o) test_SimulationIO.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
test: test_RegionCalculus test_SimulationIO
	./test_RegionCalculus
	./test_SimulationIO

sio-benchmark: $(SIO_SRCS:%.cpp=%.o) benchmark.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-example: $(SIO_SRCS:%.cpp=%.o) example.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-list: $(SIO_SRCS:%.cpp=%.o) list.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-convert-carpet-output: $(SIO_SRCS:%.cpp=%.o) convert-carpet-output.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	@$(RM) $*.o
	$(CXX) -MD $(CPPFLAGS) $(CXXFLAGS) -c -o $*.o.tmp $*.cpp
	@$(PROCESS_DEPENDENCIES)
	@mv $*.o.tmp $*.o

# Taken from <http://mad-scientist.net/make/autodep.html> as written by Paul D.
# Smith <psmith@gnu.org>, originally developed by Tom Tromey <tromey@cygnus.com>
PROCESS_DEPENDENCIES = \
  { \
  	perl -p -e 's{$*.o.tmp}{$*.o}g' < $*.o.d && \
  	perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}' < $*.o.d; \
  } > $*.d && \
  $(RM) $*.o.d
-include $(ALL_SRCS:%.cpp=%.d)

coverage:
	-lcov --directory . --capture --output-file coverage.info
	-lcov --remove coverage.info '/googletest-*' '/hdf5-*' '/usr/*' '/opt/*' '/Xcode.app/*' '*_wrap.cpp' --output-file coverage.info
	-lcov --list coverage.info

install:
	@cp sio-example $(PREFIX)/$(BIN_DIR)
	@cp sio-list $(PREFIX)/$(BIN_DIR)
	@cp sio-convert-carpet-output $(PREFIX)/$(BIN_DIR)

clean:
	$(RM) -r *.dSYM
	$(RM) *.gcda *.gcno coverage.info
	$(RM) gtest-all.o
	$(RM) -- $(ALL_SRCS:%.cpp=%.o) $(ALL_SRCS:%.cpp=%.d)
	$(RM) -- $(PYTHON_EXE:_%.so=%_wrap.cxx) $(PYTHON_EXE:_%.so=%_wrap.cpp)
	$(RM) -- $(PYTHON_EXE:_%.so=%_wrap.d) $(PYTHON_EXE:_%.so=%_wrap.o)
	$(RM) -- $(PYTHON_EXE:_%.so=%.py) $(PYTHON_EXE:_%.so=%.pyc)
	$(RM) -- $(ALL_EXE)
	$(RM) -- $(PYTHON_EXE:_%.so=${PY_PACKAGE_DIR}/%.py)
	$(RM) -- $(PYTHON_EXE:_%.so=${PY_PACKAGE_DIR}/%.pyc)
	$(RM) -- $(PYTHON_EXE:%.so=${PY_PACKAGE_DIR}/%.so)
	$(RM) ${PY_PACKAGE_DIR}/__init__.pyc

distclean: clean
	$(RM) $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) gtest
	$(RM) example.s5
	$(RM) includes.txt
	$(RM) links.txt
	$(RM) libs.txt
	$(RM) compiler.txt
	$(RM) flags.txt
	$(RM) lib_sources.txt

.PHONY: all test coverage clean distclean meta install
