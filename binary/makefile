PROGRAM := procom

CLEAN_TARGET := clean
HELP_TARGET  := help

DELETE_CMD := rm

COMPILER := gcc
COMPILE_FLAGS := -Wall -Werror -g -O0 -std=gnu99 -oFast

SOURCE_DIR := ../source
OBJECT_DIR := ../object
BINARY_DIR := ../binary

SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.c)
HEADER_FILES := $(wildcard $(SOURCE_DIR)/*.h)

OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCE_FILES:.c=.o)))

all: $(PROGRAM)

$(PROGRAM): $(OBJECT_FILES) $(SOURCE_FILES) $(HEADER_FILES)
	$(COMPILER) $(OBJECT_FILES) -o $(BINARY_DIR)/$(PROGRAM)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

.PRECIOUS: $(OBJECT_DIR)/%.o $(PROGRAM)

$(CLEAN_TARGET):
	$(DELETE_CMD) $(OBJECT_DIR)/*.o $(PROGRAM)

$(HELP_TARGET):
	@echo $(PROGRAM) $(CLEAN_TARGET)
