CXX = g++
CPPFLAGS = -I/opt/local/include
CXXFLAGS = -g -Wall -std=c++0x
LDFLAGS = -L/opt/local/lib -Wl,-rpath,/opt/local/lib

ifneq ($(COVERAGE),)
CXXFLAGS += --coverage
endif

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

selftest.o: selftest.cpp SimulationIO.hpp gtest
SimulatinoIO.o: SimulationIO.cpp SimulationIO.hpp
selftest: SimulationIO.cpp selftest.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) $(LDFLAGS) -o $@ $^ $(HDF5_LIBS)
test: selftest
	./selftest

coverage:
	lcov --directory . --capture --output-file coverage.info
	lcov --remove coverage.info '/googletest-*' '/usr/*' '/opt/*' --output-file coverage.info
	lcov --list coverage.info

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c $*.cpp

clean:
	$(RM) -r *.dSYM
	$(RM) *.gcda *.gcno coverage.info
	$(RM) gtest gtest-all.o $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) *.o
	$(RM) selftest

.PHONY: all test coverage clean
