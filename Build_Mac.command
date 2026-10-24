#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
fail(){ echo; echo "$1"; read -r -p 'Press Return to close. ' _; exit 1; }
[[ "$(uname -s)" == Darwin ]] || fail 'Run this file on your Mac.'
/usr/bin/xcode-select -p >/dev/null 2>&1 || fail 'Install Apple Command Line Tools first: open Terminal, run xcode-select --install, then run this file again.'
/usr/bin/xcrun --find clang++ >/dev/null 2>&1 || fail 'Apple C++ tools are unavailable. Finish installing Command Line Tools, then try again.'
mkdir -p "$ROOT/.build-tools" "$ROOT/dist"
exec > >(/usr/bin/tee "$ROOT/dist/mac-build.log") 2>&1
trap 'code=$?; if ((code)); then echo "Build stopped. See dist/mac-build.log."; read -r -p "Press Return to close. " _; fi' EXIT
CMAKE="$(command -v cmake || true)"
if [[ -z "$CMAKE" ]]; then
 echo 'Downloading CMake 3.31.6 (verified SHA-256). No Homebrew installation required.'
 ARCHIVE="$ROOT/.build-tools/cmake-3.31.6-macos-universal.tar.gz"
 /usr/bin/curl --fail --location --retry 3 https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-macos-universal.tar.gz -o "$ARCHIVE"
 EXPECTED=330b9514f5112e5ed4fb08b8b05803b776fd9b539a6ae12927d14dcc0ee2ba8d
 ACTUAL="$(/usr/bin/shasum -a 256 "$ARCHIVE" | /usr/bin/awk '{print $1}')"
 [[ "$ACTUAL" == "$EXPECTED" ]] || { echo 'CMake checksum mismatch. Stopping.'; exit 1; }
 /usr/bin/tar -xzf "$ARCHIVE" -C "$ROOT/.build-tools"
 CMAKE="$ROOT/.build-tools/cmake-3.31.6-macos-universal/CMake.app/Contents/bin/cmake"
fi
JOBS="${IRSAUCE_BUILD_JOBS:-2}"
echo 'Building I.R.Sauce for Apple Silicon and Intel. First build downloads JUCE and may take several minutes.'
"$CMAKE" -S "$ROOT" -B "$ROOT/build-mac" -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
"$CMAKE" --build "$ROOT/build-mac" --config Release --parallel "$JOBS"
CTEST="$(dirname "$CMAKE")/ctest"
"$CTEST" --test-dir "$ROOT/build-mac" --output-on-failure -C Release
/bin/bash "$ROOT/scripts/package-mac.sh" "$ROOT/build-mac"
/bin/bash "$ROOT/scripts/create-dmg.sh"
/usr/bin/open -R "$ROOT/dist/IRSauce-0.4.0-Mac-Installer.dmg"
echo 'Done. Open the DMG, then Install I.R.Sauce.pkg to install AU and VST3 plug-ins.'
read -r -p 'Press Return to close. ' _
