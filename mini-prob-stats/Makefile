CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

SRCS = $(wildcard src/*.c)
DEMOS = bayes_demo regression_demo hypothesis_test_demo monte_carlo_pi_demo random_walk_demo

.PHONY: all clean test

all: $(DEMOS:%=$(OUTDIR)/%)
	@for d in $(DEMOS); do echo "=== $$d ==="; $(OUTDIR)/$$d; echo; done

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%_demo: $(SRCS) examples/%_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: all

clean:
	rm -rf $(OUTDIR)
