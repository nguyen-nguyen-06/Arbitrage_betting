# arb — arbitrage finder
#
#   make            build bin/arb
#   make test       build + run all tests
#   make clean
#
# Requires: g++ (C++17), libcurl dev files (MSYS2: pacman -S mingw-w64-ucrt-x86_64-curl),
#           third_party/json.hpp (nlohmann/json single header).

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -MMD -MP
LDLIBS   ?= -lcurl

ifeq ($(OS),Windows_NT)
  EXE := .exe
else
  EXE :=
endif

BIN   := bin
BUILD := build

# Core library: everything except main and tests.
LIB_SRC := simplex.cc \
           arb_model/arb_util.cc \
           arb_model/crawler.cc \
           arb_model/http.cc \
           arb_model/modern/arb_cal.cc \
           arb_model/legacy/arb_solver.cc
LIB_OBJ := $(LIB_SRC:%.cc=$(BUILD)/%.o)

ARB      := $(BIN)/arb$(EXE)
T_SIMPLEX:= $(BIN)/test_simplex$(EXE)
T_ARB    := $(BIN)/test_arb$(EXE)

.PHONY: all test clean
all: $(ARB)

$(ARB): $(LIB_OBJ) $(BUILD)/main.o
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# test_simplex needs only the solver — no curl, no json.
$(T_SIMPLEX): $(BUILD)/simplex.o $(BUILD)/test/test_simplex.o
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(T_ARB): $(LIB_OBJ) $(BUILD)/test/test_arb.o
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

test: $(T_SIMPLEX) $(T_ARB)
	./$(T_SIMPLEX)
	./$(T_ARB)

$(BUILD)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN) $(BUILD)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
