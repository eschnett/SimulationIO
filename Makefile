CXX = gcc
CPPFLAGS =
CXXFLAGS = -g -Wall -std=c++0x
LDFLAGS =

HDF5_LIBS = -lhdf5_cpp -lhdf5

all: selftest
selftest: selftest.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(HDF5_LIBS)
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cpp
test: selftest
	./selftest
clean:
	$(RM) selftest.o selftest
.PHONY: all test clean
