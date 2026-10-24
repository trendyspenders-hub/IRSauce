# I.R.Sauce — Backyard Smoker Beta 0.3

Native stereo DAW plug-in for macOS AU/VST3 and Windows VST3. The entire interface is an actual backyard smoker scene. All names, readouts, controls, bottle lettering and meter markings are real interface code over a text-free generated background.

## Mac installer

A compiled universal Mac DMG is available from the successful installer workflow:
https://github.com/trendyspenders-hub/IRSauce/actions/runs/40122177636

Open the DMG, then Install I.R.Sauce.pkg. It installs AU and VST3 system-wide. There is no standalone application and no AAX/Pro Tools target. Restart the DAW and rescan. The package is unsigned, plug-in bundles use ad-hoc signatures, and Apple notarization has not been performed.

The universal AU/VST3 build and both CTest suites passed on Mac. Apple's auval passed after refreshing registration. Package payload/architecture/signature checks and DMG hdiutil verification passed. Intel slices are present but Intel runtime/DAW tests remain separate work. Windows x64 VST3 build and CTest also passed.

For local source builds, run Create_Mac_Installer.command on a Mac with Apple Command Line Tools. Branded installation, user and audit PDFs are in Docs/.

## Audio controls

| Smoker section | Function |
|---|---|
| Prep Station | Input trim, -24 to +24 dB |
| The Rub | Low shelf at 140 Hz, mid bell at 1.2 kHz, high shelf at 6.5 kHz; each ±12 dB |
| Firebox | Smoothed nonlinear saturation with first-order antiderivative antialiasing |
| Pitmaster | Stereo-linked soft-knee compression, amount (ratio 1:1 to 8:1), threshold |
| Double Stack | Actual Signalsmith Stretch pitch shift, ±12 semitones, and layer blend |
| Smokehouse | Stereo damped comb/allpass reverb, size and mix |
| Sauce Mix | Global dry/wet with pitch-latency-aligned dry path |
| Output | Output gain, -24 to +12 dB; safety limiter enabled by default |

The safety limiter is a stereo-linked sample-peak attenuator with instantaneous attack and 100 ms release, ceiling 0.98 linear (about -0.18 dBFS). It is not an intersample true-peak limiter. Global bypass returns the delayed dry input without output gain or limiting. A bypassed input above full scale remains above full scale.

Advanced opens the full parameter editor, including compression attack, release, makeup gain, reverb damping, safety limiter and all module switches. Double-click a knob to reset it. Drag to adjust; hover or drag to see precise values. The upper recipe selector provides five starting sounds. A chosen recipe is a starting point: the name is not updated or recalled as a saved user preset; actual parameter values are saved in the DAW session.

Signalsmith stays active at zero layer mix to preserve timing. The plug-in reports its fixed latency to the host. Module bypass crossfades; the internal reverb continues running while bypassed to avoid abruptly erasing its history. No AI singing, choir synthesis, source separation, sample playback, movable routing or parallel racks are claimed.

## Graphics

The thermometer is an output level meter. Firebox ember animation follows Drive; chimney wisps follow Reverb Mix. They do not represent CPU temperature. Animation can be disabled; its preference is stored in the DAW session. The UI timer remains active for metering. Background food and coals are static imagery; ember particles and extra wisps move. Resizing preserves the scene's aspect ratio.

## Engine provenance

Signalsmith Stretch and Linear are integrated and vendored with MIT notices. The EQ, dynamics, saturation and reverb are original project implementations of standard DSP approaches. Airwindows, Dragonfly, ChowMultiTool and ZL code are **not integrated** in this beta. See INTEGRATION_PLAN.md for that separate work.

Original source is AGPL-3.0-or-later. JUCE is fetched at tag 8.0.6 and is used through its AGPLv3 open-source license route. The generated smoker artwork is included as the project skin. Upstream trademarks, presets, sample libraries and logos are not included.

## Build manually

Requires C++17, Git, CMake 3.22+ and platform development tools.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

On Mac add `'-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0` when configuring. On Windows use Visual Studio 2022 and add `-A x64`. Output is under `build/IRSauce_artefacts/Release/`.

Earlier Linux x86_64 VST3 and development-only standalone builds complete successfully, and both CTest suites pass. See VALIDATION.md for the checks actually completed and material limitations.

## Plug-in-only product
The release intentionally contains no standalone application. Load IRSauce on a stereo audio track or bus in a compatible DAW. Mac package: AU + VST3. Windows: VST3. AAX and Pro Tools are not included.
