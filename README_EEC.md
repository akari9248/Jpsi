# Unified jet-restricted J/psi EEC

The active workflow lives in the repository top level. The previous workflow
is preserved under `archive/legacy_eec_workflows_20260910/`. The active files
are:

- `pythia_private.cpp`: produces the private `CharmoniumInfo` ntuples.
- `eec_cms.cpp`: reads `JetsAndDaughters` CMS ntuples (`CmsGen` and/or
  `CmsReco`).
- `eec_private.cpp`: reads private-Pythia `CharmoniumInfo` ntuples
  (`PrivateGen`). It also writes the retained mother-PDG-ID source categories
  and private feed-down tag outputs with the same histogram schema as the
  inclusive directory.
- `EECCommon.h`: the only implementation of jet acceptance, J/psi-jet
  association, helicity-frame boost, decay-muon removal, energy weighting, and
  histogram filling.

The comparable histograms have identical names inside the `CmsGen`, `CmsReco`,
and `PrivateGen` ROOT directories. The main ones are
`eec_alljets_all` and `eec_jpsijet_all`. Every EEC distribution also has
J/psi-pT-binned variants with suffixes such as `_jpsipt_8_12` and
`_jpsipt_200_Inf`. The redundant `count_*` companion histograms are not stored.

The J/psi-in-jet momentum fractions are stored separately as `z_pt` and `z_h`.
The former is `pT(J/psi) / pT(jet)`. The latter follows arXiv:1702.03287,
`z_h = p_J/psi^+ / p_jet^+`, where `p^+ = E + p_parallel` and the longitudinal
axis points along the jet. Both observables also have jet-pT-binned variants:
30-50, 50-100, 100-150, 150-200, 200-300, 300-400, 400-600, and >=600 GeV.

The paper's jet fragmentation function is also available in the three jet-pT
bins 50--100, 100--150, and 150--200 GeV. During event processing the code
writes merge-safe ingredients named
`fragmentation_numerator_jetpt_*` and
`inclusive_jet_denominator_jetpt_*`. After `hadd`,
`make_fragmentation_function` creates
`fragmentation_function_jetpt_*`, defined as

```text
F(z_h,pT) = (1 / N_inclusive-jet) dN_J/psi-jet / dz_h .
```

`merge.sh` performs this final step automatically. For a single unmerged ROOT
file, run `./bin/make_fragmentation_function FILE.root`. Keeping the ratio out
of chunk files is essential because ratios cannot themselves be added by
`hadd`.

The denominator is filled before J/psi-candidate cuts in `eec_cms`. Its
physical interpretation nevertheless follows the input sample: a dataset
already filtered or triggered on J/psi/dimuons does not provide the fully
inclusive jet cross section of Eq. (1). Likewise, the current private-Pythia
ntuples store only events containing one J/psi, so their result is a
J/psi-sample-normalized fragmentation density, not the paper's absolute
inclusive-jet-normalized function. An inclusive input sample (with consistent
cross-section/luminosity weights) is required for the latter.

Private source categories are: cat1 b-hadron (nonprompt), cat2 charmonium
feed-down, cat3 gluon/proton, cat4 quark, cat5 `3S1(8)`, cat6 `1S0(8)`, and
cat7 `3PJ(8)`. The category mapping uses the direct J/psi mother PDG ID and is
defined in `include/MotherCategory.h`. The empty fallback cat0 category is not
written or plotted.

Cat2 is stored as five mutually exclusive components:
`PrivateGen_cat2_from_b_hadron`, `_psi_2S`, `_chi_c1`, `_chi_c2`, and
`_others`. The first contains every b-hadron-to-charmonium event, independent
of the intermediate charmonium state. The next three contain their respective
direct mothers only when the effective grandmother is not a b hadron; all
remaining not-from-b charmonium mothers are grouped into `_others`.
`PrivateGen_cat2` is not written. Whenever an inclusive charmonium feed-down
curve or fraction is plotted, these five component directories are added
together, so their sum defines the unchanged inclusive cat2 yield.

The private adapter also forms
`J/psi + gamma` and `J/psi + pi+ pi-` candidates entirely from stable generator
particles, without using ancestry to select the photon or pions. The exclusive
outputs are `PrivateFeeddownTag_untagged`, `_chi_c`, and `_psi_2S`. Every tag
photon or pion must satisfy `DeltaR(particle,J/psi) < R`, using the configured
jet radius. Defaults are photon pT > 0.5 GeV, pion pT > 0.4 GeV, |eta| < 2.5,
0.38 < DeltaM_chi < 0.50 GeV, and 0.55 < DeltaM_psi(2S) < 0.63 GeV. Within
each hypothesis only the candidate with the smallest mass pull is retained. If
both hypotheses pass their mass windows, the one with the smaller mass pull is
selected. These fixed baseline settings and a perfect truth b-origin veto are
always applied before feed-down candidates are formed.
Best-candidate mass differences plus weighted and unweighted truth-versus-tag
migration matrices are stored under `PrivateFeeddownTagDiagnostics`; pre-veto
b/prompt yields are retained there as well. These tags are a baseline for
efficiency/purity studies, not a replacement for the truth-origin categories.

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
Merging uses eight parallel `hadd` workers by default because the Condor output
contains many small ROOT files; override this with `--hadd-jobs N` when needed.
No expected chunk count is needed; optionally use `--expected-chunks N` when a
strict completeness check is wanted. Different generator settings, data, and
MC remain separate by default. Use `--dataset NAME` to restrict the merge, or
`--combine-output all.root` when combining distinct datasets is really
intended. The old `./merge.sh OUTPUT_DIR DATASET [...]` syntax remains supported.
