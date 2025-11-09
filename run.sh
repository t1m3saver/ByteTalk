#!/bin/bash
set -e

VERSION="v0.1.0"
ROOT_DIR=${ROOT_DIR:-$(pwd)}
BUILD_DIR=${BUILD_DIR:-"$ROOT_DIR/build"}
EXECUTABLE=${EXECUTABLE:-"$BUILD_DIR/ByteTalk/ByteTalk"}
LOG_FILE=${LOG_FILE:-"$ROOT_DIR/app.log"}
export LOG_PATH=$LOG_FILE

check_executable() {
    if [ ! -f "$EXECUTABLE" ]; then
        echo "Error: Executable $EXECUTABLE not found!"
        exit 1
    fi
}

start_foreground() {
    stop_process
    echo "Starting $EXECUTABLE in foreground..."
    "$EXECUTABLE"
}

start_background() {
    stop_process
    echo "Starting $EXECUTABLE in background..."
    nohup "$EXECUTABLE" > "$LOG_FILE" 2>&1 &
    echo "Process started. Logs are in $LOG_FILE"
}

stop_process() {
    pids=$(pgrep -f "$EXECUTABLE" || true)
    if [ -n "$pids" ]; then
        echo "Stopping processes: $pids..."
        for pid in $pids; do
            kill "$pid"
            while kill -0 "$pid" 2>/dev/null; do
                sleep 0.1
            done
        done
        echo "All stopped."
    else
        echo "No running process found for $EXECUTABLE"
    fi
}

status_process() {
    local pid
    pid=$(pgrep -f "$EXECUTABLE" || true)
    if [ -n "$pid" ]; then
        echo "Process is running with PID $pid"
    else
        echo "Process is not running"
    fi
}

print_logo() {
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    MAGENTA='\033[0;35m'
    CYAN='\033[0;36m'
    BOLD='\033[1m'
    RESET='\033[0m'

    echo -e "${CYAN}------------------------------------------------------------${RESET}"
    echo -e "${BOLD}${CYAN}           Run ByteTalk ${VERSION}${RESET}"
    echo -e "${CYAN}------------------------------------------------------------${RESET}"
}

main() {
    print_logo

    check_executable

    case "$1" in
        -daemon)
            start_background
            ;;
        -stop)
            stop_process
            ;;
        -status)
            status_process
            ;;
        *)
            start_foreground
            ;;
    esac
}

main "$1"
