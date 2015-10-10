CXX = g++
CPPFLAGS = -I/opt/local/include
CXXFLAGS = -g -Wall -std=c++0x
LDFLAGS = -L/opt/local/lib -Wl,-rpath,/opt/local/lib

ifneq ($(COVERAGE),)
CXXFLAGS += --coverage
endif

SRCS = SimulationIO.cpp selftest.cpp

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

# Taken from <http://mad-scientist.net/make/autodep.html> as written by Paul D.
# Smith <psmith@gnu.org>, originally developed by Tom Tromey <tromey@cygnus.com>
PROCESS_DEPENDENCIES = \
  { \
  	perl -p -e 's{$*.o.tmp}{$*.o}g' < $*.o.d && \
  	perl -p -e 's{\#.*}{};s{^[^:]*: *}{};s{ *\\$$}{};s{$$}{ :}' < $*.o.d; \
  } > $*.d && \
  $(RM) $*.o.d
-include $(SRCS:%.cpp=%.d)

coverage:
	-lcov --directory . --capture --output-file coverage.info
	-lcov --remove coverage.info '/googletest-*' '/hdf5-*' '/usr/*' '/opt/*' '/Xcode.app/*' --output-file coverage.info
	-lcov --list coverage.info

clean:
	$(RM) -r *.dSYM
	$(RM) *.gcda *.gcno coverage.info
	$(RM) gtest-all.o
	$(RM) $(SRCS:%.cpp=%.o) $(SRCS:%.cpp=%.d)
	$(RM) selftest

distclean: clean
	$(RM) $(GTEST_DIR).tar.gz
	$(RM) -r $(GTEST_DIR)
	$(RM) gtest

.PHONY: all test coverage clean distclean
