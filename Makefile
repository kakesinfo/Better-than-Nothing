SHELL := /bin/bash
PRESET := ninja-debug

.PHONY: all configure build check-format check-tidy fclean re

all: configure build

configure:
	time { cmake --preset $(PRESET); }

build:
	time { cmake --build --preset $(PRESET) --parallel $$(nproc); }

check-format:
	time { find Sources -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run --Werror; }

format:
	time { find Sources -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i; }

fclean:
	rm -rf build

re: fclean all
