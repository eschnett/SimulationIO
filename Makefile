CXX = g++
CPPFLAGS = $(GTEST_CPPFLAGS) $(HDF5_CPPFLAGS) $(MPI_CPPFLAGS) $(PYTHON_CPPFLAGS)
CXXFLAGS = $(GTEST_CXXFLAGS) $(HDF5_CXXFLAGS) $(MPI_CXXFLAGS) $(PYTHON_CXXFLAGS) -g -Wall -std=c++0x -fPIC # -O2 -march=native
# On Travis Darwin:
# [-O2 -march=native]: times out
# []: works
LDFLAGS = $(GTEST_LDFLAGS) $(HDF5_LDFLAGS) $(MPI_LDFLAGS)
LIBS = $(GTEST_LIBS) $(HDF5_LIBS) $(MPI_LIBS)

ifneq ($(COVERAGE),)
CXXFLAGS += --coverage
endif

RC_SRCS =
SIO_SRCS =					\
	Basis.cpp				\
	BasisVector.cpp				\
	Configuration.cpp			\
	CoordinateField.cpp			\
	CoordinateSystem.cpp			\
	DiscreteField.cpp			\
	DiscreteFieldBlock.cpp			\
	DiscreteFieldBlockComponent.cpp		\
	Discretization.cpp			\
	DiscretizationBlock.cpp			\
	Field.cpp				\
	Manifold.cpp				\
	Parameter.cpp				\
	ParameterValue.cpp			\
	Project.cpp				\
	SubDiscretization.cpp			\
	TangentSpace.cpp			\
	TensorComponent.cpp			\
	TensorType.cpp
ALL_SRCS =					\
	$(SIO_SRCS)				\
	$(RC_SRCS)				\
	benchmark.cpp				\
	convert-carpet-output.cpp		\
	copy.cpp				\
	example.cpp				\
	list.cpp				\
	test_RegionCalculus.cpp			\
	test_SimulationIO.cpp
PYTHON_EXE = _H5.so _RegionCalculus.so _SimulationIO.so
ALL_EXE =							\
	$(PYTHON_EXE)						\
	benchmark convert-carpet-output copy list example	\
	test_RegionCalculus test_SimulationIO

HDF5_DIR = /opt/local
HDF5_CPPFLAGS = -I$(HDF5_DIR)/include
HDF5_CXXFLAGS =
HDF5_LDFLAGS = -L$(HDF5_DIR)/lib -Wl,-rpath,$(HDF5_DIR)/lib
HDF5_LIBS = -lhdf5_cpp -lhdf5

MPI_DIR = /opt/local
MPI_CPPFLAGS = -I$(MPI_DIR)/include/openmpi-gcc6
MPI_CXXFLAGS =
MPI_LDFLAGS = -L$(MPI_DIR)/lib/openmpi-gcc6 -Wl,-rpath,$(MPI_DIR)/lib/openmpi-gcc6
MPI_LIBS = -lmpi_cxx -lmpi

PYTHON_DIR = /opt/local/Library/Frameworks/Python.framework/Versions/2.7
PYTHON_CPPFLAGS = -I$(PYTHON_DIR)/include/python2.7
PYTHON_CXXFLAGS =
PYTHON_LDFLAGS = -L$(PYTHON_DIR)/lib -Wl,-rpath,$(PYTHON_DIR)/lib
PYTHON_LIBS = -lpython2.7

GTEST_DIR = googletest-release-1.7.0
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
check: test_RegionCalculus test_SimulationIO
	./test_RegionCalculus
	./test_SimulationIO

benchmark: $(SIO_SRCS:%.cpp=%.o) benchmark.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

copy: $(SIO_SRCS:%.cpp=%.o) copy.o
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
	swig -v -Wall -Wextra -c++ -python $*.i
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
PROCESS_DEPENDENCIES =							  \
  {									  \
    perl -p -e 's{$*.o.tmp}{$*.o}g' < $*.o.d &&				  \
    perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}' < $*.o.d; \
  } > $*.d &&								  \
  $(RM) $*.o.d
-include $(ALL_SRCS:%.cpp=%.d)

coverage:
	-lcov --directory . --capture --output-file coverage.info
	-lcov --remove coverage.info '/googletest-*' '/hdf5-*' '/usr/*' '/opt/*' '/Xcode.app/*' '*_wrap.cpp' --output-file coverage.info
	-lcov --list coverage.info

format:
	find *.hpp *.cpp | grep -v _wrap.cpp | xargs -n 8 clang-format -i

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
