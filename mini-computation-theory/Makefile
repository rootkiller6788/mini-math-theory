CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lm

SRCDIR = src
EXDIR = examples
OUTDIR = bin
SRCS = $(wildcard $(SRCDIR)/*.c)

DFA_DEPS = $(SRCDIR)/dfa.c
NFA_DEPS = $(SRCDIR)/nfa.c $(SRCDIR)/dfa.c
REGEX_DEPS = $(SRCDIR)/regex.c $(SRCDIR)/nfa.c $(SRCDIR)/dfa.c
GRAMMAR_DEPS = $(SRCDIR)/grammar.c
PDA_DEPS = $(SRCDIR)/pda.c
TURING_DEPS = $(SRCDIR)/turing_machine.c
SAT_DEPS = $(SRCDIR)/sat.c
CHOMSKY_DEPS = $(SRCDIR)/chomsky.c $(SRCDIR)/grammar.c
DECIDABILITY_DEPS = $(SRCDIR)/decidability.c
THEORY_DEPS = $(SRCDIR)/chomsky.c $(SRCDIR)/decidability.c $(SRCDIR)/grammar.c

.PHONY: all clean test check

all: $(OUTDIR)/dfa_demo $(OUTDIR)/regex_demo $(OUTDIR)/grammar_demo \
     $(OUTDIR)/turing_demo $(OUTDIR)/sat_demo $(OUTDIR)/theory_demo

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/dfa_demo: $(DFA_DEPS) $(EXDIR)/dfa_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/regex_demo: $(REGEX_DEPS) $(EXDIR)/regex_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/grammar_demo: $(GRAMMAR_DEPS) $(EXDIR)/grammar_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/turing_demo: $(TURING_DEPS) $(EXDIR)/turing_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/sat_demo: $(SAT_DEPS) $(EXDIR)/sat_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUTDIR)/theory_demo: $(THEORY_DEPS) $(EXDIR)/theory_demo.c | $(OUTDIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

check: all
	@echo "=== Compiling all source for verification ==="
	$(CC) $(CFLAGS) -c $(SRCDIR)/chomsky.c -o $(OUTDIR)/chomsky.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/decidability.c -o $(OUTDIR)/decidability.o
	@echo "All sources compile cleanly."

test: check
	@for t in dfa regex grammar turing sat theory; do \
		echo "=== $$t ==="; \
		$(OUTDIR)/$${t}_demo; \
		echo; \
	done

clean:
	rm -rf $(OUTDIR) *.o $(EXDIR)/*.o $(EXDIR)/*.exe
