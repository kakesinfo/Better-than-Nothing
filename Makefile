SHELL := /bin/bash
PRESET := ninja-debug

.PHONY: all configure build fclean re

all: configure build

configure:
	time { cmake --preset $(PRESET); }

build:
	time { cmake --build --preset $(PRESET) --parallel $$(nproc); }

run:
	cmake --build --preset $(PRESET) --target Runtime

fclean:
	rm -rf build

re: fclean all
