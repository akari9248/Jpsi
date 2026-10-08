#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
CMSSW_PATH="${CMSSW_BASE:-$(cd "${PROJECT_DIR}/../.." && pwd)}"
TARGET="all"
OUTPUT_DIR="${PROJECT_DIR}/bin"
COMPILE_COMMANDS_ONLY=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --cmssw|--target|--output-dir)
            if [[ $# -lt 2 || "$2" == --* ]]; then
                echo "ERROR: $1 requires a value" >&2
                exit 1
            fi
            case "$1" in
                --cmssw) CMSSW_PATH="$2" ;;
                --target) TARGET="$2" ;;
                --output-dir) OUTPUT_DIR="$2" ;;
            esac
            shift 2
            ;;
        --compile-commands-only) COMPILE_COMMANDS_ONLY=1; shift ;;
        -h|--help)
            echo "Usage: $0 [--cmssw CMSSW_BASE] [--target cms|private|generate|plot|fragmentation|tools|all] [--output-dir DIR] [--compile-commands-only]"
            exit 0
            ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

case "${TARGET}" in
    cms|private|generate|plot|fragmentation|tools|all) ;;
    *) echo "ERROR: target must be cms, private, generate, plot, fragmentation, tools, or all" >&2; exit 1 ;;
esac
if [[ ! -d "${CMSSW_PATH}/src" ]]; then
    echo "ERROR: invalid CMSSW path: ${CMSSW_PATH}" >&2
    exit 1
fi

# shellcheck source=/dev/null
source /cvmfs/cms.cern.ch/cmsset_default.sh
pushd "${CMSSW_PATH}/src" >/dev/null
eval "$(scramv1 runtime -sh)"
popd >/dev/null

FASTJET_BASE="$(scram tool info fastjet | awk -F= '/^FASTJET_BASE=/{print $2; exit}')"
PYTHIA8_BASE="$(scram tool info pythia8 | awk -F= '/^PYTHIA8_BASE=/{print $2; exit}')"
TBB_BASE="$(scram tool info tbb | awk -F= '/^TBB_BASE=/{print $2; exit}')"
GCC_BASE="$(scram tool info gcc-cxxcompiler | awk -F= '/^GCC_CXXCOMPILER_BASE=/{print $2; exit}')"
ROOT_LIBDIR="$(root-config --libdir)"
read -r -a ROOT_CFLAGS <<< "$(root-config --cflags)"
read -r -a ROOT_LIBS <<< "$(root-config --glibs)"
CXX_COMPILER="$(command -v g++)"
COMMANDS_TEMP="$(mktemp "${TMPDIR:-/tmp}/jpsi-compile-commands.XXXXXX")"
trap 'rm -f -- "${COMMANDS_TEMP}"' EXIT
if [[ ${COMPILE_COMMANDS_ONLY} -eq 0 ]]; then
    mkdir -p "${OUTPUT_DIR}"
fi

# All targets share project includes and ROOT/runtime linker settings.
build_source() {
    local relative_source="$1"
    local binary_name="$2"
    local selected="$3"
    shift 3
    local source_file="${PROJECT_DIR}/${relative_source}"
    local output_file="${OUTPUT_DIR}/${binary_name}"
    local command=(
        "${CXX_COMPILER}" -std=c++17 -O2 -Wall -Wextra
        "${ROOT_CFLAGS[@]}" -I"${PROJECT_DIR}/include"
        "${source_file}" -o "${output_file}"
        -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64"
        "-Wl,--disable-new-dtags"
        "-Wl,-rpath,${ROOT_LIBDIR}" "-Wl,-rpath,${TBB_BASE}/lib"
        "-Wl,-rpath,${GCC_BASE}/lib64"
        "$@" "${ROOT_LIBS[@]}"
    )
    # Record every source, even when only one build target is selected.
    python3 - "${COMMANDS_TEMP}" "${PROJECT_DIR}" "${source_file}" "${command[@]}" <<'PY'
import json
import sys

with open(sys.argv[1], "a") as output:
    json.dump({"directory": sys.argv[2], "file": sys.argv[3],
               "arguments": sys.argv[4:]}, output)
    output.write("\n")
PY
    if [[ "${selected}" == "yes" && ${COMPILE_COMMANDS_ONLY} -eq 0 ]]; then
        echo "Compiling ${source_file} -> ${output_file}"
        "${command[@]}"
    fi
}

# Select execution separately from compilation-database coverage.
selected() {
    local target
    for target in all "$@"; do
        if [[ "${TARGET}" == "${target}" ]]; then
            echo yes
            return
        fi
    done
    echo no
}

build_source src/analysis/eec_cms.cpp eec_cms "$(selected cms)" \
    -I"${FASTJET_BASE}/include" -L"${FASTJET_BASE}/lib" \
    -Wl,-rpath,"${FASTJET_BASE}/lib" -lfastjet
build_source src/analysis/eec_private.cpp eec_private "$(selected private)" \
    -I"${FASTJET_BASE}/include" -L"${FASTJET_BASE}/lib" \
    -Wl,-rpath,"${FASTJET_BASE}/lib" -lfastjet
build_source src/generation/pythia_private.cpp pythia_private "$(selected generate)" \
    -I"${PYTHIA8_BASE}/include" -L"${PYTHIA8_BASE}/lib" \
    -Wl,-rpath,"${PYTHIA8_BASE}/lib" -lpythia8 -ltbb
build_source src/plotting/plot_eec.cpp plot_eec "$(selected plot)"
build_source src/plotting/plot_sideband_subtraction.cpp plot_sideband_subtraction "$(selected plot)"
build_source src/plotting/plot_jpsi_jet_pt.cpp plot_jpsi_jet_pt "$(selected plot)"
build_source src/tools/make_fragmentation_function.cpp make_fragmentation_function "$(selected cms private fragmentation tools)"
build_source src/tools/countnum.cpp countnum "$(selected tools)"

# Replace the old symlink itself, never write into the shared CMSSW database.
python3 - "${COMMANDS_TEMP}" "${PROJECT_DIR}/compile_commands.json" <<'PY'
import json
import os
from pathlib import Path
import sys
import tempfile

records = [json.loads(line) for line in Path(sys.argv[1]).read_text().splitlines()]
destination = Path(sys.argv[2])
with tempfile.NamedTemporaryFile(mode="w", dir=destination.parent,
                                 prefix=".compile_commands-", delete=False) as output:
    json.dump(records, output, indent=2)
    output.write("\n")
    temporary = output.name
os.replace(temporary, destination)
PY
echo "Compilation database updated: ${PROJECT_DIR}/compile_commands.json"
if [[ ${COMPILE_COMMANDS_ONLY} -eq 0 ]]; then
    echo "Build complete: ${OUTPUT_DIR}"
fi
