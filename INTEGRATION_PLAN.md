# Upstream integration status

This beta has one integrated upstream engine: Signalsmith Stretch, plus Signalsmith Linear. All visible controls have working processing implementations; those implementations are not substitutes claimed to be the named upstream products.

| Proposed source | Status |
|---|---|
| Airwindows colour effects | Not ported; original ADAA saturation currently used |
| ZL Equalizer | Not ported; original three-band biquad EQ currently used |
| ZL Compressor | Not ported; original stereo-linked compressor currently used |
| Signalsmith Stretch | Integrated; one pitch-shifted layer |
| Dragonfly Reverb | Not ported; original comb/allpass stereo reverb currently used |
| ChowMultiTool | Not ported; original input trim currently used |

Porting the remaining projects requires individual DSP adapters, dependency/license inspection, upstream equivalence tests, CPU/latency work, and compatible GPL/AGPL release licensing before distribution. No claim of sonic equivalence or endorsement is made. See UPSTREAM.md for vendored pins.
