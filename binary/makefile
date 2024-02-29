# This is the compiler and the compile flags you want to use
COMPILER := gcc
COMPILE_FLAGS := -Wall -Werror -g -Og -std=gnu99 -oFast

SOURCE_DIR := ../source
OBJECT_DIR := ../object

# These are the source files, object files and linker flags for the FIFO files
FIFO_DIR := $(SOURCE_DIR)/fifo

FIFO_SOURCE_FILES := $(wildcard $(FIFO_DIR)/*.c)
FIFO_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(FIFO_SOURCE_FILES:.c=.o)))

FIFO_LINKER_FLAGS :=

# These are the source files, object files and linker flags for the Socket files
SOCKET_DIR := $(SOURCE_DIR)/socket

SOCKET_SOURCE_FILES := $(wildcard $(SOCKET_DIR)/*.c)
SOCKET_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(SOCKET_SOURCE_FILES:.c=.o)))

SOCKET_LINKER_FLAGS :=

# These are the source files, object files and linker flags for the Debug files
DEBUG_DIR := $(SOURCE_DIR)/debug

DEBUG_SOURCE_FILES := $(wildcard $(DEBUG_DIR)/*.c)
DEBUG_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(DEBUG_SOURCE_FILES:.c=.o)))

DEBUG_LINKER_FLAGS :=

# These are the source files, object files and linker flags for the Thread files
THREAD_DIR := $(SOURCE_DIR)/thread

THREAD_SOURCE_FILES := $(wildcard $(THREAD_DIR)/*.c)
THREAD_OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(THREAD_SOURCE_FILES:.c=.o)))

THREAD_LINKER_FLAGS := -pthread

default: socket-console fifo-socket fifo-console console

# These are rules for compiling the executable programs
socket-console: %: $(OBJECT_DIR)/%.o $(SOCKET_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(SOCKET_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(SOCKET_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOCKET_LINKER_FLAGS) $(THREAD_LINKER_FLAGS) $(DEBUG_LINKER_FLAGS) -o $@

fifo-socket: %: $(OBJECT_DIR)/%.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(SOCKET_SOURCE_FILES) $(FIFO_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOCKET_LINKER_FLAGS) $(FIFO_LINKER_FLAGS) $(THREAD_LINKER_FLAGS) $(DEBUG_LINKER_FLAGS) -o $@

fifo-console: %: $(OBJECT_DIR)/%.o $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(FIFO_SOURCE_FILES) $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(FIFO_OBJECT_FILES) $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(FIFO_LINKER_FLAGS) $(THREAD_LINKER_FLAGS) $(DEBUG_LINKER_FLAGS) -o $@

console: %: $(OBJECT_DIR)/%.o $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(SOURCE_DIR)/%.c $(THREAD_SOURCE_FILES) $(DEBUG_SOURCE_FILES)
	$(COMPILER) $(OBJECT_DIR)/$@.o $(THREAD_OBJECT_FILES) $(DEBUG_OBJECT_FILES) $(THREAD_LINKER_FLAGS) $(DEBUG_LINKER_FLAGS) -o $@

# These are rules for compiling object files out of source files
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/*/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

# These are the executable programs that you can create
PROGRAMS := $(patsubst %.c, %, $(notdir $(wildcard $(SOURCE_DIR)/*.c)))

# These are the files that should be kept (not automatically deleted)
.PRECIOUS: $(OBJECT_DIR)/*.o $(PROGRAMS)

# This is the clean target, along with which files to delete and how to delete them
CLEAN_TARGET := clean

DELETE_FLAGS :=
DELETE_COMMAND := rm

DELETE_FILES := $(OBJECT_DIR)/*.o $(PROGRAMS)

$(CLEAN_TARGET):
	$(DELETE_COMMAND) $(DELETE_FLAGS) $(DELETE_FILES)
