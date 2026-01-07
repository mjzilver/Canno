MAKE_FLAGS := -j$(shell nproc --ignore=1)

.PHONY: all run core ui py debug asan clean format

all: core ui

core:
	$(MAKE) $(MAKE_FLAGS) -C core

ui: core
	$(MAKE) $(MAKE_FLAGS) -C ui

py: core
	$(MAKE) $(MAKE_FLAGS) -C py

debug:
	$(MAKE) $(MAKE_FLAGS) -C core debug
	$(MAKE) $(MAKE_FLAGS) -C ui debug

asan:
	$(MAKE) $(MAKE_FLAGS) -C core asan
	$(MAKE) $(MAKE_FLAGS) -C ui asan

format:
	$(MAKE) $(MAKE_FLAGS) -C core format
	$(MAKE) $(MAKE_FLAGS) -C ui format

clean:
	$(MAKE) $(MAKE_FLAGS) -C core clean
	$(MAKE) $(MAKE_FLAGS) -C ui clean
	$(MAKE) $(MAKE_FLAGS) -C py clean
