# Target specific variables
TARGET := sc
CFLAGS += -I. -Wall -Wextra -Werror -DWITH_LINENOISE
LDFLAGS += -lm

ifndef OFLAGS
OFLAGS := -O2
endif #OFLAGS

# Use clang's Address Sanitizer to help detect memory errors
override CFLAGS += -fsanitize=address
override LDFLAGS += -fsanitize=address

# General build path variables
BUILD := build
SRCS := $(wildcard *.c) linenoise/linenoise.c
OBJS := $(patsubst %,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
BUILD_DIR_RULES := $(addsuffix /.dir,$(sort $(dir $(OBJS))))

# Variables for unit testing
BUILDCHK := build/chk
TESTPROG := $(BUILDCHK)/sc_tests
TEST_SRCS := $(filter-out main.c,$(SRCS)) $(wildcard tests/*.c)
TEST_OBJS := $(patsubst %,$(BUILDCHK)/%.o,$(TEST_SRCS))
DEPS := $(sort $(DEPS) $(TEST_OBJS:.o=.d))
BUILD_DIR_RULES := $(sort $(BUILD_DIR_RULES) $(addsuffix /.dir,$(sort $(dir $(TEST_OBJS)))))
TEST_CFLAGS := $(CFLAGS) -DWITH_OBJECT_COUNTS=1

# Tools to use
CLANG := clang
CC := $(CLANG)
LD := $(CLANG)

ANALYZE_FLAGS := -DDEBUG=1 -UNDEBUG -Xanalyzer -analyzer-output=text
ANALYZE_TARGETS := $(addsuffix .analyze,$(SRCS))

# Print all commands executed when VERBOSE is defined
ifdef VERBOSE
_v :=
else
_v := @
endif

# Pretty printing of status messages
RESET_ALL := "\x1b[0m"
RESET_COLOR := "\x1b[39m"
RESET_UNDERLINE := "\x1b[24m"
START_UNDERLINE := "\x1b[4m"
START_GREEN := "\x1b[32m"

# Printing macros
underline = $(START_UNDERLINE)$1$(RESET_UNDERLINE)
color = $(START_$1)$2$(RESET_COLOR)
say = @echo $1
status = $(call say,$(call color,GREEN,'[+] '$1))

# Build the target by default
.PHONY: all
all: $(TARGET)

# Build in debug mode (with asserts enabled)
.PHONY: debug
debug: override CFLAGS += -ggdb -DDEBUG=1 -UNDEBUG
debug: override OFLAGS := -Og
debug: $(TARGET)


# Linking rule
$(TARGET): $(OBJS)
	$(call status,'Linking '$(call underline,'$@'))
	$(_v)$(LD) $(LDFLAGS) -o $@ $^


# Compiling rule
$(BUILD)/%.o: % | $(BUILD_DIR_RULES)
	$(call status,'Analyzing '$(call underline,'$<'))
	$(_v)$(CLANG) --analyze $(CFLAGS) $(ANALYZE_FLAGS) $<
	$(call status,'Compiling '$(call underline,'$<'))
	$(_v)$(CC) $(CFLAGS) $(OFLAGS) -I$(<D) -MD -MP -MF $(BUILD)/$*.d -c -o $@ $<

$(BUILDCHK)/%.o: % | $(BUILD_DIR_RULES)
	$(call status,'[CHK] Analyzing '$(call underline,'$<'))
	$(_v)$(CLANG) --analyze $(TEST_CFLAGS) $(ANALYZE_FLAGS) $<
	$(call status,'[CHK] Compiling '$(call underline,'$<'))
	$(_v)$(CC) $(TEST_CFLAGS) $(OFLAGS) -I$(<D) -MD -MP -MF $(BUILDCHK)/$*.d -c -o $@ $<


# Git submodules must be pulled before compiling this project's sources
$(SRCS): linenoise/linenoise.h
linenoise/linenoise.h:
	$(call status,'Pulling git submodule '$(call underline,'linenoise'))
	$(_v)git submodule update --init --recursive

%.analyze: %
	$(call status,'Analyzing '$(call underline,'$<'))
	$(_v)$(CLANG) --analyze $(ANALYZE_FLAGS) $<

.PHONY: analyze
analyze: $(ANALYZE_TARGETS)

.PHONY: check
check: $(TESTPROG)
	$(call status,'Running test suite')
	$(_v)./$<

$(TESTPROG): $(TEST_OBJS)
	$(call status,'Linking '$(call underline,'$(@F)'))
	$(_v)$(LD) $(LDFLAGS) -o $@ $^

$(wildcard tests/*.c): tests/utest/utest.h
tests/utest/utest.h:
	$(call status,'Pulling git submodule '$(call underline,'utest.h'))
	$(_v)git submodule update --init --recursive


# Build dependency rules
-include $(DEPS)

# Directory creation rule
%/.dir:
	$(_v)mkdir -p $* && touch $@

# Build products cleaning rule
.PHONY: clean
clean:
	$(call status,'Removing build products')
	$(_v)rm -rf $(BUILD) $(TARGET)

# Make sure that the .dir files aren't automatically deleted after building
.SECONDARY:

# Disable built-in rules
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:
