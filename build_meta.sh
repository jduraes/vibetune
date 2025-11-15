#!/bin/bash
# VibeTune Build Metadata - Single Source of Truth

VERSION=$(cat VERSION)
BUILD_DATE=$(date "+%d-%b-%Y")

# Generate assembly include file
cat > src/build_meta.inc << ASMEOF
;===============================================================================
; VibeTune Build Metadata - Auto-generated, do not edit manually
; Generated: $(date "+%Y-%m-%d %H:%M:%S")
;===============================================================================

; Version string for banner
VERSION_STR: .DB "VibeTune v${VERSION} for RomWBW, ${BUILD_DATE}",0
ASMEOF

echo "✅ Generated src/build_meta.inc with version ${VERSION} and date ${BUILD_DATE}"
