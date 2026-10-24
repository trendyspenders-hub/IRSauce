#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build-mac}"
VERSION="0.3.0"
DIST="$ROOT/dist"
STAGE="$(mktemp -d "${TMPDIR:-/tmp}/irsauce-pkg.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT
if [[ "$(uname -s)" != Darwin ]]; then echo 'This packaging script requires macOS.' >&2; exit 1; fi
mkdir -p "$DIST" "$STAGE/Library/Audio/Plug-Ins/Components" "$STAGE/Library/Audio/Plug-Ins/VST3"
ARTEFACTS="$BUILD_DIR/IRSauce_artefacts/Release"
for SPEC in 'AU|IRSauce.component|Library/Audio/Plug-Ins/Components' 'VST3|IRSauce.vst3|Library/Audio/Plug-Ins/VST3'; do
 IFS='|' read -r FORMAT BUNDLE DEST <<< "$SPEC"
 SOURCE="$ARTEFACTS/$FORMAT/$BUNDLE"
 [[ -d "$SOURCE" ]] || { echo "Missing built bundle: $SOURCE" >&2; exit 1; }
 EXECUTABLE="$SOURCE/Contents/MacOS/IRSauce"
 [[ -f "$EXECUTABLE" ]] || { echo "Missing executable: $EXECUTABLE" >&2; exit 1; }
 /usr/bin/lipo "$EXECUTABLE" -verify_arch arm64 x86_64
 /usr/bin/ditto "$SOURCE" "$STAGE/$DEST/$BUNDLE"
 NOTICE_DIR="$STAGE/$DEST/$BUNDLE/Contents/Resources/Licenses"
 mkdir -p "$NOTICE_DIR"
 /usr/bin/ditto "$ROOT/LICENSE" "$NOTICE_DIR/AGPL-3.0.txt"
 /usr/bin/ditto "$ROOT/THIRD_PARTY_NOTICES.md" "$NOTICE_DIR/THIRD_PARTY_NOTICES.md"
 /usr/bin/ditto "$ROOT/vendor/signalsmith-stretch/LICENSE.txt" "$NOTICE_DIR/Signalsmith-Stretch-MIT.txt"
 /usr/bin/ditto "$ROOT/vendor/signalsmith-stretch/include/signalsmith-linear/LICENSE.txt" "$NOTICE_DIR/Signalsmith-Linear-MIT.txt"
 /usr/bin/codesign --force --deep --sign - "$STAGE/$DEST/$BUNDLE"
 /usr/bin/codesign --verify --deep --strict "$STAGE/$DEST/$BUNDLE"
done
/usr/bin/pkgbuild --root "$STAGE" --identifier audio.ir.sauce --version "$VERSION" --install-location / --ownership recommended "$DIST/IRSauce-$VERSION-unsigned.pkg"
/usr/sbin/pkgutil --payload-files "$DIST/IRSauce-$VERSION-unsigned.pkg" > "$DIST/package-contents.txt"
/usr/bin/shasum -a 256 "$DIST/IRSauce-$VERSION-unsigned.pkg" > "$DIST/IRSauce-$VERSION-unsigned.pkg.sha256"
echo "Created: $DIST/IRSauce-$VERSION-unsigned.pkg"
echo 'Package is unsigned; bundles use local ad-hoc signing, not Developer ID or notarization.'
