# Compiler
CC = gcc

# Output file name
TARGET = lex

# Source files
SRC = lex.c

# Compiler flags
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
         -Wcast-align -Wpointer-arith -Wwrite-strings \
         -Wmissing-declarations -Wmissing-prototypes \
         -Wstrict-prototypes -Wold-style-definition \
         -Winline -Wconversion -Wuninitialized \
         -g -O3 -fno-omit-frame-pointer

# Linker flags (optional but good with ASan)
LDFLAGS = -fsanitize=address

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)

.PHONY: all clean