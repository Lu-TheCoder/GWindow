ASSEMBLY:= Sandbox
BUILD_DIR:=bin 
COMPILER_FLAGS:= -D_DEBUG -std=c2x -g -Wall -ObjC
INCLUDE_FLAGS:=  -I./GWindow/src -L./bin -I$(VULKAN_SDK)/include -I$(VULKAN_SDK)/lib
LINKER_FLAGS:= -lGWindow -lvulkan -lobjc -Wl,-rpath,.
SOURCE_FILES:= $(shell find $(ASSEMBLY) -type f \( -name "*.c" -o -name "*.m" \))

.PHONY: build
build:
	@clang $(COMPILER_FLAGS) $(SOURCE_FILES) -o bin/app $(LINKER_FLAGS)  $(INCLUDE_FLAGS)