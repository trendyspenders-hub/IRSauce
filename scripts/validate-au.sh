#!/bin/bash
# Run on a clean CI runner; refresh AU registration after placing the bundle.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"
DEST="$HOME/Library/Audio/Plug-Ins/Components/IRSauce.component"
mkdir -p "$(dirname "$DEST")" "$ROOT/dist"
/usr/bin/ditto "$BUILD_DIR/IRSauce_artefacts/Release/AU/IRSauce.component" "$DEST"
chmod +x "$DEST/Contents/MacOS/IRSauce"
/usr/bin/codesign --force --deep --sign - "$DEST"
/usr/bin/codesign --verify --deep --strict "$DEST"
# AudioComponentRegistrar may have cached its inventory before the new AU existed.
/usr/bin/killall AudioComponentRegistrar 2>/dev/null || true
for attempt in 1 2 3; do
 /usr/bin/auval -a > "$ROOT/dist/au-inventory.log" 2>&1 || true
 if /usr/bin/auval -v aufx IrSc IrAu > "$ROOT/dist/auval.log" 2>&1; then
  cat "$ROOT/dist/auval.log"
  exit 0
 fi
 # A real validation failure must not be hidden by registration retries.
 if ! grep -q "didn't find the component" "$ROOT/dist/auval.log"; then
  cat "$ROOT/dist/auval.log"; exit 1
 fi
 sleep 3
done
cat "$ROOT/dist/au-inventory.log"
cat "$ROOT/dist/auval.log"
exit 1
