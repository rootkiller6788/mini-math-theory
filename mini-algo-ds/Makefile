CC       := gcc
CFLAGS   := -Wall -Wextra -std=c99 -O2 -Iinclude
LDFLAGS  :=
DBG_FLAGS:= -g -O0

BUILD_DIR := build
SRC_DIR   := src
INC_DIR   := include
TEST_DIR  := tests
EX_DIR    := examples
DEMO_DIR  := demos

SRCS      := $(wildcard $(SRC_DIR)/*.c)
OBJS      := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TESTS     := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/test_%, $(TESTS))
EXAMPLES  := $(wildcard $(EX_DIR)/*.c)
EX_BINS   := $(patsubst $(EX_DIR)/%.c, $(BUILD_DIR)/ex_%, $(EXAMPLES))

.PHONY: all clean test dirs debug

all: dirs $(OBJS) $(TEST_BINS) $(EX_BINS)

dirs:
	@mkdir -p "$(BUILD_DIR)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%: $(TEST_DIR)/%.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/ex_%: $(EX_DIR)/%.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

# Convenience targets to build individual examples by their base name
define example_target
$(1): $(BUILD_DIR)/ex_$(1)
endef
$(foreach ex,$(notdir $(basename $(EXAMPLES))),$(eval $(call example_target,$(ex))))

test: all
	@echo === Running all tests ===
	@for t in $(TEST_BINS); do echo --- $$t ---; $$t; done

clean:
	@rm -rf "$(BUILD_DIR)"
	@echo Clean complete.

debug: CFLAGS = -Wall -Wextra -std=c99 $(DBG_FLAGS) -Iinclude
debug: all
