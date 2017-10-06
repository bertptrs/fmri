.PHONY: all clean

# Compilation settings
CXXFLAGS=-Wall -Wextra -pedantic -std=c++17 -g -O2
LDLIBS=


# Project artifacts
_EXE=fmri
_OBJECTS=\
		 main.o \
		 Options.o

all: fmri

fmri: $(_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) *.o *.d
	$(RM) $(_EXE)

# Automatic header dependency detection
%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

_DEPFILES=$(patsubst %.cpp, %.d, $(wildcard *.cpp))
-include $(_DEPFILES)
