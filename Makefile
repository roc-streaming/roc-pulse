all: build

.PHONY: build
build:
	mkdir -p build
	cd build && cmake ..
	make --no-print-directory -C build

clean:
	rm -rf build
	rm -rf bin
