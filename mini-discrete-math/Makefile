CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

SRCDIR = src
OUTDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OUTDIR)/%.o)

APPS = logic number set graph tree combo boolalg
TARGETS = $(APPS:%=$(OUTDIR)/%)

.PHONY: all clean test

all: $(TARGETS)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%.o: $(SRCDIR)/%.c | $(OUTDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/logic: $(OUTDIR)/logic.o examples/logic_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/number: $(OUTDIR)/number.o examples/rsa_toy_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/set: $(OUTDIR)/set.o $(OUTDIR)/relation.o examples/relation_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/graph: $(OUTDIR)/graph.o examples/graph_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/tree: $(OUTDIR)/tree.o examples/huffman_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/combo: $(OUTDIR)/combo.o examples/combo_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/boolalg: $(OUTDIR)/boolalg.o examples/circuit_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: all
	@for t in $(APPS); do echo "=== $$t ==="; $(OUTDIR)/$$t; echo; done

clean:
	rm -rf $(OUTDIR)
