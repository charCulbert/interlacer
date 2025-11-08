#!/bin/bash

# Simple interactive script to run interlacer
# No dependencies - uses built-in macOS tools

cd "$(dirname "$0")"

# Remove quarantine flags and make executable (first run setup)
xattr -d com.apple.quarantine ./interlacer 2>/dev/null
chmod +x ./interlacer 2>/dev/null

echo ""
echo "┌─────────────────────────┐"
echo "│   INTERLACER LAUNCHER   │"
echo "└─────────────────────────┘"
echo ""

# Function to select a file
select_file() {
    local prompt="$1"
    local file=$(osascript -e "POSIX path of (choose file with prompt \"$prompt\" of type {\"public.tiff\"})")
    echo "$file"
}

# Get input files
echo "Select the first TIFF file..."
FILE1=$(select_file "Select first TIFF image")
if [ -z "$FILE1" ]; then
    echo "Cancelled."
    exit 0
fi

echo "Selected: $FILE1"
echo ""

echo "Select the second TIFF file..."
FILE2=$(select_file "Select second TIFF image")
if [ -z "$FILE2" ]; then
    echo "Cancelled."
    exit 0
fi

echo "Selected: $FILE2"
echo ""

# Ask for interlacing intervals
echo "Interlacing options:"
echo "(Enter 0 to disable that axis, press Enter for defaults)"
echo ""
read -p "Interlace every X rows [default: 0]: " ROW_INT
read -p "Interlace every X columns [default: 0]: " COL_INT

# Use defaults if empty
ROW_INT=${ROW_INT:-0}
COL_INT=${COL_INT:-0}

ARGS="-r $ROW_INT -c $COL_INT"

# Ask for output location
OUTPUT=$(osascript -e 'POSIX path of (choose file name with prompt "Save output as:" default name "interlaced_output.tif")' 2>/dev/null)
if [ -z "$OUTPUT" ]; then
    OUTPUT="interlacedOutput.tif"
    echo "Using default output: $OUTPUT"
else
    ARGS="$ARGS -o \"$OUTPUT\""
fi

echo ""
echo "Running interlacer..."
echo ""

# Run the program
eval ./interlacer \"$FILE1\" \"$FILE2\" $ARGS

echo ""
read -p "Press Enter to close..."
