#!/usr/bin/env python3
"""Arrange ROOT pT comparison PDFs into vector slides, like montage_pt_pdfs.py."""

import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from montage_pt_pdfs import JPSI_BINS, latex_document, row

PRIVATE_KEYS = ("oniaoff_cr0", "oniaoff_cr1", "oniaon_cr0", "oniaon_cr1")
CATEGORY_KEYS = (
    "b_hadron", "charmonium_from_b", "non_b_charmonium", "quark", "octet"
)
OBSERVABLES = ("jpsi_pt", "jpsijet_pt")


def overview_document(entries):
    if len(entries) <= 4:
        width, height = 10.8, 7.55
        top, bottom = entries[:2], entries[2:]
    else:
        width, height = 7.45, 7.55
        top, bottom = entries[:3], entries[3:]
    body = row(top, width, height) + r"\vfill" + "\n" + row(bottom, width, height)
    return (
        r"\documentclass{article}" + "\n"
        + r"\usepackage[paperwidth=23.4in,paperheight=16.5in,margin=0.25in]{geometry}" + "\n"
        + r"\usepackage{graphicx}" + "\n"
        + r"\pagestyle{empty}" + "\n"
        + r"\setlength{\parindent}{0pt}" + "\n"
        + r"\begin{document}" + "\n" + body
        + r"\end{document}" + "\n"
    )


def family_plots(source, zoom):
    tag = "_zoom" if zoom else ""
    families = []
    for group in ("cms", "private"):
        entries = [(source / f"{group}_jpsijet_pt{tag}.pdf", "inclusive")]
        entries.extend(
            (source / "sample_by_jpsipt" /
             f"{group}_jpsijet_pt{suffix}{tag}.pdf", label)
            for suffix, label in JPSI_BINS
        )
        families.append((f"{group}_samples", entries))
    for key in PRIVATE_KEYS:
        entries = [(
            source / "private_categories" /
            f"private_{key}_jpsijet_pt_categories{tag}.pdf", "inclusive"
        )]
        entries.extend(
            (source / "private_categories_by_jpsipt" /
             f"private_{key}_jpsijet_pt_categories{suffix}{tag}.pdf", label)
            for suffix, label in JPSI_BINS
        )
        families.append((f"private_{key}_categories", entries))
    for key in CATEGORY_KEYS:
        entries = [(
            source / "category_across_samples" /
            f"private_{key}_jpsijet_pt_samples{tag}.pdf", "inclusive"
        )]
        entries.extend(
            (source / "category_across_samples_by_jpsipt" /
             f"private_{key}_jpsijet_pt_samples{suffix}{tag}.pdf", label)
            for suffix, label in JPSI_BINS
        )
        families.append((f"private_{key}_across_samples", entries))
    return families


def overview_plots(source, zoom):
    tag = "_zoom" if zoom else ""
    overviews = [(
        "samples_overview",
        [(source / f"{group}_{observable}{tag}.pdf", group + observable)
         for group in ("cms", "private") for observable in OBSERVABLES]
    )]
    for observable in OBSERVABLES:
        entries = [
            (source / "private_categories" /
             f"private_{key}_{observable}_categories{tag}.pdf", key)
            for key in PRIVATE_KEYS
        ]
        entries.append((
            source / "private_categories" /
            f"private_oniaon_cr1_{observable}_non_b_charmonium{tag}.pdf",
            "non-B charmonium modes"
        ))
        overviews.append((f"private_categories_{observable}_overview", entries))
        entries = [
            (source / "category_across_samples" /
             f"private_{key}_{observable}_samples{tag}.pdf", key)
            for key in CATEGORY_KEYS
        ]
        overviews.append((f"category_across_samples_{observable}_overview", entries))
    return overviews


def build_pdf(name, tex_content, expected_pages, work):
    tex = work / "montage.tex"
    tex.write_text(tex_content)
    result = subprocess.run(
        ["pdflatex", "-halt-on-error", "-interaction=batchmode",
         "-output-directory", str(work), "-jobname", name, str(tex)],
        cwd=work, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    if result.returncode:
        log = work / f"{name}.log"
        detail = log.read_text()[-1800:] if log.exists() else result.stdout
        raise RuntimeError(f"Failed to build {name}:\n{detail}")
    pdf = work / f"{name}.pdf"
    info = subprocess.check_output(["pdfinfo", str(pdf)], text=True)
    if not re.search(rf"^Pages:\s+{expected_pages}\s*$", info, re.MULTILINE):
        raise RuntimeError(f"Unexpected page count in {pdf}:\n{info}")
    return pdf


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, default=Path("plots_jpsi_jet_pt"))
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--family", help="Build only one family or overview in both sets")
    args = parser.parse_args()
    source = args.input_dir.resolve()
    slides = (args.output_dir or source / "slides").resolve()
    slides.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="jpsi-jet-pt-montage-") as temporary:
        work = Path(temporary)
        for zoom in (False, True):
            kind = "zoom" if zoom else "full"
            target_dir = slides / kind
            target_dir.mkdir(parents=True, exist_ok=True)
            families = family_plots(source, zoom)
            overviews = overview_plots(source, zoom)
            if args.family:
                families = [item for item in families if item[0] == args.family]
                overviews = [item for item in overviews if item[0] == args.family]
                if not families and not overviews:
                    raise SystemExit(f"Unknown family: {args.family}")
            outputs = []
            for name, entries in overviews:
                for path, _ in entries:
                    if not path.exists():
                        raise FileNotFoundError(path)
                pdf = build_pdf(name, overview_document(entries), 1, work)
                target = target_dir / f"{name}.pdf"
                shutil.copy2(pdf, target)
                outputs.append(target)
                print(f"Wrote {target}")
            for name, entries in families:
                for path, _ in entries:
                    if not path.exists():
                        raise FileNotFoundError(path)
                pdf = build_pdf(name + "_pt_montage", latex_document(entries), 2, work)
                pattern = target_dir / f"{name}_pt_montage_page%d.pdf"
                subprocess.run(["pdfseparate", "-f", "1", "-l", "2", str(pdf),
                                str(pattern)], check=True)
                for page in (1, 2):
                    target = target_dir / f"{name}_pt_montage_page{page}.pdf"
                    outputs.append(target)
                    print(f"Wrote {target}")
            if not args.family:
                deck = slides / f"jpsi_jet_pt_{kind}_slides.pdf"
                subprocess.run(["pdfunite", *(str(path) for path in outputs),
                                str(deck)], check=True)
                print(f"Wrote {deck} ({len(outputs)} pages)")


if __name__ == "__main__":
    main()
