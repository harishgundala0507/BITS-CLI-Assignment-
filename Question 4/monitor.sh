#!/bin/bash

# Real-time log monitoring tool
# Displays new log entries, extracts ERRORs, maintains a report, suppresses noise

LOG_FILE="${1:-/var/log/syslog}"
REPORT_FILE="${2:-error_report.txt}"
PID_FILE="/tmp/log_monitor.pid"

show_help() {
    echo "Usage: $0 [log_file] [report_file]"
    echo "  log_file    - file to monitor (default: /var/log/syslog)"
    echo "  report_file - where to save ERROR messages (default: error_report.txt)"
    exit 0
}

[ "$1" = "-h" ] || [ "$1" = "--help" ] && show_help

> "$REPORT_FILE"

cleanup() {
    echo "Stopping monitor..."
    rm -f "$PID_FILE"
    exit 0
}
trap cleanup SIGINT SIGTERM

if [ ! -f "$LOG_FILE" ]; then
    echo "Error: $LOG_FILE not found"
    echo "Using sample.log instead (run: echo 'test ERROR' > sample.log)"
    exit 1
fi

echo "Monitoring: $LOG_FILE"
echo "Errors ->  $REPORT_FILE"
echo "PID: $$"
echo "$$" > "$PID_FILE"

# Pipeline: tail new lines -> filter ERRORs -> save + display
tail -f "$LOG_FILE" 2>/dev/null | while read line; do
    if echo "$line" | grep -q "ERROR"; then
        echo "$line" | tee -a "$REPORT_FILE"
    fi
done