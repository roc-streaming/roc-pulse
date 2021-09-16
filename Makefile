all: build

.PHONY: build
build:
	mkdir -p build
	cd build && cmake ..
	$(MAKE) --no-print-directory -C build

install:
	$(MAKE) --no-print-directory -C build install

clean:
	rm -rf build
	rm -rf bin
