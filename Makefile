.PHONY: all format package clean
all:
	@echo "Nothing here"

format:
	@clang-format -i include/*.h src/*.cpp

package:
	git archive --format=tar.gz -o Release/ExemplarD3D9Chams.zip main
	git archive --format=zip -o Release/ExemplarD3D9Chams.zip main

clean:
	@rm -rf Debug
	@rm -rf Release