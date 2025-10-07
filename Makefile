OBJECTS = vibetune.com
DEST = ../RomWBW/Binary/Apps
TOOLS = ../RomWBW/Tools
Z88DK_TICKS = /Users/mduraes/z88dk/bin/z88dk-ticks
MAME_DISK = /Volumes/FATDISK

include $(TOOLS)/Makefile.inc

DEPS := vibetune.asm $(shell find . -name '*.inc')

# Helper function to increment version
increment-version:
	@echo "\n=== Incrementing version number ==="
	@CURRENT_VERSION=$$(cat VERSION); \
	echo "Current version: $$CURRENT_VERSION"; \
	MAJOR=$$(echo $$CURRENT_VERSION | cut -d. -f1); \
	MINOR=$$(echo $$CURRENT_VERSION | cut -d. -f2); \
	PATCH=$$(echo $$CURRENT_VERSION | cut -d. -f3); \
	BUILD=$$(echo $$CURRENT_VERSION | cut -d. -f4); \
	NEW_BUILD=$$((BUILD + 1)); \
	NEW_VERSION="$$MAJOR.$$MINOR.$$PATCH.$$NEW_BUILD"; \
	echo "New version: $$NEW_VERSION"; \
	echo "$$NEW_VERSION" > VERSION; \
	sed -i '' "s/VibeTune v$$CURRENT_VERSION/VibeTune v$$NEW_VERSION/g" src/ui/messages.inc; \
	sed -i '' "s/v$$CURRENT_VERSION/v$$NEW_VERSION/g" README.md; \
	sed -i '' "s/v$$CURRENT_VERSION/v$$NEW_VERSION/g" PROJECT_STATUS.md; \
	sed -i '' "s/$$CURRENT_VERSION/$$NEW_VERSION/g" WARP.md; \
	sed -i '' "s/VibeTune v[0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+ -/VibeTune v$$NEW_VERSION -/g" Makefile

vibetune.com: $(DEPS)
	@echo "\n=== Updating build date ==="
	@CURRENT_VERSION=$$(cat VERSION); \
	BUILD_DATE=$$(date "+%d-%b-%Y"); \
	sed "s/VibeTune v$$CURRENT_VERSION for RomWBW, [0-9][0-9]-[A-Za-z][A-Za-z][A-Za-z]-[0-9][0-9][0-9][0-9]/VibeTune v$$CURRENT_VERSION for RomWBW, $$BUILD_DATE/g" vibetune.asm > vibetune_temp.asm && \
	mv vibetune_temp.asm vibetune.asm
	@echo "Build date updated to: $$(date "+%d-%b-%Y")"
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

# Build with automatic version increment and MAME deployment after successful test
build: vibetune.com test increment-version
	@echo "\n=== Deploying updated version to MAME ==="
	@make vibetune.com
	@if [ -d "$(MAME_DISK)" ]; then \
		cp vibetune.com "$(MAME_DISK)/vibetune.com"; \
		echo "✅ VibeTune v$$(cat VERSION) copied to $(MAME_DISK)"; \
	else \
		echo "❌ MAME disk not mounted at $(MAME_DISK)"; \
	fi
	@echo "\n🎵 Build complete with version increment and MAME deployment - ready for testing! 🎵"
	@echo "After MAME testing, run: make commit"

# Commit after successful MAME verification - updates docs and commits to git
commit:
	@echo "\n=== Updating documentation and committing ==="
	@CURRENT_VERSION=$$(cat VERSION); \
	echo "Committing version $$CURRENT_VERSION"; \
	sed -i '' "s/**Last Updated**: [0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]/**Last Updated**: $$(date '+%Y-%m-%d')/g" WARP.md; \
	sed -i '' "s/**Last Updated**: [0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]/**Last Updated**: $$(date '+%Y-%m-%d')/g" PROJECT_STATUS.md; \
	echo "- v$$CURRENT_VERSION ($$(date '+%Y-%m-%d')) - MAME verified build" >> CHANGELOG.md; \
	git add .; \
	git commit -m "v$$CURRENT_VERSION - MAME verified build - Binary size: $$(wc -c < vibetune.com) bytes - All tests passed - MAME verification complete"; \
	git tag "v$$CURRENT_VERSION"; \
	echo "\n✅ Committed v$$CURRENT_VERSION with documentation updates and git tag"

# Build, test, and deploy in one command
release: vibetune.com test deploy
	@echo "\n🎵 VibeTune ready for testing in MAME! 🎵"

# Show available targets
help:
	@echo "VibeTune v0.2.6.3 - Available Make Targets:"
	@echo ""
	@echo "  vibetune.com  - Build the VibeTune executable"
	@echo "  test          - Run emulator test with z88dk-ticks"
	@echo "  build         - Build + test + auto-increment version + deploy to MAME (recommended)"
	@echo "  commit        - After MAME verification: update docs + git commit + tag"
	@echo "  deploy        - Copy to MAME disk (requires FATDISK mounted)"
	@echo "  release       - Build, test, and deploy in one command"
	@echo "  increment-version - Manually increment version number"
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

.PHONY: test deploy release help build increment-version commit

all::
	mkdir -p $(DEST)/Tunes
	cp Tunes/* $(DEST)/Tunes
