#!/bin/bash
# Move vibetune to local development directory for building

SOURCE_DIR="/mnt/GitHub/vibetune"
DEST_DIR="/mnt/c/Users/miguel/Documents/development/vibetune"

echo "Moving vibetune project to local filesystem..."
echo "Source: $SOURCE_DIR"
echo "Destination: $DEST_DIR"

# Create parent directory if it doesn't exist
mkdir -p "$(dirname "$DEST_DIR")"

# Copy the entire directory
echo "Copying files..."
cp -r "$SOURCE_DIR" "$DEST_DIR"

echo "✅ Project copied to $DEST_DIR"
echo ""
echo "To work there, run:"
echo "  cd $DEST_DIR"
echo ""
echo "When done, run ./move-back.sh to restore to network location"
