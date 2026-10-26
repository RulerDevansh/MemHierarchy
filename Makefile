# Makefile for CSF Assignment 3

CXX = g++
CXXFLAGS = -g -Wall -std=c++20 -no-pie

# Added flags for Mac memory error detection
# These flags enable AddressSanitizer (ASan) and LeakSanitizer (LSan)
# -fsanitize=address: Enables both ASan and LSan
# -fno-omit-frame-pointer: Improves stack traces for debugging
MAC_FLAGS = -fsanitize=address -fno-omit-frame-pointer
MAC_LDFLAGS = -fsanitize=address

ASMFLAGS = -g -no-pie

LDFLAGS = -no-pie

# C source files for the cache simulator
CSF_SRCS = csim.cpp
CSF_OBJS = $(CSF_SRCS:.cpp=.o)

# Executable Target
EXES = csim

# Added executable target for Mac memory error detection
MAC_EXES = csim_mac

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

%.o.mac : %.cpp
	$(CXX) $(CXXFLAGS) $(MAC_FLAGS) -c $*.cpp -o $*.o.mac

all : $(EXES)

# Building the main executable
csim : $(CSF_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Added build target for Mac memory error detection
mac : $(MAC_EXES)

csim_mac : $(CSF_OBJS:.o=.o.mac)
	$(CXX) $(MAC_LDFLAGS) -o $@ $^

clean :
	rm -f *.o *.o.mac $(EXES) $(MAC_EXES)


.PHONY: solution.zip
solution.zip :
	rm -f $@
	zip -9r $@ *.h *.c *.cpp *.S Makefile README.txt
