ifneq ($(TOOLCHAIN_PREFIX),)
toolchain := $(TOOLCHAIN_PREFIX)
else
toolchain := $(shell cc -v -E - </dev/null 2>&1 | grep Target | awk '{print $$2}')
endif

all: build

.PHONY: build
build:
	mkdir -p build/$(toolchain)
	cd build/$(toolchain) && cmake ../..
	$(MAKE) --no-print-directory -C build/$(toolchain)

install:
	$(MAKE) --no-print-directory -C build/$(toolchain) install

clean:
	rm -rf build
	rm -rf bin
	rm -f compile_commands.json

md:
	markdown-toc --maxdepth 2 -i README.md
	md-authors --format modern --append AUTHORS.md
