CPPFLAGS = $(GTEST_CPPFLAGS) $(HDF5_CPPFLAGS) $(MPI_CPPFLAGS) $(PYTHON_CPPFLAGS)
CXXFLAGS = $(GTEST_CXXFLAGS) $(HDF5_CXXFLAGS) $(MPI_CXXFLAGS) $(PYTHON_CXXFLAGS) -g -Wall -std=c++0x -fPIC
LDFLAGS = $(GTEST_LDFLAGS) $(HDF5_LDFLAGS) $(MPI_LDFLAGS)
LIBS = $(GTEST_LIBS) $(HDF5_LIBS) $(MPI_LIBS)

DEFAULT_CXX = g++
DEFAULT_SWIG = swig
DEFAULT_MPI_DIR = /usr/
DEFAULT_HDF5_DIR = /usr/local/hdf5
DEFAULT_PYTHON_DIR = /usr
DEFAULT_PYTHON_VERSION=2.7
DEFAULT_MPI_NAME = mpich
DEFAULT_MPI_LIBS= -lmpichcxx -lmpich

CXX ?= $(DEFAULT_CXX)
SWIG ?= $(DEFAULT_SWIG)

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
	$(PYTHON_EXE) \
	benchmark convert-carpet-output list example \
	test_RegionCalculus test_SimulationIO

HDF5_DIR ?= $(DEFAULT_HDF5_DIR)
HDF5_CPPFLAGS ?= -I$(HDF5_DIR)/include
HDF5_CXXFLAGS ?=
HDF5_LDFLAGS ?= -L$(HDF5_DIR)/lib -Wl,-rpath,$(HDF5_DIR)/lib
HDF5_LIBS ?= -lhdf5_cpp -lhdf5

MPI_NAME ?= $(DEFAULT_MPI_NAME)
MPI_DIR ?= $(DEFAULT_MPI_DIR)
MPI_CPPFLAGS ?= -I$(MPI_DIR)/include/$(MPI_NAME)
MPI_CXXFLAGS ?=
MPI_LDFLAGS ?= -L$(MPI_DIR)/lib/x86_64-gnu -Wl,-rpath,$(MPI_DIR)/lib/x86_64-linux-gnu
MPI_LIBS ?= $(DEFAULT_MPI_LIBS)

PYTHON_VERSION ?= $(DEFAULT_PYTHON_VERSION)
PYTHON_NAME ?= python${PYTHON_VERSION}
PYTHON_DIR ?= $(DEFAULT_PYTHON_DIR)
PYTHON_CPPFLAGS ?= -I$(PYTHON_DIR)/include/${PYTHON_NAME} -I$(PYTHON_DIR)/include/x86_64-linux-gnu/${PYTHON_NAME}
PYTHON_CXXFLAGS ?=
PYTHON_LDFLAGS ?= -L$(PYTHON_DIR)/lib/x86_64-linux-gnu -Wl,-rpath,$(PYTHON_DIR)/lib/x86_64-linux-gnu
PYTHON_LIBS ?= -lpython${PYTHON_VERSION}

GTEST_VERSION = release-1.7.0
GTEST_DIR = googletest-${GTEST_VERSION}
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_CXXFLAGS = -pthread
GTEST_LIBS =

all: $(ALL_EXE)

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

benchmark: $(SIO_SRCS:%.cpp=%.o) benchmark.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

example: $(SIO_SRCS:%.cpp=%.o) example.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

list: $(SIO_SRCS:%.cpp=%.o) list.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

convert-carpet-output: $(SIO_SRCS:%.cpp=%.o) convert-carpet-output.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

os = $(shell uname)
ifeq ($(os), Linux)
make-dynamiclib = -shared
else ifeq ($(os), Darwin)
make-dynamiclib = -dynamiclib
else
make-dynamiclib = -shared
endif
_%.so: %_wrap.o $(SIO_SRCS:%.cpp=%.o)
	$(CXX) $(make-dynamiclib) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(PYTHON_LDFLAGS) -o $@ $^ $(LIBS) $(PYTHON_LIBS)

%_wrap.cpp: %.i
	$(SWIG) -Wall -c++ -python $*.i
	mv $*_wrap.cxx $*_wrap.cpp
.PRECIOUS: $(PYTHON_EXE:_%.so=%_wrap.cpp)
.PRECIOUS: $(PYTHON_EXE:_%.so=%_wrap.o)

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

clean:
	$(RM) -r *.dSYM
	$(RM) *.gcda *.gcno coverage.info
	$(RM) gtest-all.o
	$(RM) -- $(ALL_SRCS:%.cpp=%.o) $(ALL_SRCS:%.cpp=%.d)
	$(RM) -- $(PYTHON_EXE:_%.so=%_wrap.cxx) $(PYTHON_EXE:_%.so=%_wrap.cpp)
	$(RM) -- $(PYTHON_EXE:_%.so=%_wrap.d) $(PYTHON_EXE:_%.so=%_wrap.o)
	$(RM) -- $(PYTHON_EXE:_%.so=%.py) $(PYTHON_EXE:_%.so=%.pyc)
	$(RM) -- $(ALL_EXE)

distclean: clean
	$(RM) $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) gtest

.PHONY: all test coverage clean distclean
