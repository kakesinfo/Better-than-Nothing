SHELL := /bin/bash

all:
	mkdir -p build/debug
	time { cd build/debug; cmake --preset ninja-debug ../.. ; ninja -j$(shell nproc); }

ninja-debug:
	time { cd build/debug; ninja -j$(shell nproc); }

fclean:
	rm -rf build

re: fclean all
