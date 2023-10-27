# This is the compiler and the compile flags you want to use
COMPILER := gcc
COMPILE_FLAGS := -Wall -Werror -g -Og -std=gnu99 -oFast

SOURCE_FOLDER := ../source
OBJECT_FOLDER := ../object

# These are the source files, object files and linker flags for the FIFO files
FIFO_FOLDER := $(SOURCE_FOLDER)/fifo

FIFO_SOURCE_FILES := $(wildcard $(FIFO_FOLDER)/*.c)
FIFO_OBJECT_FILES := $(addprefix $(OBJECT_FOLDER)/, $(notdir $(FIFO_SOURCE_FILES:.c=.o)))

FIFO_LINKER_FLAGS :=

# These are the source files, object files and linker flags for the Socket files
SOCKET_FOLDER := $(SOURCE_FOLDER)/socket

SOCKET_SOURCE_FILES := $(wildcard $(SOCKET_FOLDER)/*.c)
SOCKET_OBJECT_FILES := $(addprefix $(OBJECT_FOLDER)/, $(notdir $(SOCKET_SOURCE_FILES:.c=.o)))

SOCKET_LINKER_FLAGS :=

# These are rules for compiling the executable programs
socket-client: %: $(OBJECT_FOLDER)/%.o $(SOCKET_OBJECT_FILES) $(SOURCE_FOLDER)/%.c $(SOCKET_SOURCE_FILES)
	$(COMPILER) $(OBJECT_FOLDER)/$@.o $(SOCKET_OBJECT_FILES) $(SOCKET_LINKER_FLAGS) -o $@

fifo-socket-server: %: $(OBJECT_FOLDER)/%.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(SOURCE_FOLDER)/%.c $(SOCKET_SOURCE_FILES) $(FIFO_SOURCE_FILES)
	$(COMPILER) $(OBJECT_FOLDER)/$@.o $(SOCKET_OBJECT_FILES) $(FIFO_OBJECT_FILES) $(SOCKET_LINKER_FLAGS) $(FIFO_LINKER_FLAGS) -o $@

fifo-console: %: $(OBJECT_FOLDER)/%.o $(FIFO_OBJECT_FILES) $(SOURCE_FOLDER)/%.c $(FIFO_SOURCE_FILES)
	$(COMPILER) $(OBJECT_FOLDER)/$@.o $(FIFO_OBJECT_FILES) $(FIFO_LINKER_FLAGS) -o $@

# These are rules for compiling object files out of source files
$(OBJECT_FOLDER)/%.o: $(SOURCE_FOLDER)/*/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

$(OBJECT_FOLDER)/%.o: $(SOURCE_FOLDER)/%.c 
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

# These are the executable programs that you can create
PROGRAMS := $(patsubst %.c, %, $(notdir $(wildcard $(SOURCE_FOLDER)/*.c)))

# These are the files that should be kept (not automatically deleted)
.PRECIOUS: $(OBJECT_FOLDER)/*.o $(PROGRAMS)

# This is the clean target, along with which files to delete and how to delete them
CLEAN_TARGET := clean

DELETE_FLAGS :=
DELETE_COMMAND := rm

DELETE_FILES := $(OBJECT_FOLDER)/*.o $(PROGRAMS)

$(CLEAN_TARGET):
	$(DELETE_COMMAND) $(DELETE_FLAGS) $(DELETE_FILES)
