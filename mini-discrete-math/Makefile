CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

SRCDIR = src
OUTDIR = bin
TESTDIR = tests

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OUTDIR)/%.o)

APPS = logic number set graph tree combo boolalg
TARGETS = $(APPS:%=$(OUTDIR)/%)

TESTS = test_logic test_number
TEST_TARGETS = $(TESTS:%=$(OUTDIR)/%)

.PHONY: all clean test

all: $(TARGETS)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%.o: $(SRCDIR)/%.c | $(OUTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/logic: $(OUTDIR)/logic.o examples/logic_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/number: $(OBJS) examples/rsa_toy_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/set: $(OBJS) examples/relation_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/graph: $(OBJS) examples/graph_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/tree: $(OBJS) examples/huffman_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/combo: $(OBJS) examples/combo_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/boolalg: $(OBJS) examples/circuit_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Test targets
$(OUTDIR)/test_logic: $(OBJS) $(TESTDIR)/test_logic.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/test_number: $(OBJS) $(TESTDIR)/test_number.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: all $(TEST_TARGETS)
	@echo "====== Unit Tests ======"
	@for t in $(TESTS); do echo "=== $$t ==="; $(OUTDIR)/$$t; echo; done
	@echo "====== Integration Demos ======"
	@for t in $(APPS); do echo "=== $$t ==="; $(OUTDIR)/$$t; echo; done

clean:
	rm -rf $(OUTDIR)
