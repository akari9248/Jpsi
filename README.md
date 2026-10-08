# Unified jet-restricted J/psi EEC

The active workflow is organized by function. See [the tool guide](docs/TOOLS.md)
for each executable, build target, and script.

| Directory | Purpose |
| --- | --- |
| `src/analysis/` | CMS and private-Pythia EEC adapters |
| `src/generation/` | Private `CharmoniumInfo` ntuple generator |
| `src/plotting/` | EEC, sideband-subtraction, and J/psi/jet-pT C++ plotters |
| `src/tools/` | Fragmentation postprocessor and event-count utility |
| `include/` | Shared EEC implementation and ntuple/helper headers |
| `scripts/` | Build, Condor preparation, batch worker, and merge scripts |
| `scripts/plotting/` | PDF montage tools |
| `macros/` | Interactive ROOT comparison macro |
| `bin/`, `generated/`, `plots_*/` | Local build and runtime outputs (ignored by Git) |

The root-level `compile.sh`, `condor.sh`, `merge.sh`, and `run_job.sh` forward
to the organized implementations, so existing shell workflow commands continue
to work. Run ROOT comparisons through `macros/compare.C`. All C++ targets are built with `include/` on the
header search path; `include/EECCommon.h` remains the shared implementation of
jet acceptance, J/psi-jet association, helicity-frame boost, decay-muon removal,
energy weighting, and histogram filling.

Previous workflows and local results are preserved under `archive/`.
`pythia_generation/` retains historical ROOT/PDF results; `xsection/` contains
the cross-section reference table. Obsolete generation programs, job scripts,
logs, binary copies, and unused headers have been removed. Local notes remain
in `chat/`.

The comparable histograms have identical names inside the `CmsGen`, `CmsReco`,
and `PrivateGen` ROOT directories. The main ones are
`eec_alljets_all` and `eec_jpsijet_all`. Every EEC distribution also has
J/psi-pT-binned variants with suffixes such as `_jpsipt_0_12` and
`_jpsipt_200_Inf`. The redundant `count_*` companion histograms are not stored.
The J/psi pT bins are 0-12, 12-16, 16-20, 20-30, 30-50, 50-100,
100-200, and >=200 GeV. When plotting older ROOT files, `plot_eec` combines
the five old bins below 12 GeV into the new 0-12 GeV bin.

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
`PrivateGen_cat2` is not written. The main private fraction plot has six
exclusive sources: b-hadron, b-hadron via charmonium, non-B charmonium,
gluon/proton, quark, and the sum of the three octet categories. Two
subfraction plots show the three octet components within the octet yield and
the four non-B charmonium components within the non-B charmonium yield. Each
fraction plot labels the unweighted number of selected J/psi candidates in
each sample and J/psi-pT interval, summed from the categories shown in that
plot. The octet and non-B charmonium subplots therefore show their own J/psi
counts.

The private adapter also forms
`J/psi + gamma` and `J/psi + pi+ pi-` candidates entirely from stable generator
particles, without using ancestry to select the photon or pions. The exclusive
outputs are `PrivateFeeddownTag_untagged`, `_chi_c`, and `_psi_2S`. Every tag
photon or pion must satisfy `DeltaR(particle,J/psi) < R`, using the configured
jet radius. Defaults are photon pT > 0.5 GeV, pion pT > 0.4 GeV, |eta| < 2.5,
0.38 < DeltaM_chi < 0.50 GeV, and 0.579 < DeltaM_psi(2S) < 0.599 GeV. Within
each hypothesis only the candidate with the smallest mass pull is retained. If
both hypotheses pass their mass windows, the one with the smaller mass pull is
selected. These fixed baseline settings and a perfect truth b-origin veto are
always applied before feed-down candidates are formed.
Weighted and unweighted truth-versus-tag migration matrices are stored under
`PrivateFeeddownTagDiagnostics`; pre-veto b/prompt yields are retained there
as well. The merged ROOT file also contains tag efficiency and purity metrics.
Best-candidate mass and dipion diagnostic plots are no longer produced. These
tags are a baseline for efficiency/purity studies, not a replacement for the
truth-origin categories.

## Common operational definition

1. Require the same dimuon mass, eta, and asymmetric muon-pT cuts.
2. Select jets with the same `R`, pT threshold, and eta acceptance.
3. Require the two highest-pT selected jets to have pT >= 30 GeV,
   |eta| <= 2.1, |DeltaPhi| > 2, and
   |pT1 - pT2| / (pT1 + pT2) < 0.3.
4. Require both selected decay muons in the same J/psi jet and
   `DeltaR(J/psi, jet) < R`.
5. Exclude the two selected decay muons.
6. Boost every remaining selected-jet constituent to the dimuon rest frame.
7. Fill with weight `eventWeight * E_rest / M_dimuon`.

The EEC `alljets` histograms still use every selected jet in a passing
dijet event; `jpsijet` uses only the jet containing the J/psi. New
`jpsi_jet_rank` and `jpsi_jet_rank_unweighted` histograms have three visible
bins: leading, subleading, and other (third jet or later). The postprocessor
also reads older CMS files whose "other" count is in overflow. It writes both
`jpsi_jet_rank_fraction_weighted` and
`jpsi_jet_rank_fraction_unweighted`, each normalized over all three
categories. The legacy `jpsi_jet_rank_fraction` name uses weighted counts
for CMS and unweighted counts for private samples. To produce separate PDF
plots of weighted and unweighted fractions across CMS and private samples,
read these fraction histograms from the merged ROOT files. The standalone
`plot_jet_rank_fractions.py` script is not included in this checkout. Weighted
MC counts use the stored generator event weight; data has unit weight.

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
muon pT > 2/2 GeV.

