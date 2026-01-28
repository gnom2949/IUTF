CC = gcc
# Added -fPIC for the library
CFLAGS = -Wall -Wextra -std=c99 -g -fsanitize=address -fPIC
LDFLAGS = -fsanitize=address
SRCDIR = src/core
INCDIR = includes

# Files for the shared library
LIB_SOURCES = $(SRCDIR)/iutf-lexer.c $(SRCDIR)/iutf-ast.c $(SRCDIR)/iutf-parser.c \
              $(SRCDIR)/iutf-validator.c $(SRCDIR)/iutf-api.c $(SRCDIR)/iutf-import.c
LIB_TARGET = libiutf.so

# Files for the main program
MAIN = $(SRCDIR)/main.c
TARGET = iutf-parser

all: $(LIB_TARGET) $(TARGET)

# 1. Compile the Shared Library
$(LIB_TARGET): $(LIB_SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -shared -o $@ $(LIB_SOURCES)

# 2. Compile the Program linked to the .so
# -L. tells it to look for libiutf in the current directory
# -Wl,-rpath,'$$ORIGIN' tells the program to find the .so in its own folder at runtime
$(TARGET): $(MAIN) $(LIB_TARGET)
	$(CC) $(CFLAGS) -I$(INCDIR) $(MAIN) -L. -liutf -Wl,-rpath,'$$ORIGIN' -o $@

clean:
	rm -f $(TARGET) $(LIB_TARGET)

.PHONY: all clean

