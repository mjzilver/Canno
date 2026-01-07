.PHONY: all run core ui py debug asan clean format

all: core ui

core:
	$(MAKE) -C core

ui: core
	$(MAKE) -C ui

py: core
	$(MAKE) -C py

debug:
	$(MAKE) -C core debug
	$(MAKE) -C ui debug

asan:
	$(MAKE) -C core asan
	$(MAKE) -C ui asan

format:
	$(MAKE) -C core format
	$(MAKE) -C ui format

clean:
	$(MAKE) -C core clean
	$(MAKE) -C ui clean
	$(MAKE) -C py clean
