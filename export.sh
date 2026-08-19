#!/bin/bash

# vLaunchSELF Export Script
# Exports the complete ps3-filebrowser project to shared folder
# Author: VTSTech
# Date: 2024-12-10

# Configuration
SOURCE_DIR="/home/vtstech/workspace/ps3-filebrowser"
DEST_DIR="/media/sf_!VBoxShared/ps3-filebrowser"
LOG_FILE="/home/vtstech/workspace/ps3-filebrowser/export.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo -e "$1" | tee -a "$LOG_FILE"
}

log "${BLUE}=========================================${NC}"
log "${BLUE}vLaunchSELF Export Script${NC}"
log "${BLUE}=========================================${NC}"
log "${GREEN}Starting export at: $(date)${NC}"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    log "${RED}ERROR: Source directory not found: $SOURCE_DIR${NC}"
    exit 1
fi

# Check if destination directory exists
if [ ! -d "$DEST_DIR" ]; then
    log "${YELLOW}WARNING: Destination directory not found: $DEST_DIR${NC}"
    log "${YELLOW}Attempting to create destination directory...${NC}"
    mkdir -p "$DEST_DIR"
    if [ $? -ne 0 ]; then
        log "${RED}ERROR: Failed to create destination directory: $DEST_DIR${NC}"
        exit 1
    fi
    log "${GREEN}Successfully created destination directory: $DEST_DIR${NC}"
fi

# Erase contents of destination directory
log "${YELLOW}Erasing contents of destination directory: $DEST_DIR${NC}"
rm -rf "$DEST_DIR"/*

if [ $? -ne 0 ]; then
    log "${RED}ERROR: Failed to erase destination directory contents${NC}"
    exit 1
fi

log "${GREEN}Successfully erased destination directory contents${NC}"

# Create destination directory structure
mkdir -p "$DEST_DIR"
if [ $? -ne 0 ]; then
    log "${RED}ERROR: Failed to create destination directory${NC}"
    exit 1
fi

# Copy all files from source to destination
log "${BLUE}Copying files from $SOURCE_DIR to $DEST_DIR${NC}"
log "${BLUE}=========================================${NC}"

# Use rsync for efficient copying with progress
rsync -avh --progress "$SOURCE_DIR/" "$DEST_DIR/" 2>&1 | tee -a "$LOG_FILE"

if [ $? -ne 0 ]; then
    log "${RED}ERROR: Failed to copy files${NC}"
    exit 1
fi

log "${GREEN}=========================================${NC}"
log "${GREEN}Export completed successfully!${NC}"

# Set appropriate permissions
log "${BLUE}Setting permissions...${NC}"
chmod -R 755 "$DEST_DIR"
find "$DEST_DIR" -name "*.sh" -exec chmod +x {} \;
find "$DEST_DIR" -name "*.elf" -exec chmod +x {} \;
find "$DEST_DIR" -name "*.self" -exec chmod +x {} \;

# Verify the export
log "${BLUE}Verifying export...${NC}"
SOURCE_FILE_COUNT=$(find "$SOURCE_DIR" -type f | wc -l)
DEST_FILE_COUNT=$(find "$DEST_DIR" -type f | wc -l)

log "${GREEN}Source files: $SOURCE_FILE_COUNT${NC}"
log "${GREEN}Destination files: $DEST_FILE_COUNT${NC}"

if [ "$SOURCE_FILE_COUNT" -eq "$DEST_FILE_COUNT" ]; then
    log "${GREEN}✓ File count matches - export verified${NC}"
else
    log "${YELLOW}⚠ File count mismatch - manual verification recommended${NC}"
fi

# List important files
log "${BLUE}Important files in destination:${NC}"
ls -la "$DEST_DIR/" | grep -E "\.(elf|self|pkg|sh|md|c|h)$" | tee -a "$LOG_FILE"

log "${BLUE}=========================================${NC}"
log "${GREEN}Export completed at: $(date)${NC}"
log "${BLUE}Destination: $DEST_DIR${NC}"

# Display final status
echo ""
echo -e "${GREEN}✅ Export completed successfully!${NC}"
echo -e "${BLUE}📁 Destination: $DEST_DIR${NC}"
echo -e "${BLUE}📄 Log file: $LOG_FILE${NC}"
echo ""