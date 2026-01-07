PYTHON := $(shell command -v python3 || command -v python)
PY_FILES := $(shell find . -name "*.py")
CONFIG := config.py
LIBCANNO := ../bin/libcanno.so

.PHONY: py format clean

all: py

py: $(LIBCANNO) $(CONFIG)
	$(PYTHON) front.py

$(CONFIG): $(LIBCANNO)
	rm -f $(CONFIG)
	@echo "LIBCANNO_PATH = \"$(LIBCANNO)\"" >> $@

format:
	black $(PY_FILES)

clean:
	rm -rf ./__pycache__