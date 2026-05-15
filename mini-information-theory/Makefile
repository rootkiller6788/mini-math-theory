CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

SRCS = $(wildcard src/*.c)
DEMOS = entropy_demo kl_demo mutual_info_demo huffman_demo channel_demo error_correction_demo

.PHONY: all clean test

all: $(DEMOS:%=$(OUTDIR)/%)
	@for d in $(DEMOS); do echo "=== $$d ==="; $(OUTDIR)/$$d; echo; done

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/entropy_demo: $(SRCS) examples/entropy_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/kl_demo: $(SRCS) examples/kl_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/mutual_info_demo: $(SRCS) examples/mutual_info_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/huffman_demo: $(SRCS) examples/huffman_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/channel_demo: $(SRCS) examples/channel_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/error_correction_demo: $(SRCS) examples/error_correction_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: all

clean:
	rm -rf $(OUTDIR)
