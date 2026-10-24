#!/bin/bash
# Wrap a successfully built installer package in a verified, compressed disk image.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION=0.3.0
DIST="$ROOT/dist"
PACKAGE="${1:-$DIST/IRSauce-$VERSION-unsigned.pkg}"
DMG="$DIST/IRSauce-$VERSION-Mac-Installer.dmg"
[[ "$(uname -s)" == Darwin ]] || { echo 'DMG creation requires macOS.' >&2; exit 1; }
[[ -s "$PACKAGE" ]] || { echo "No compiled installer package found: $PACKAGE" >&2; exit 1; }
# Never label an empty image or a source archive as an installer.
PAYLOAD="$(/usr/sbin/pkgutil --payload-files "$PACKAGE")"
for REQUIRED in 'Library/Audio/Plug-Ins/Components/IRSauce.component/Contents/MacOS/IRSauce' 'Library/Audio/Plug-Ins/VST3/IRSauce.vst3/Contents/MacOS/IRSauce'; do
 printf '%s\n' "$PAYLOAD" | /usr/bin/awk -v required="$REQUIRED" '{sub(/^\.\//, ""); if ($0==required) found=1} END {exit !found}' || { echo "Installer is missing executable: $REQUIRED" >&2; exit 1; }
done
WORK="$(mktemp -d "${TMPDIR:-/tmp}/irsauce-dmg.XXXXXX")"
cleanup(){ rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$DIST" "$WORK/content"
/usr/bin/ditto "$PACKAGE" "$WORK/content/Install I.R.Sauce.pkg"
cat > "$WORK/content/START HERE.txt" <<'TXT'
I.R.Sauce — Backyard Smoker Multi-Effects

Double-click Install I.R.Sauce.pkg, follow Apple's installer, then reopen your DAW.

Installs:
AU: /Library/Audio/Plug-Ins/Components/IRSauce.component
VST3: /Library/Audio/Plug-Ins/VST3/IRSauce.vst3

Intel + Apple Silicon. macOS 11 or later is the build target.
AU is for compatible hosts such as Logic; VST3 is for compatible VST3 hosts.
Pro Tools AAX is not included.

This beta package is unsigned and not notarized. Bundles have local ad-hoc signatures.
No system-security settings are changed by this installer.
TXT
/usr/bin/ditto "$ROOT/LICENSE" "$WORK/content/Source License.txt"
/usr/bin/ditto "$ROOT/THIRD_PARTY_NOTICES.md" "$WORK/content/Third Party Notices.md"
if [[ -d "$ROOT/Docs" ]]; then
 /usr/bin/ditto "$ROOT/Docs" "$WORK/content/Guides"
fi
if [[ -f "$ROOT/Preview/IRSauce_Smoker_Native.jpg" ]]; then
 /usr/bin/ditto "$ROOT/Preview/IRSauce_Smoker_Native.jpg" "$WORK/content/I.R.Sauce Preview.jpg"
fi
# Write to a temporary output and publish only after image verification succeeds.
/usr/bin/hdiutil create -ov -volname 'I.R.Sauce Installer' -srcfolder "$WORK/content" -fs HFS+ -format UDZO "$WORK/IRSauce.dmg"
/usr/bin/hdiutil verify "$WORK/IRSauce.dmg"
/usr/bin/ditto "$WORK/IRSauce.dmg" "$DMG"
(cd "$DIST" && /usr/bin/shasum -a 256 "$(basename "$DMG")" > "$(basename "$DMG").sha256")
echo "Created verified disk image: $DMG"
