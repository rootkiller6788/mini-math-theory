CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include -DCONTRACT_CHECK
LDFLAGS = -lm
OUTDIR = build

SRCS = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

EXAMPLES = verified_stack_demo verified_queue_demo verified_sort_demo \
           wp_calculus_demo abstract_interpret_demo ltl_model_check_demo \
           concolic_test_demo test_suite

.PHONY: all clean test

all: $(EXAMPLES:%=$(OUTDIR)/%)
	@echo "===== All examples built ====="

test: $(EXAMPLES:%=$(OUTDIR)/%)
	@echo "===== Running all modules ====="
	$(OUTDIR)/verified_stack_demo
	$(OUTDIR)/verified_queue_demo
	$(OUTDIR)/verified_sort_demo
	$(OUTDIR)/wp_calculus_demo
	$(OUTDIR)/abstract_interpret_demo
	$(OUTDIR)/ltl_model_check_demo
	$(OUTDIR)/concolic_test_demo
	$(OUTDIR)/test_suite
	@echo ""
	@echo "===== All tests passed ====="

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%: examples/%.c $(SRCS) | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(OUTDIR)
