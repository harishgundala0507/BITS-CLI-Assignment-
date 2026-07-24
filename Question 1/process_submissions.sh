#!/bin/bash

SUBMISSION_DIR="./submissions"
BACKUP_DIR="./backup"
REPORT_FILE="./report.txt"
ERROR_LOG="./error.log"
DUPLICATE_DIR="./duplicates"

> "$ERROR_LOG"
> "$REPORT_FILE"

mkdir -p "$BACKUP_DIR" "$DUPLICATE_DIR" 2>>"$ERROR_LOG"

echo "Duplicate Submission Report - $(date)" | tee -a "$REPORT_FILE"
echo "----------------------------------------" | tee -a "$REPORT_FILE"

if [ ! -d "$SUBMISSION_DIR" ]; then
    echo "Error: Submission directory not found" | tee -a "$ERROR_LOG"
    exit 1
fi

TOTAL_FILES=0
for f in "$SUBMISSION_DIR"/*; do
    [ -f "$f" ] && TOTAL_FILES=$((TOTAL_FILES + 1))
done

echo "Total files found: $TOTAL_FILES" | tee -a "$REPORT_FILE"

DUPLICATE_COUNT=0
BACKUP_COUNT=0
declare -A seen_checksums

for file in "$SUBMISSION_DIR"/*; do
    [ -f "$file" ] || continue
    filename=$(basename "$file")
    checksum=$(md5sum "$file" 2>>"$ERROR_LOG" | awk '{print $1}')

    if [ -z "$checksum" ]; then
        echo "Error: Could not checksum $file" >> "$ERROR_LOG"
        continue
    fi

    if [ "${seen_checksums[$checksum]}" = "1" ]; then
        DUPLICATE_COUNT=$((DUPLICATE_COUNT + 1))
        cp "$file" "$DUPLICATE_DIR/${filename}.dup.$DUPLICATE_COUNT" 2>>"$ERROR_LOG"
        echo "DUPLICATE: $file matches another submission" | tee -a "$REPORT_FILE"
    else
        seen_checksums[$checksum]=1
        BACKUP_COUNT=$((BACKUP_COUNT + 1))
        cp "$file" "$BACKUP_DIR/$filename" 2>>"$ERROR_LOG"
    fi
done

echo "" | tee -a "$REPORT_FILE"
echo "SUMMARY" | tee -a "$REPORT_FILE"
echo "  Files processed  : $TOTAL_FILES" | tee -a "$REPORT_FILE"
echo "  Duplicates found : $DUPLICATE_COUNT" | tee -a "$REPORT_FILE"
echo "  Files backed up  : $BACKUP_COUNT" | tee -a "$REPORT_FILE"
echo "  Errors logged    : $(wc -l < "$ERROR_LOG")" | tee -a "$REPORT_FILE"
echo "Report: $REPORT_FILE | Errors: $ERROR_LOG"