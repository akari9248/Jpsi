# Historical generation results

This directory retains the historical ROOT files and PDF comparisons at their
original paths. They are reference results outside the active build.

Obsolete standalone binaries, duplicate headers, job scripts requiring missing
`pythia.cpp` sources, submit files, and old job logs have been removed. The
removed tracked files remain recoverable from Git history.

For new generation jobs, use the maintained workflow from the repository root:

```bash
./compile.sh --target generate
./condor.sh --mode generate --chunks 1 --events 500 --output-dir /path/to/output
```

The generator source is `src/generation/pythia_private.cpp`. The submit command
only prepares jobs unless `--submit` is supplied.
