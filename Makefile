DEFAULT_EXE=bin/main

CXXFLAGS_BASE=-std=c++20 -Wall -Werror -pedantic-errors -Iinclude -Isrc
LDFLAGS_BASE=-std=c++20

# Optimized build
CXXFLAGS_REL=$(CXXFLAGS_BASE) -O2 -DNDEBUG
LDFLAGS_REL=$(LDFLAGS_BASE)

# Debug build (for valgrind/gdb)
CXXFLAGS_DBG=$(CXXFLAGS_BASE) -Og -ggdb -fno-inline -fno-omit-frame-pointer -D_DEBUG
LDFLAGS_DBG=$(LDFLAGS_BASE)

# Source files
EXECUTABLE_SOURCE_FILES=$(shell /usr/bin/find src -name "main*.cpp" 2>/dev/null)
SOURCE_FILES=$(filter-out $(EXECUTABLE_SOURCE_FILES),$(shell /usr/bin/find src -name "*.cpp" 2>/dev/null))
HEADER_FILES=$(shell /usr/bin/find include src -name "*.hpp" 2>/dev/null)

OBJECT_FILES_REL=$(SOURCE_FILES:src/%.cpp=build/exe/%.o)
OBJECT_FILES_DBG=$(SOURCE_FILES:src/%.cpp=build/exe/%_dbg.o)
EXECUTABLES_REL=$(patsubst src/%.cpp,bin/%,$(EXECUTABLE_SOURCE_FILES))
EXECUTABLES_DBG=$(patsubst src/%.cpp,bin/%_dbg,$(EXECUTABLE_SOURCE_FILES))

.PHONY: all clean exe exe_dbg run run_dbg

all: exe exe_dbg

exe: $(EXECUTABLES_REL)
exe_dbg: $(EXECUTABLES_DBG)

run: $(DEFAULT_EXE)
	$(DEFAULT_EXE)

run_dbg: bin/main_dbg
	bin/main_dbg

$(EXECUTABLES_REL): bin/%: build/exe/%.o $(OBJECT_FILES_REL)
	mkdir -p $(dir $@)
	g++ $^ $(LDFLAGS_REL) -o $@

$(EXECUTABLES_DBG): bin/%: build/exe/%.o $(OBJECT_FILES_DBG)
	mkdir -p $(dir $@)
	g++ $^ $(LDFLAGS_DBG) -o $@

build/exe/%.o: src/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_REL) -o $@

build/exe/%_dbg.o: src/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_DBG) -o $@

clean:
	rm -rf bin/ build/