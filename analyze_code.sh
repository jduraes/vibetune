#!/bin/bash

# VibeTune Code Analysis Script
# This script analyzes the structure of vibetune.asm without modifying it

echo "🔍 VibeTune Code Structure Analysis"
echo "=================================="
echo ""

echo "📊 Basic Statistics:"
echo "Total lines: $(wc -l < vibetune.asm)"
echo "Code lines (non-comment): $(grep -v '^;' vibetune.asm | grep -v '^$' | wc -l)"
echo "Comment lines: $(grep '^;' vibetune.asm | wc -l)"
echo "Binary size: $(wc -c < vibetune.com 2>/dev/null || echo 'N/A') bytes"
echo ""

echo "🏗️ Major Sections (by comment headers):"
grep -n "^;===============================================================================" vibetune.asm | while read line; do
    line_num=$(echo "$line" | cut -d: -f1)
    next_line=$((line_num + 1))
    section_name=$(sed -n "${next_line}p" vibetune.asm | sed 's/^; //')
    echo "Line $line_num: $section_name"
done
echo ""

echo "🎵 Audio Engine Components:"
grep -n -i "player\|ptx\|mym\|pt2\|pt3" vibetune.asm | head -10
echo ""

echo "🔧 Hardware Related:"
grep -n -i "hardware\|sound\|chip\|port\|detect" vibetune.asm | head -10
echo ""

echo "📁 File I/O Operations:"
grep -n -i "file\|load\|read\|fcb\|bdos" vibetune.asm | head -10
echo ""

echo "🖥️ User Interface:"
grep -n -i "message\|banner\|error\|usage" vibetune.asm | head -10
echo ""

echo "✅ Analysis complete! Review sections for modularization planning."