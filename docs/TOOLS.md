# Functional file guide

Run the examples from the repository root. The shell scripts resolve the
repository location independently of the caller's working directory.

## Build

```bash
./compile.sh --target all
```

The build initializes the requested CMSSW release and puts executables in
`bin/`. Use `--cmssw /path/to/CMSSW` to select a release or `--output-dir DIR`
to choose another binary directory. Existing binaries are local artifacts.

Each build generates a project-local `compile_commands.json` for all eight
active C++ sources using the actual CMSSW compiler and include flags. The
database is a generated local file ignored by Git; it replaces the old symlink
to the shared CMSSW database, which had no entries for this project.

To refresh editor configuration without compiling binaries, run:

```bash
./compile.sh --compile-commands-only
```

VS Code's clangd and C/C++ configuration read this database from the workspace
root. `scripts/clangd.sh` initializes the CMSSW runtime before launching clangd
so its compiler queries can load the required GCC libraries. Open this
directory as the workspace. After changing the configuration,
run **clangd: Restart language server** from the Command Palette, or reload the
window. `Ctrl+Shift+B` runs the full project build; the task list also provides
a CMS-only build and **Jpsi: refresh C++ configuration**.

| Target | Source files | Executables |
| --- | --- | --- |
| `cms` | `src/analysis/eec_cms.cpp`, fragmentation postprocessor | `eec_cms`, `make_fragmentation_function` |
| `private` | `src/analysis/eec_private.cpp`, fragmentation postprocessor | `eec_private`, `make_fragmentation_function` |
| `generate` | `src/generation/pythia_private.cpp` | `pythia_private` |
| `plot` | Three files in `src/plotting/` | `plot_eec`, `plot_sideband_subtraction`, `plot_jpsi_jet_pt` |
| `fragmentation` | `src/tools/make_fragmentation_function.cpp` | `make_fragmentation_function` |
| `tools` | Two files in `src/tools/` | `make_fragmentation_function`, `countnum` |
| `all` | All active C++ sources | All of the above |

Only the headers used by the active programs remain in `include/`:

| Header | Purpose |
| --- | --- |
| `EECCommon.h` | Shared selection, kinematics, and histogram schema |
| `MotherCategory.h` | Truth-origin categories |
| `MCJetsAndDaughters.h` | CMS ntuple reader |
| `CharmoniumInfoPrivate.h` | Private-Pythia ntuple reader |
| `ProgressBar.h` | Generator progress display |

## Workflow scripts

| Implementation | Root entry point | Purpose |
| --- | --- | --- |
| `scripts/compile.sh` | `./compile.sh` | Compile selected targets once on the host |
| `scripts/condor.sh` | `./condor.sh` | Compile and create HTCondor submit files under `generated/`; `--submit` submits them |
| `scripts/run_job.sh` | `./run_job.sh` | Run a prebuilt generator or EEC adapter on a batch worker |
| `scripts/merge.sh` | `./merge.sh` | Validate and merge ROOT chunks, then construct fragmentation functions |

Condor jobs use `scripts/run_job.sh` and the repository's `bin/` directory.
Existing submit files using the root worker entry point continue to work.
`run_job.sh --source-dir DIR` expects a project root containing `bin/`.
`merge.sh --cleanup` removes chunks only after all requested merges succeed.

## Plotting and presentation

| File | Purpose | Example |
| --- | --- | --- |
| `src/plotting/plot_eec.cpp` | CMS/private EEC, momentum-fraction, and origin comparisons | `./bin/plot_eec --output-dir plots_eec` |
| `src/plotting/plot_sideband_subtraction.cpp` | Data sideband subtraction and signal/MC comparison | `./bin/plot_sideband_subtraction --output-dir plots_sideband_subtraction` |
| `src/plotting/plot_jpsi_jet_pt.cpp` | Selected J/psi and matching-jet pT comparisons | `./bin/plot_jpsi_jet_pt --output-dir plots_jpsi_jet_pt` |
| `scripts/plotting/montage_pt_pdfs.py` | Inclusive and pT-binned EEC/z PDF slides | `python3 scripts/plotting/montage_pt_pdfs.py --input-dir plots_eec` |
| `scripts/plotting/montage_jpsi_jet_pt_pdfs.py` | Full-range and zoomed pT PDF slides | `python3 scripts/plotting/montage_jpsi_jet_pt_pdfs.py --input-dir plots_jpsi_jet_pt` |
| `macros/compare.C` | Quick two-file ROOT shape and ratio comparison | See below |

The montage scripts require `pdflatex`; the J/psi/jet-pT montage also uses
`pdfunite` to combine pages. The scripts stay together because the pT montage
imports the shared layout helpers from `montage_pt_pdfs.py`. The EEC montage
default input directory matches `plot_eec`: `plots_eec_cms_private/`.

```bash
root -l -q 'macros/compare.C("private.root","PrivateGen","cms.root","CmsGen")'
```

Jet-rank fraction histograms are stored by the fragmentation postprocessor;
`plot_jet_rank_fractions.py` is not present in this checkout.

## Small utilities

```bash
./compile.sh --target tools
./bin/countnum -i /path/to/cms/dataset -o counts -n 1 -e 0
./bin/make_fragmentation_function FILE.root
```

`countnum` totals the CMS ntuple event counters. The fragmentation tool updates
the specified ROOT file after merging; `merge.sh` calls it automatically.

## Local and historical files

- `archive/`: preserved legacy workflows, results, and local state; ignored.
- `pythia_generation/`: historical ROOT/PDF results preserved at their original
  paths. Obsolete job scripts, logs, binary copies, and duplicate headers were
  removed; use `src/generation/pythia_private.cpp` and `scripts/condor.sh` for
  new generation jobs.
- `xsection/`: cross-section reference table.
- `chat/`: local conversation notes; ignored.
- `shuangyu.cc` files: local Kerberos credential caches despite the `.cc`
  extension. Keep them locally and exclude them from Git. Removing them from
  current tracking does not remove copies already present in Git history.

Do not put generated ROOT files, binaries, caches, or job logs alongside the
active sources. The ignore rules cover these outputs and local agent state.
