CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -O2 -march=native
LDFLAGS  = -lm

SRCDIR   = src
INCDIR   = include
EXDIR    = examples
TSTDIR   = tests
BUILDDIR = build
LIB      = $(BUILDDIR)/liblinalg.a

SRCS     = $(wildcard $(SRCDIR)/*.c)
OBJS     = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
EXAMPLES = $(wildcard $(EXDIR)/*.c)
EXBINS   = $(patsubst $(EXDIR)/%.c, $(BUILDDIR)/%, $(EXAMPLES))
TEST_SRC = $(TSTDIR)/test_all.c
TEST_BIN = $(BUILDDIR)/test_all

.PHONY: all test clean

all: $(LIB) $(EXBINS)

$(LIB): $(OBJS)
	@mkdir -p $(BUILDDIR)
	$(AR) rcs $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/%: $(EXDIR)/%.c $(LIB)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -llinalg $(LDFLAGS) -o $@

$(TEST_BIN): $(TEST_SRC) $(LIB)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -llinalg $(LDFLAGS) -o $@

examples/%: $(BUILDDIR)/% ;

test: $(TEST_BIN)
	@echo "=== Running Test Suite ==="
	@$(TEST_BIN)
	@echo ""

clean:
	rm -rf $(BUILDDIR)
