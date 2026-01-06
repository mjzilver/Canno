.PHONY: all run core ui py clean

all: core ui py

core:
	$(MAKE) -C core

ui: core
	$(MAKE) -C ui

py: core
	$(MAKE) -C py

clean:
	$(MAKE) -C core clean
	$(MAKE) -C ui clean
	$(MAKE) -C py clean
