CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm
OUTDIR = build

SRCS = $(wildcard src/*.c)
DEMOS = bayes_demo regression_demo hypothesis_test_demo monte_carlo_pi_demo random_walk_demo
TEST_SRC = tests/test_all.c

.PHONY: all clean test run-test

all: $(DEMOS:%=$(OUTDIR)/%)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%_demo: $(SRCS) examples/%_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/test_all: $(SRCS) $(TEST_SRC) | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: $(OUTDIR)/test_all
	$(OUTDIR)/test_all

run-test: test

clean:
	rm -rf $(OUTDIR)
