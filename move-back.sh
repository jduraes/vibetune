#!/bin/bash
# Move vibetune back from local development directory to network location

SOURCE_DIR="/mnt/c/Users/miguel/Documents/development/vibetune"
DEST_DIR="/mnt/GitHub/vibetune"

echo "Moving vibetune project back to network location..."
echo "Source: $SOURCE_DIR"
echo "Destination: $DEST_DIR"

if [ ! -d "$SOURCE_DIR" ]; then
    echo "❌ Error: Source directory $SOURCE_DIR does not exist"
    echo "   Nothing to move back"
    exit 1
fi

# Sync all files back (rsync preserves modifications)
echo "Syncing files back..."
rsync -av --delete "$SOURCE_DIR/" "$DEST_DIR/"

echo "✅ Project synced back to $DEST_DIR"
echo ""
echo "You can now safely delete the local copy:"
echo "  rm -rf $SOURCE_DIR"
