CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include -DCONTRACT_CHECK
LDFLAGS = -lm

SRCS = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

EXAMPLES = verified_stack_demo verified_queue_demo verified_sort_demo

.PHONY: all clean test

all: $(EXAMPLES:%=$(OUTDIR)/%)
	$(OUTDIR)/verified_stack_demo
	$(OUTDIR)/verified_queue_demo
	$(OUTDIR)/verified_sort_demo

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/verified_stack_demo: $(SRCS) examples/verified_stack_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/verified_queue_demo: $(SRCS) examples/verified_queue_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/verified_sort_demo: $(SRCS) examples/verified_sort_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: all

clean:
	rm -rf $(OUTDIR)
