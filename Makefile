CXX = gcc
CPPFLAGS =
CXXFLAGS = -g -Wall -std=c++11
all: selftest
selftest: selftest.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cpp
clean:
	$(RM) selftest selftest.o
.PHONY: all clean
