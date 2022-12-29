ifneq ($(TOOLCHAIN_PREFIX),)
subdir := $(TOOLCHAIN_PREFIX)
else
subdir := native
endif

all: build

.PHONY: build
build:
	mkdir -p build/$(subdir)
	cd build/$(subdir) && cmake ../..
	$(MAKE) --no-print-directory -C build/$(subdir)

install:
	$(MAKE) --no-print-directory -C build/$(subdir) install

clean:
	rm -rf build
	rm -rf bin
	rm -f compile_commands.json
