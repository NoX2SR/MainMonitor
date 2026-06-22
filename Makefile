.PHONY: all build test verify clean

all: verify

build:
	./scripts/build.sh

test:
	./scripts/test.sh

verify: build test

clean:
	rm -rf build
