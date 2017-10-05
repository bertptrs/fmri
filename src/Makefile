.PHONY: all clean

# Compilation settings
CXXFLAGS=-Wall -Wextra -pedantic -std=c++14 -g -O2
LDLIBS=


# Project artifacts
_EXE=fmri
_OBJECTS=\
		 main.o

all: fmri

fmri: $(_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) *.o *.d
	$(RM)

# Automatic header dependency detection
%.d: %.cpp
	$(CXX) -MM -MF $@ $<

_DEPFILES=$(patsubst %.cpp, %.d, $(wildcard *.cpp))
-include $(_DEPFILES)
