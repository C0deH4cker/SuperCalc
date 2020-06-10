# Target specific variables
TARGET := sc
CFLAGS += -Wall -Wextra -Werror
LDFLAGS += -lm

ifndef OFLAGS
OFLAGS := -O2
endif #OFLAGS

# General build path variables
BUILD := build
SRCS := $(wildcard *.c) linenoise/linenoise.c
OBJS := $(patsubst %,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
BUILD_DIR_RULES := $(addsuffix /.dir,$(sort $(dir $(OBJS))))

# Tools to use
CLANG := clang
CC := $(CLANG)
LD := $(CLANG)

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
all: $(TARGET)

.PHONY: all

# Build in debug mode (with asserts enabled)
.PHONY: debug
debug: override CFLAGS += -ggdb -DDEBUG=1 -UNDEBUG
debug: override OFLAGS :=
debug: $(TARGET)

# Uses clang's Address Sanitizer to help detect memory errors
.PHONY: debug+
debug+: override CFLAGS += -fsanitize=address
debug+: override LDFLAGS += -fsanitize=address
debug+: debug


# Linking rule
$(TARGET): $(OBJS)
	$(call status,'Linking '$(call underline,'$@'))
	$(_v)$(LD) $(LDFLAGS) -o $@ $^


# Compiling rule
$(BUILD)/%.o: % | $(BUILD_DIR_RULES)
	$(call status,'Compiling '$(call underline,'$<'))
	$(_v)$(CC) $(CFLAGS) $(OFLAGS) -I$(<D) -MD -MP -MF $(BUILD)/$*.d -c -o $@ $<


# Git submodules must be pulled before compiling this project's sources
$(SRCS): linenoise/linenoise.h
linenoise/linenoise.h:
	$(call status,'Pulling git submodule '$(call underline,'linenoise'))
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