For a quick shape-and-ratio comparison:

```bash
root -l -q 'macros/compare.C("private.root","PrivateGen","cms.root","CmsGen")'
```

For the full CMS-sample, private-sample, and OniaOn-origin comparisons,
`plot_eec` reads CMS ROOT files from `/eos/user/s/shuangyu/public/Jpsi/eec_cms`
by default. CMS plots compare the six samples without source-category splitting:

```bash
./compile.sh --target plot
./bin/plot_eec --output-dir plots_eec
```

For the selected J/psi and matching-jet pT distributions, use the CERN ROOT
C++ plotter. It reads `CmsReco` and `PrivateGen`, applies each histogram's
existing event weights, and normalizes each sample to unit area. The main
figures use the same upper-distribution/lower-ratio ROOT template as `plot_eec`;
matching `_zoom.pdf` figures show a focused pT range (shifted upward for high J/psi pT bins). The plots use 4 GeV
J/psi pT bins and 20 GeV jet pT bins above the 30 GeV jet threshold. Private
comparisons use the four PYTHIA 8.311 samples. The plotter writes PDF files only:

```bash
./compile.sh --target plot
./bin/plot_jpsi_jet_pt --output-dir plots_jpsi_jet_pt
```

The top-level PDFs compare inclusive CMS or private samples. The
`sample_by_jpsipt/` PDFs compare samples in each J/psi pT bin using the
matching jet pT histogram. `private_categories/` compares source categories
within each private sample, and `private_categories_by_jpsipt/` repeats that
comparison for matching jet pT in each J/psi pT bin. The
`category_across_samples/` and `category_across_samples_by_jpsipt/` PDFs fix
a source category and compare private settings. The Onia on CR1 category
plots also resolve the non-B charmonium feed-down modes. Ratios use the first
visible curve as reference; empty categories are omitted. Only PYTHIA 8.311
private samples are used.

To arrange the pT PDFs into the same A2 slide layout as the EEC figures, run:

```bash
python3 scripts/plotting/montage_jpsi_jet_pt_pdfs.py --input-dir plots_jpsi_jet_pt
```

Every comparison is produced in both full-range and zoomed PDF versions.
The montage command writes 27 A2 single-page vector PDFs under each of
`plots_jpsi_jet_pt/slides/full/` and `plots_jpsi_jet_pt/slides/zoom/`, plus
`jpsi_jet_pt_full_slides.pdf` and `jpsi_jet_pt_zoom_slides.pdf` (27 pages
each). Sample and category families place the inclusive matching-jet pT plot
next to its eight J/psi pT bins over two pages. Overview pages collect the
inclusive J/psi and matching-jet pT plots.

For CMS-only figures from the `eec_cms` ROOT directory (six samples, without
source categories), use:

```bash
./bin/plot_eec --cms-dir /eos/user/s/shuangyu/public/Jpsi/eec_cms \
  --draw-private off --output-dir plots_eec_cms
python3 scripts/plotting/montage_pt_pdfs.py --input-dir plots_eec_cms
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

The private EEC, `z_pt = pT(J/psi)/pT(jet)`, and `z_h` plots first compare
sources within each sample and then compare samples for each source. Within
a sample, the category curves are b-hadron, b-hadron via charmonium, all
non-B charmonium combined, quark, and the combined octet category. The
cross-sample source plots also show #psi(2S), #chi_c1, and #chi_c2 separately.
Gluon/proton stays in the fraction plots. Within OniaOff CR0, the category plots show only
b-hadron and b-hadron via charmonium; OniaOff CR1 also shows quark. The
same restrictions apply when selecting samples for each category's cross-sample
comparison, including the combined non-B charmonium comparison.
Empty histograms are omitted from a comparison; sparse nonempty z curves
remain visible with their statistical errors. Both EEC and category z plots
have a lower ratio panel relative to the first nonempty curve. Category z and
z_h ratio axes show 0-2 so the sparse high-z tail does not set the scale. EEC plots
include all J/psi-pT bins and optional positive-coschi versions (disable with
`--positive-half off`). The z plots include all jet-pT bins. Separate EEC, z, and z_h figures
also overlay #psi(2S), #chi_c1, and #chi_c2 within each OniaOn sample,
so their shapes can be compared directly. A separate EEC, z, and z_h series
compares the sum of all non-B charmonium sources across the two OniaOn samples.

To place each inclusive plot and its eight pT bins together, run
`python3 scripts/plotting/montage_pt_pdfs.py --input-dir plots_eec_private`. The script uses
`pdflatex` and writes single-page vector PDFs directly to
`plots_eec_private/slides`: page 1 contains inclusive plus the first four pT
bins, and page 2 contains the remaining four. Rows with two plots are centered;
fraction plots use wider landscape pages so their bars remain large. It groups
EEC, fraction, z, and z_h plot families; missing bin plots receive a labeled
placeholder.

The CMS Hard-QCD nominal and ext1 histograms are added before shape
normalization.

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
`--seed-start` (default `100000000`). `src/generation/pythia_private.cpp` also stores its full
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
