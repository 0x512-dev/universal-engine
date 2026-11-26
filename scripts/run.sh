#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$PROJECT_DIR/build/universal_engine"

if [ ! -x "$BIN" ]; then
    echo "Executable not found. Building first..."
    "$PROJECT_DIR/scripts/compile.sh"
fi

export DISPLAY=:1
export SDL_VIDEODRIVER=x11
export XDG_RUNTIME_DIR="${TMPDIR:-$PREFIX/tmp}"
mkdir -p "$XDG_RUNTIME_DIR"

echo "[1/3] Stopping old Termux:X11 server (if any)..."
pkill -x termux-x11 2>/dev/null || true
sleep 0.5

echo "[2/3] Starting Termux:X11 on :1..."
termux-x11 :1 >/dev/null 2>&1 &
X11_PID=$!
sleep 1

echo "[3/3] Opening Termux:X11 activity..."
am start --user 0 -n com.termux.x11/com.termux.x11.MainActivity >/dev/null 2>&1 || true
sleep 1

if ! kill -0 "$X11_PID" 2>/dev/null; then
    echo "Warning: termux-x11 process exited. Check the Termux:X11 app/add-on installation."
fi

cd "$PROJECT_DIR"
exec "$BIN"
