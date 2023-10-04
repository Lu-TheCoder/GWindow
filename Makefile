.PHONY: build
build:
	@clang Sandbox/main.c -o bin/app -L./bin -lGWindow -lobjc  -I./GWindow/src -Wl,-rpath,. -ObjC