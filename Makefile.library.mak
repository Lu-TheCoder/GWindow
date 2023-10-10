BUILD_DIR := bin
OBJ_DIR := obj

# Needs to be set by the caller.
# ASSEMBLY := GWindow

DEFINES := -DGEXPORT

ifeq ($(OS),Windows_NT)

else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
	# OSX
	BUILD_PLATFORM := macos
	EXTENSION := .dylib
	COMPILER_FLAGS := -std=c2x -Wall -Werror -Wvla -Wgnu-folding-constant -Wno-missing-braces -fdeclspec -fPIC -ObjC -fobjc-arc
	INCLUDE_FLAGS := -I./$(ASSEMBLY)/src 
	LINKER_FLAGS := -shared -dynamiclib -install_name @rpath/lib$(ASSEMBLY).dylib -lobjc -framework QuartzCore -framework Cocoa -L./$(BUILD_DIR)
	# .c and .m files
	SRC_FILES := $(shell find $(ASSEMBLY) -type f \( -name "*.c" -o -name "*.m" \))
	# directories with .h files
	DIRECTORIES := $(shell find $(ASSEMBLY) -type d)
	OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)
	endif
endif

# Defaults to debug unless release is specified
ifeq ($(TARGET), release)
# Release
else
# Debug
DEFINES += -D_DEBUG
COMPILER_FLAGS += -g -MD 
LINKER_FLAGS += -g
endif

all: scaffold compile link

.PHONY: scaffold
scaffold: # creates build directory
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/,$(DIRECTORIES))

.PHONY: link
link: scaffold $(OBJ_FILES) #link
	@echo Linking "$(ASSEMBLY)"...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/lib$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile:
	@echo --- Performing "$(ASSEMBLY)" $(TARGET) build ---
-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean: # cleans our build directory
	@echo --- Cleaning "$(ASSEMBLY)" ---
	@rm -rf $(BUILD_DIR)/lib$(ASSEMBLY)$(EXTENSION)
	@rm -rf $(OBJ_DIR)/$(ASSEMBLY)

# compile .c to .o object for windows, linux and mac
$(OBJ_DIR)/%.c.o: %.c 
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

# compile .m to .o object only for macos
ifeq ($(BUILD_PLATFORM),macos)
$(OBJ_DIR)/%.m.o: %.m
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)
endif

-include $(OBJ_FILES:.o=.d)
