# Standalone build for macOS (Apple Clang) and POSIX validation hosts.
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lm
TREK_CPPFLAGS = -D_DARWIN_C_SOURCE -D_DEFAULT_SOURCE -include src/compat.h
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)

.PHONY: all clean
all: bin/trek

bin/trek: $(SOURCES) $(HEADERS) Makefile
	@mkdir -p bin
	$(CC) $(CPPFLAGS) $(TREK_CPPFLAGS) $(CFLAGS) -std=c11 $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f bin/trek
