CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

OUTDIR = bin
SRCS = $(wildcard src/*.c)
TEST_SRC = tests/test_all.c
DEMOS = entropy_demo kl_demo mutual_info_demo huffman_demo channel_demo error_correction_demo

.PHONY: all clean test demos

all: test demos

# ── Test target (one-click pass) ────────────────────────────────────
$(OUTDIR)/test_all: $(SRCS) $(TEST_SRC) | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: $(OUTDIR)/test_all
	@echo "=== Running Test Suite ==="
	$(OUTDIR)/test_all
	@echo "=== All tests passed ==="

# ── Demo targets ────────────────────────────────────────────────────
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

demos: $(DEMOS:%=$(OUTDIR)/%)
	@for d in $(DEMOS); do echo "=== $$d ==="; $(OUTDIR)/$$d; echo; done

clean:
	rm -rf $(OUTDIR)
