# Unified jet-restricted J/psi EEC

The active workflow lives in the repository top level. The previous workflow
is preserved under `archive/legacy_eec_workflows_20260910/`. The active files
are:

- `pythia_private.cpp`: produces the private `CharmoniumInfo` ntuples.
- `eec_cms.cpp`: reads `JetsAndDaughters` CMS ntuples (`CmsGen` and/or
  `CmsReco`).
- `eec_private.cpp`: reads private-Pythia `CharmoniumInfo` ntuples
  (`PrivateGen`). It also writes mother-PDG-ID source categories in
  `PrivateGen_cat0` through `PrivateGen_cat7`; every category directory has
  the same histogram schema as the inclusive directory.
- `EECCommon.h`: the only implementation of jet acceptance, J/psi-jet
  association, helicity-frame boost, decay-muon removal, energy weighting, and
  histogram filling.

The comparable histograms have identical names inside the `CmsGen`, `CmsReco`,
and `PrivateGen` ROOT directories. The main ones are
`eec_alljets_all` and `eec_jpsijet_all`. `count_*` histograms are unweighted by
constituent energy and are useful for debugging. Every EEC/count distribution
also has J/psi-pT-binned variants with suffixes such as `_jpsipt_8_12` and
`_jpsipt_200_Inf`.

Private source categories are: cat0 other, cat1 b-hadron (nonprompt), cat2
charmonium feed-down, cat3 gluon/proton, cat4 quark, cat5 `3S1(8)`, cat6
`1S0(8)`, and cat7 `3PJ(8)`. The category mapping uses the direct J/psi mother
PDG ID and is defined in `include/MotherCategory.h`.

## Common operational definition

1. Require the same dimuon mass, eta, and asymmetric muon-pT cuts.
2. Select jets with the same `R`, pT threshold, and eta acceptance.
3. Require both selected decay muons in the same J/psi jet and
   `DeltaR(J/psi, jet) < R`.
4. Exclude the two selected decay muons.
5. Boost every remaining selected-jet constituent to the dimuon rest frame.
6. Fill with weight `eventWeight * E_rest / M_dimuon`.

By default, all constituents in a jet receive the common factor
`correctedJetPt / vectorSumDaughterPt`. This preserves the operational choice
in the old CMS analysis. Set `--constituent-scaling off` to use unmodified
particle/PF-candidate four-vectors. The factor is stored in the
`constituent_scale` histogram.

CMS-only trigger, MET, hot-zone, SoftID, vertex-probability, and prompt-lifetime
requirements cannot be reproduced in private Pythia. For a generator-level
closure/debug comparison, run CMS with `--level gen --cms-filters off`; for the
actual CMS analysis, keep the default filters enabled.

## Build and local examples

```bash
./compile.sh --target all

mkdir -p private_events
./bin/pythia_private \
  --seed 100001 --chunknum 0 --events 1000 \
  --outputdir private_events --oniashower off --crmode 0

./bin/eec_private \
  -i private_events -o ./private_debug -n 1 -e 0 -r 0.4

./bin/eec_cms \
  -i /path/to/cms/dataset -o ./cms_debug -n 1 -e 0 -r 0.4 \
  --level gen --cms-filters off
```

Both commands default to jet pT > 30 GeV, |jet eta| < 5, and leading/subleading
muon pT > 4/3 GeV.

For a quick shape-and-ratio comparison:

```bash
root -l -q 'compare.C("private.root","PrivateGen","cms.root","CmsGen")'
```

For the full CMS-sample, private-sample, and OniaOn-origin comparisons:

```bash
./compile.sh --target plot
./bin/plot_eec --output-dir plots_eec
```

For data sideband subtraction and comparison of the extracted signal with CMS
MC, use `plot_sideband_subtraction`. Its defaults are the 2.9--3.3 GeV data
window and the 2.7--2.9/3.3--3.5 GeV sidebands under the standard EOS output
directories:

```bash
./compile.sh --target plot
./bin/plot_sideband_subtraction --output-dir plots_sideband_subtraction
```

The raw background estimate is scaled by
`signal_width / (lower_width + upper_width)` before it is subtracted. With the
default window widths this factor is one. All raw inputs, the background
estimate, and the extracted signal are also written to
`plots_sideband_subtraction/sideband_subtraction.root`.

The CMS Hard-QCD nominal and ext1 histograms are added before shape
normalization. The private OniaOn origin comparison combines cat5+cat6+cat7 as
one octet curve. Inclusive and all J/psi-pT bins are plotted, with optional
positive-coschi versions (disable with `--positive-half off`).

## Condor

Generate submit files without submitting:

```bash
./condor.sh --mode generate --chunks 4000 --events 500 \
  --output-dir /eos/path/to/private_events \
  --oniashower off --crmode 0 --jobtag OniaOff_CR0

./condor.sh --mode cms --level both
./condor.sh --mode private
```

Use `--dataset NAME` one or more times to override the built-in dataset list,
and add `--submit` after inspecting the generated `.sub` files. All physics
parameters accepted by `condor.sh` are forwarded identically to both adapters.
The submit script compiles the requested executable once on the submit host;
workers run the prebuilt AFS binary and do not initialize or compile CMSSW.
Generation mode creates deterministic, unique Pythia seeds beginning at
`--seed-start` (default `100000000`). `pythia_private.cpp` also stores its full
generation setup in the ROOT `generation_config` object.

Merge every discovered dataset after all jobs have completed:

```bash
./merge.sh --mode private --cleanup
./merge.sh --mode cms --cleanup
```

This matches `${OUTPUT_DIR}/${DATASET}_Chunk*.root` and writes one merged file
per dataset as `${OUTPUT_DIR}/${DATASET}.root`. `--cleanup` removes the matched
chunk files only after every requested merge and ROOT-file validation succeeds.
No expected chunk count is needed; optionally use `--expected-chunks N` when a
strict completeness check is wanted. Different generator settings, data, and
MC remain separate by default. Use `--dataset NAME` to restrict the merge, or
`--combine-output all.root` when combining distinct datasets is really
intended. The old `./merge.sh OUTPUT_DIR DATASET [...]` syntax remains supported.
