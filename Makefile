.PHONY: static

static:
	@mkdir -p bin || exit
	@echo "Building static binary..."
	@g++ -Wall -Werror -static -static-libgcc -static-libstdc++ -std=c++23 src/*.cpp -o bin/gh_wh_handler || exit

