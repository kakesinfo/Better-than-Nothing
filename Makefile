SHELL := /bin/bash
PRESET := ninja-debug

.PHONY: all configure build fclean re

all: configure build

configure:
	time { cmake --preset $(PRESET); }

build:
	time { cmake --build --preset $(PRESET) --parallel $$(nproc); }

fclean:
	rm -rf build

re: fclean all
