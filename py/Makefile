PYTHON := $(shell command -v python3 || command -v python)
PY_FILES := $(shell find . -name "*.py")
CONFIG := config.py
LIBCANNO := ../bin/libcanno.so

.PHONY: py format clean debug

all: py

py: $(LIBCANNO) $(CONFIG)
	$(PYTHON) front.py

$(CONFIG): $(LIBCANNO)
	rm -f $(CONFIG)
	@echo "LIBCANNO_PATH = \"$(LIBCANNO)\"" >> $@

debug: 
	gdb --args $(PYTHON) front.py

format:
	black $(PY_FILES)

clean:
	rm -rf ./__pycache__