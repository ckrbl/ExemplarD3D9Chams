.PHONY: all format
all:
	echo "Nothing here"

format:
	clang-format -i src/*.cpp
