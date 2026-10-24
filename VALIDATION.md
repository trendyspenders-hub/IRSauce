# I.R.Sauce Smoker Beta 0.3 — validation

## Passed in this environment

- DSP core compiled with GCC 13/C++17.
- Core tests at 22.05, 44.1, 48, 96 and 192 kHz: default unity, EQ boost/cut responses, compressor gain reduction, stereo linking, rapid automation, bounded finite output, tail processing and full reset.
- Actual vendored Signalsmith pitch processing runs at all five rates without nonfinite output.
- Native processor compiled and linked with JUCE 8.0.6.
- Processor runtime tests at 44.1/48/96 kHz: buffers larger than the advertised block size; zero-length blocks; default enabled safety limiter; reset clears tails; dry/default/bypass latency null comparisons; stereo channel polarity preservation; parameter and animation-preference state round-trip.
- Actual native editor rendered to PNG and visually inspected. No generated lettering remains in the background; all labels, values, brand/bottle text, knobs, switches and meter are native code.
- Mac shell scripts pass `bash -n` syntax checks.

## Build status

Mac universal AU/VST3: compiled successfully; 2/2 CTest suites passed on Apple Silicon. Both arm64 and x86_64 slices verified. Architecture-check argument order was corrected after the first packaging attempt.
Mac package/DMG: built successfully; package payload, ad-hoc signatures, universal architectures and hdiutil checksums verified.
AU validation: PASSED after refreshing AudioComponentRegistrar. The prior failure was component discovery before refresh.
Windows x64 VST3: build and both CTest suites PASSED.
Successful installer workflow: https://github.com/trendyspenders-hub/IRSauce/actions/runs/40122177636
Audio build/test workflow: https://github.com/trendyspenders-hub/IRSauce/actions/runs/40121644137
Developer ID signing/notarization: NOT PERFORMED. Bundles are ad-hoc signed; the installer is unsigned.

## Material limits

This is a beta, not a radio/mastering certification or a listening-panel evaluation. Apple auval passed. No representative Mac DAW session tests, host automation stress suite, pluginval, CPU profiling, long-session leak testing or listening assessment has been completed. VST3/AU support is a build target until validated on each platform.

Pitch processing always runs and reports 5760 samples at 48 kHz with the current library preset (120 ms). This makes the beta better suited to mixing than live low-latency monitoring. Other sample rates have their own reported latency.

Only Signalsmith is integrated from the proposed six-project combination; other sections use original implementations. EQ frequency bands are fixed. No dynamic EQ, true-peak limiting, convolution IR loader, multivoice choir engine, draggable routing or external preset saving.

The interface animation is lightweight vector overlay on a static generated scene. It does not animate the food or every wisp baked into the background. Metering continues while motion is disabled. The recipe dropdown is a starting-point selector, not a name-synchronized preset manager.

Release scope updated: AU/VST3 only on Mac; VST3 only on Windows. Standalone target removed at user request.
