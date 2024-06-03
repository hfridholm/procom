# These are the names of the targets
CLEAN_TARGET := clean
HELP_TARGET := help

# These are the flags and command for the clean target
DELETE_CMD := rm

# This is the compiler and the compile flags you want to use
COMPILER := gcc
COMPILE_FLAGS := -Wall -Werror -g -Og -std=gnu99 -oFast
LINKER_FLAGS := -pthread

SOURCE_DIR := ../source
OBJECT_DIR := ../object
BINARY_DIR := ../binary

# These are the source files, object files and linker flags for the FIFO files
FIFO_SOURCE_FILES := $(SOURCE_DIR)/fifo.c
FIFO_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(FIFO_SOURCE_FILES:.c=.o)))

# These are the source files, object files and linker flags for the Socket files
SOCKET_SOURCE_FILES := $(SOURCE_DIR)/socket.c
SOCKET_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(SOCKET_SOURCE_FILES:.c=.o)))

# These are the source files, object files and linker flags for the Debug files
DEBUG_SOURCE_FILES := $(SOURCE_DIR)/debug.c
DEBUG_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(DEBUG_SOURCE_FILES:.c=.o)))

# These are the source files, object files and linker flags for the Thread files
THREAD_SOURCE_FILES := $(SOURCE_DIR)/thread.c
THREAD_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(THREAD_SOURCE_FILES:.c=.o)))

all: socket-console fifo-socket fifo-console console

# These are rules for compiling the executable programs
socket-console: %: $(OBJECT_DIR)/%.o $(SOCKET_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(SOCKET_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(SOCKET_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(LINKER_FLAGS) -o $(BINARY_DIR)/$@

fifo-socket: %: $(OBJECT_DIR)/%.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(SOCKET_SOURCE_FILES) $(FIFO_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(LINKER_FLAGS) -o $(BINARY_DIR)/$@

fifo-console: %: $(OBJECT_DIR)/%.o $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(FIFO_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(LINKER_FLAGS) -o $(BINARY_DIR)/$@

console: %: $(OBJECT_DIR)/%.o $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(LINKER_FLAGS) -o $(BINARY_DIR)/$@

# These are rules for compiling object files out of source files
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/*/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

# These are the executable programs that you can create
PROGRAMS := $(patsubst %.c, %, $(notdir $(wildcard $(SOURCE_DIR)/*.c)))

# These are the files that should be cleaned
DELETE_FILES := $(OBJECT_DIR)/*.o $(PROGRAMS)

# These are the files that should be kept (not automatically deleted)
.PRECIOUS: $(DELETE_FILES)

$(CLEAN_TARGET):
	$(DELETE_CMD) $(DELETE_FLAGS) $(DELETE_FILES)

$(HELP_TARGET):
	@echo $(PROGRAMS) $(CLEAN_TARGET)
