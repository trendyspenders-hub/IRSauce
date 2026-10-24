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

Linux x86_64 VST3 and standalone: COMPLETE build and link passed. VST3 manifest generated successfully. Both CTest suites pass (2/2). Native editor rendered and inspected. This is not a Linux DAW host test.
macOS Intel/Apple Silicon AU/VST3: NOT COMPILED in this session.
Mac .pkg/DMG: GitHub build is being submitted; final status is reported separately.
GitHub Mac workflow and auval: supplied; NOT RUN.
Developer ID signing/notarization: NOT PERFORMED. Ad-hoc signing is part of the Mac packaging script.

## Material limits

This is a beta, not a radio/mastering certification or a listening-panel evaluation. No Mac DAW loading tests, host automation stress suite, pluginval, Apple auval, CPU profiling, long-session leak testing or listening assessment has been completed. VST3/AU support is a build target until validated on each platform.

Pitch processing always runs and reports 5760 samples at 48 kHz with the current library preset (120 ms). This makes the beta better suited to mixing than live low-latency monitoring. Other sample rates have their own reported latency.

Only Signalsmith is integrated from the proposed six-project combination; other sections use original implementations. EQ frequency bands are fixed. No dynamic EQ, true-peak limiting, convolution IR loader, multivoice choir engine, draggable routing or external preset saving.

The interface animation is lightweight vector overlay on a static generated scene. It does not animate the food or every wisp baked into the background. Metering continues while motion is disabled. The recipe dropdown is a starting-point selector, not a name-synchronized preset manager.

Release scope updated: AU/VST3 only on Mac; VST3 only on Windows. Standalone target removed at user request.
