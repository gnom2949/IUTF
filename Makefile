CC = gcc
CFLAGS = -Wall Wextra -std=c99 -g
SRCDIR = src
INCDIR = includes
SOURCES = $(SRCDIR)/iutf-lexer.c
HEADERS = $(INCDIR)/iutf-lexer.h
TARGET = example
EXAMPLE = example.c

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) $(EXAMPLE)
	$(CC) $(CFLAGS) -I$(INCDIR) $(SOURCES) $(EXAMPLE) -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
