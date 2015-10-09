CXX = g++
CPPFLAGS =
CXXFLAGS = -g -Wall -std=c++0x
LDFLAGS =

SRCS = SimulationIO.cpp selftest.cpp

HDF5_LIBS = -lhdf5_cpp -lhdf5

GTEST_DIR = googletest-release-1.7.0
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_CXXFLAGS = -pthread

# Taken from <http://mad-scientist.net/make/autodep.html> as written by Paul D.
# Smith <psmith@gnu.org>, originally developed by Tom Tromey <tromey@cygnus.com>
PROCESS_DEPENDENCIES = \
  { \
  	perl -p -e 's{$*.o.tmp}{$*.o}g' < $*.o.d && \
  	perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}' < $*.o.d; \
  } > $*.d && \
  $(RM) $*.o.d
-include $(SRCS:%.cpp=%.d)

all: selftest

gtest:
	$(RM) $@
	wget -O $(GTEST_DIR).tar.gz https://github.com/google/googletest/archive/release-1.7.0.tar.gz
	$(RM) -r $(GTEST_DIR)
	tar xzf $(GTEST_DIR).tar.gz
	:> $@
gtest-all.o: gtest
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

selftest.o: gtest
selftest: SimulationIO.o selftest.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) $(LDFLAGS) -o $@ $^ $(HDF5_LIBS)
test: selftest
	./selftest

%.o: %.cpp
	@$(RM) $*.o
	$(CXX) -MD $(CPPFLAGS) $(GTEST_CPPFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $*.o.tmp $*.cpp
	@$(PROCESS_DEPENDENCIES)
	@mv $*.o.tmp $*.o

clean:
	$(RM) gtest gtest-all.o $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) $(SRCS:%.cpp=%.o) $(SRCS:%.cpp=%.d)
	$(RM) selftest

.PHONY: all test clean
