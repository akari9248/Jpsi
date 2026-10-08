#!/usr/bin/env python3
"""Arrange each EEC/z plot's inclusive and eight pT-bin PDFs for slides."""

import argparse
import re
from pathlib import Path
import subprocess
import tempfile

JPSI_BINS = (
    ("_jpsipt_0_12", "J/psi pT 0-12 GeV"),
    ("_jpsipt_12_16", "J/psi pT 12-16 GeV"),
    ("_jpsipt_16_20", "J/psi pT 16-20 GeV"),
    ("_jpsipt_20_30", "J/psi pT 20-30 GeV"),
    ("_jpsipt_30_50", "J/psi pT 30-50 GeV"),
    ("_jpsipt_50_100", "J/psi pT 50-100 GeV"),
    ("_jpsipt_100_200", "J/psi pT 100-200 GeV"),
    ("_jpsipt_200_Inf", "J/psi pT >=200 GeV"),
)
JET_BINS = (
    ("_jetpt_30_50", "jet pT 30-50 GeV"),
    ("_jetpt_50_100", "jet pT 50-100 GeV"),
    ("_jetpt_100_150", "jet pT 100-150 GeV"),
    ("_jetpt_150_200", "jet pT 150-200 GeV"),
    ("_jetpt_200_300", "jet pT 200-300 GeV"),
    ("_jetpt_300_400", "jet pT 300-400 GeV"),
    ("_jetpt_400_600", "jet pT 400-600 GeV"),
    ("_jetpt_600_Inf", "jet pT >=600 GeV"),
)


def discover_families(directory):
    families = []
    for bins in (JPSI_BINS, JET_BINS):
        first_suffix = bins[0][0]
        for positive in ((False, True) if bins is JPSI_BINS else (False,)):
            tail = first_suffix + ("_positive" if positive else "") + ".pdf"
            for first in directory.glob("*" + tail):
                base = first.name[: -len(tail)]
                inclusive = directory / (base + ("_positive" if positive else "") + ".pdf")
                if inclusive.exists():
                    families.append((base, bins, positive))
    return sorted(families, key=lambda item: (item[0], item[2], item[1][0][0]))


def tile(path, label, height):
    if path.exists():
        return (rf"\includegraphics[width=\linewidth,height={height}in,keepaspectratio]"
                + r"{\detokenize{" + str(path.resolve()) + "}}")
    safe_label = label.replace(">=", r"$\geq$")
    return (r"\fbox{\begin{minipage}[c][4in][c]{5.8in}\centering "
            + safe_label + r"\\[0.4in]No comparison plot\\"
            + r"(fewer than two nonempty curves)\end{minipage}}")


def row(paths_and_labels, width, height):
    parts = [rf"\begin{{minipage}}[c][{height}in][c]{{{width}in}}\centering "
             + tile(path, label, height) + r"\end{minipage}"
             for path, label in paths_and_labels]
    if len(parts) <= 2:
        return (r"\noindent\makebox[\textwidth][c]{"
                + r"\hspace{0.4in}".join(parts) + r"}\par" + "\n")
    return r"\noindent" + r"\hfill ".join(parts) + r"\par" + "\n"


def latex_document(entries, fraction=False):
    first, second = entries[:5], entries[5:]
    if fraction:
        page_size = "paperwidth=32in,paperheight=18in"
        width, height = 10.2, 8.4
    else:
        page_size = "paperwidth=23.4in,paperheight=16.5in"
        width, height = 7.45, 7.55
    pages = (row(first[:3], width, height) + r"\vfill" + "\n"
             + row(first[3:], width, height) + r"\newpage" + "\n"
             + row(second[:2], width, height) + r"\vfill" + "\n"
             + row(second[2:], width, height))
    return (r"\documentclass{article}" + "\n"
            + r"\usepackage[" + page_size + r",margin=0.25in]{geometry}" + "\n"
            + r"\usepackage{graphicx}" + "\n"
            + r"\pagestyle{empty}" + "\n"
            + r"\setlength{\parindent}{0pt}" + "\n"
            + r"\begin{document}" + "\n" + pages
            + r"\end{document}" + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=Path("plots_eec_cms_private"))
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--family", help="Build only this plot family (for previews)")
    args = parser.parse_args()
    source = args.input_dir.resolve()
    slides = (args.output_dir or source / "slides").resolve()
    slides.mkdir(parents=True, exist_ok=True)
    families = discover_families(source)
    if args.family:
        families = [family for family in families if family[0] == args.family]
    if not families:
        raise SystemExit(f"No plot families found in {source}")

    with tempfile.TemporaryDirectory(prefix="jpsi-pt-montage-") as temporary:
        work = Path(temporary)
        tex = work / "montage.tex"
        for base, bins, positive in families:
            tag = "_positive" if positive else ""
            inclusive = source / f"{base}{tag}.pdf"
            entries = [(inclusive, "inclusive")]
            entries.extend((source / f"{base}{suffix}{tag}.pdf", label)
                           for suffix, label in bins)
            job = f"{base}{tag}_pt_montage"
            tex.write_text(latex_document(entries, "_fractions" in base))
            result = subprocess.run(
                ["pdflatex", "-halt-on-error", "-interaction=batchmode",
                 "-output-directory", str(work), "-jobname", job, str(tex)],
                cwd=work, text=True, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT)
            if result.returncode:
                log = work / f"{job}.log"
                detail = log.read_text()[-1800:] if log.exists() else result.stdout
                raise RuntimeError(f"Failed to build {job}:\n{detail}")
            pdf = work / f"{job}.pdf"
            info = subprocess.check_output(["pdfinfo", str(pdf)], text=True)
            if not re.search(r"^Pages:\s+2\s*$", info, re.MULTILINE):
                raise RuntimeError(f"Expected two pages in {pdf}:\n{info}")
            pattern = slides / f"{job}_page%d.pdf"
            subprocess.run(
                ["pdfseparate", "-f", "1", "-l", "2", str(pdf),
                 str(pattern)], check=True, stdout=subprocess.DEVNULL)
            print(f"Wrote {slides / f'{job}_page1.pdf'} and "
                  f"{slides / f'{job}_page2.pdf'}")
    print(f"Built {2 * len(families)} single-page slide PDFs in {slides}")


if __name__ == "__main__":
    main()
