CXX = g++
CPPFLAGS =
CXXFLAGS = -g -Wall -std=c++0x
LDFLAGS =

HDF5_LIBS = -lhdf5_cpp -lhdf5

GTEST_DIR = googletest-release-1.7.0
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_CXXFLAGS = -pthread

all: selftest

gtest:
	$(RM) $@
	wget -O $(GTEST_DIR).tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
	$(RM) -r $(GTEST_DIR)
	tar xzf $(GTEST_DIR).tar.gz
	:> $@
gtest-all.o: gtest
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

selftest.o: selftest.cpp SimulationIO.hpp
SimulatinoIO.o: SimulationIO.cpp SimulationIO.hpp
selftest: SimulationIO.cpp selftest.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) $(LDFLAGS) -o $@ $^ $(HDF5_LIBS)
test: selftest
	./selftest

%.o: %.cpp gtest
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c $*.cpp

clean:
	$(RM) gtest gtest-all.o $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) SimulationIO.o
	$(RM) selftest selftest.o

.PHONY: all test clean
