CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LIBS = -lX11 -lcairo
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

TARGET = $(BINDIR)/window-manager

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

all: $(TARGET)

# Build the target executable.
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Build object files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean