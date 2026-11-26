#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$PREFIX/bin"

cat > "$PREFIX/bin/compile" <<EOF
#!/data/data/com.termux/files/usr/bin/bash
exec "$PROJECT_DIR/scripts/compile.sh" "\$@"
EOF

cat > "$PREFIX/bin/run" <<EOF
#!/data/data/com.termux/files/usr/bin/bash
exec "$PROJECT_DIR/scripts/run.sh" "\$@"
EOF

chmod +x "$PREFIX/bin/compile" "$PREFIX/bin/run"
chmod +x "$PROJECT_DIR/scripts/"*.sh
echo "Installed commands:"
echo "  compile"
echo "  run"
echo "Project: $PROJECT_DIR"
