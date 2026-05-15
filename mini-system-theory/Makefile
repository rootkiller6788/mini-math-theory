CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -O2 -march=native
LDFLAGS  = -lm

SRCDIR   = src
INCDIR   = include
EXDIR    = examples
BUILDDIR = build
LIB      = $(BUILDDIR)/libsystem.a

SRCS     = $(wildcard $(SRCDIR)/*.c)
OBJS     = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
EXAMPLES = $(wildcard $(EXDIR)/*.c)
EXBINS   = $(patsubst $(EXDIR)/%.c, $(BUILDDIR)/%, $(EXAMPLES))

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
	$(CC) $(CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lsystem $(LDFLAGS) -o $@

define run_example
	@echo "=== Running $(1) ==="
	@$(BUILDDIR)/$(1) || echo "[FAIL] $(1)"
	@echo ""
endef

test: $(EXBINS)
	$(foreach ex,$(notdir $(EXBINS)),$(call run_example,$(ex)))

clean:
	rm -rf $(BUILDDIR)
