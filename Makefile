OBJECTS = vibetune.com
DEST = ../RomWBW/Binary/Apps
TOOLS = ../RomWBW/Tools
Z88DK_TICKS = /Users/mduraes/z88dk/bin/z88dk-ticks
MAME_DISK = /Volumes/FATDISK

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm $(shell find . -name '*.inc')

vibetune.com: $(DEPS)
	$(TASM) -dWBW vibetune.asm vibetune.com vibetune.lst
	@echo "\n=== Build successful! ==="
	@echo "Binary size: $$(wc -c < vibetune.com) bytes"

# Emulator test target
test: vibetune.com
	@echo "\n=== Running VibeTune in z88dk-ticks emulator ==="
	@echo "Expected: VibeTune banner followed by HBIOS version error"
	@echo "---"
	$(Z88DK_TICKS) -pc 0x100 vibetune.com
	@echo "---"
	@echo "✅ Emulator test completed successfully!"

# Deploy to MAME disk
deploy: vibetune.com test
	@echo "\n=== Deploying to MAME disk ==="
	@if [ -d "$(MAME_DISK)" ]; then \
		cp vibetune.com "$(MAME_DISK)/vibetune.com"; \
		echo "✅ VibeTune copied to $(MAME_DISK)"; \
	else \
		echo "❌ MAME disk not mounted at $(MAME_DISK)"; \
		echo "   Please mount your FATDISK and try again"; \
		exit 1; \
	fi

# Build, test, and deploy in one command
release: vibetune.com test deploy
	@echo "\n🎵 VibeTune ready for testing in MAME! 🎵"

# Show available targets
help:
	@echo "VibeTune v0.1.0 - Available Make Targets:"
	@echo ""
	@echo "  vibetune.com  - Build the VibeTune executable"
	@echo "  test          - Run emulator test with z88dk-ticks"
	@echo "  deploy        - Copy to MAME disk (requires FATDISK mounted)"
	@echo "  release       - Build, test, and deploy in one command"
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Environment:"
	@echo "  Z88DK_TICKS = $(Z88DK_TICKS)"
	@echo "  MAME_DISK   = $(MAME_DISK)"
	@echo ""

clean::
	@echo "Cleaning VibeTune build artifacts..."
	rm -f vibetune.com vibetune.lst
	@echo "✅ VibeTune clean completed"

.PHONY: test deploy release help

all::
	mkdir -p $(DEST)/Tunes
	cp Tunes/* $(DEST)/Tunes
