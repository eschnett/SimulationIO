CXX = g++
CPPFLAGS = $(GTEST_CPPFLAGS) $(HDF5_CPPFLAGS) $(MPI_CPPFLAGS) $(PYTHON_CPPFLAGS)
CXXFLAGS = $(GTEST_CXXFLAGS) $(HDF5_CXXFLAGS) $(MPI_CXXFLAGS) $(PYTHON_CXXFLAGS) -g -Wall -std=c++0x -fPIC -march=native # -O2
LDFLAGS = $(GTEST_LDFLAGS) $(HDF5_LDFLAGS) $(MPI_LDFLAGS)
LIBS = $(GTEST_LIBS) $(HDF5_LIBS) $(MPI_LIBS)

ifneq ($(COVERAGE),)
CXXFLAGS += --coverage
endif

os = $(shell uname)
ifeq ($(os), Linux)
make-dynamiclib = -shared
dynamiclib-suffix = so
else ifeq ($(os), Darwin)
make-dynamiclib = -dynamiclib
dynamiclib-suffix = dylib
else
make-dynamiclib = -shared
dynamiclib-suffix = so
endif

RC_SRCS =
SIO_SRCS =					\
	Basis.cpp				\
	BasisVector.cpp				\
	Configuration.cpp			\
	CoordinateField.cpp			\
	CoordinateSystem.cpp			\
	DataBlock.cpp				\
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
	merge.cpp				\
	test_RegionCalculus.cpp			\
	test_SimulationIO.cpp
SWIG_SRCS = H5.i RegionCalculus.i SimulationIO.i
PYTHON_EXE = $(SWIG_SRCS:%.i=_%.so)
ALL_EXE =							\
	libSimulationIO.a libSimulationIO.$(dynamiclib-suffix)	\
	$(PYTHON_EXE)						\
	test_RegionCalculus test_SimulationIO			\
	sio-benchmark						\
	sio-copy sio-example sio-list sio-merge			\
	sio-convert-carpet-output

HDF5_DIR = /opt/local
HDF5_CPPFLAGS = -I$(HDF5_DIR)/include
HDF5_CXXFLAGS =
HDF5_LDFLAGS = -L$(HDF5_DIR)/lib -Wl,-rpath,$(HDF5_DIR)/lib
HDF5_LIBS = -lhdf5_cpp -lhdf5

MPI_DIR = /opt/local
MPI_CPPFLAGS = -I$(MPI_DIR)/include/openmpi-gcc6
MPI_CXXFLAGS =
MPI_LDFLAGS = -L$(MPI_DIR)/lib/openmpi-gcc6 -Wl,-rpath,$(MPI_DIR)/lib/openmpi-gcc6
MPI_LIBS = -lmpi

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
	curl -kLv -o $(GTEST_DIR).tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
	$(RM) -r $(GTEST_DIR)
	tar xzf $(GTEST_DIR).tar.gz
	:> $@
gtest-all.o: gtest
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

test_RegionCalculus.o: gtest
test_SimulationIO.o: gtest
test_RegionCalculus: test_RegionCalculus.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
test_SimulationIO: test_SimulationIO.o gtest-all.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
check: $(ALL_EXE)
	./test_RegionCalculus
	./test_SimulationIO
	./sio-example
	./sio-list example.s5
	./sio-copy example.s5 example2.s5
	./sio-list example2.s5
	./python-example.py
	./python-read.py
	-$(HDF5_DIR)/bin/h5format_convert python-example.s5
	./python-read-direct.py
#	./julia-example.jl
#	./julia-read.jl
	echo SUCCESS

sio-benchmark: benchmark.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-copy: copy.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-example: example.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-list: list.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-merge: merge.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

sio-convert-carpet-output: convert-carpet-output.o libSimulationIO.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

libSimulationIO.a: $(SIO_SRCS:%.cpp=%.o) $(RC_SRCS:%.cpp=%.o)
	$(AR) -rc $@ $^
	if [[ -n '$(RANLIB)' ]]; then $(RANLIB) $@; fi
libSimulationIO.$(dynamiclib-suffix): $(SIO_SRCS:%.cpp=%.o) $(RC_SRCS:%.cpp=%.o)
	$(CXX) $(make-dynamiclib) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

_%.so: %_wrap.o $(SIO_SRCS:%.cpp=%.o)
	$(CXX) $(make-dynamiclib) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(PYTHON_LDFLAGS) -o $@ $^ $(LIBS) $(PYTHON_LIBS)

%_wrap.cpp %.py: %.i
	$(RM) $*_wrap.cpp
	swig -MD -v -Wall -Wextra -c++ -python $*.i
	mv $*_wrap.d $*_wrap.cpp.d.tmp
	{								   \
	  perl -p -e 's{$*_wrap.cxx}{$*_wrap.cpp}g' < $*_wrap.cpp.d.tmp && \
	  perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}'	   \
	    < $*_wrap.cpp.d.tmp;					   \
	} > $*_wrap.cpp.d
	$(RM) $*_wrap.cpp.d.tmp
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
PROCESS_DEPENDENCIES =							      \
  mv $*.o.d $*.o.d.tmp &&						      \
  {									      \
    perl -p -e 's{$*.o.tmp}{$*.o}g' < $*.o.d.tmp &&			      \
    perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}' < $*.o.d.tmp; \
  } > $*.o.d &&								      \
  $(RM) $*.o.d.tmp
-include $(ALL_SRCS:%.cpp=%.o.d)
-include $(SWIG_SRCS:%.i=%_wrap.cpp.d) $(SWIG_SRCS:%.i=%_wrap.o.d)

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
	$(RM) -- $(ALL_SRCS:%.cpp=%.o) $(ALL_SRCS:%.cpp=%.o.d)
	$(RM) -- $(SWIG_SRCS:%.i=%_wrap.cxx) $(SWIG_SRCS:%.i=%_wrap.cpp)
	$(RM) -- $(SWIG_SRCS:%.i=%_wrap.cpp.d)
	$(RM) -- $(SWIG_SRCS:%.i=%_wrap.o.d) $(SWIG_SRCS:%.i=%_wrap.o)
	$(RM) -- $(SWIG_SRCS:%.i=_%.so)
	$(RM) -- $(SWIG_SRCS:%.i=%.py) $(SWIG_SRCS:%.i=%.pyc)
	$(RM) -- libSimulationIO.a libSimulationIO.$(dynamiclib-suffix)
	$(RM) -- $(ALL_EXE)
	$(RM) -- example.s5 example2.s5 python-example.s5 julia-example.s5

distclean: clean
	$(RM) $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) gtest

.PHONY: all check coverage format clean distclean
