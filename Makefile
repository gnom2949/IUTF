CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -fsanitize=address
LDFLAGS = -fsanitize=address
SRCDIR = src/core
INCDIR = includes
SOURCES = $(SRCDIR)/iutf-lexer.c $(SRCDIR)/iutf-ast.c $(SRCDIR)/iutf-parser.c $(SRCDIR)/iutf-validator.c $(SRCDIR)/iutf-api.c $(SRCDIR)/iutf-import.c
HEADERS = $(wildcard $(INCDIR)/*.h)
TARGET = iutf-parser
MAIN = $(SRCDIR)/main.c

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) $(MAIN)
	$(CC) $(CFLAGS) -I$(INCDIR) $(SOURCES) $(MAIN) -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
