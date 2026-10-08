#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
CMSSW_PATH="${CMSSW_BASE:-$(cd "${PROJECT_DIR}/../.." && pwd)}"
TARGET="all"
OUTPUT_DIR="${PROJECT_DIR}/bin"

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
        -h|--help)
            echo "Usage: $0 [--cmssw CMSSW_BASE] [--target cms|private|generate|plot|fragmentation|tools|all] [--output-dir DIR]"
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
mkdir -p "${OUTPUT_DIR}"

# All targets share project includes and ROOT/runtime linker settings.
build_source() {
    local relative_source="$1"
    local binary_name="$2"
    shift 2
    local source_file="${PROJECT_DIR}/${relative_source}"
    local output_file="${OUTPUT_DIR}/${binary_name}"
    echo "Compiling ${source_file} -> ${output_file}"
    g++ -std=c++17 -O2 -Wall -Wextra \
        "${ROOT_CFLAGS[@]}" -I"${PROJECT_DIR}/include" \
        "${source_file}" -o "${output_file}" \
        -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64" \
        -Wl,--disable-new-dtags \
        -Wl,-rpath,"${ROOT_LIBDIR}" -Wl,-rpath,"${TBB_BASE}/lib" \
        -Wl,-rpath,"${GCC_BASE}/lib64" \
        "$@" "${ROOT_LIBS[@]}"
}

if [[ "${TARGET}" == "all" || "${TARGET}" == "cms" ]]; then
    build_source src/analysis/eec_cms.cpp eec_cms \
        -I"${FASTJET_BASE}/include" -L"${FASTJET_BASE}/lib" \
        -Wl,-rpath,"${FASTJET_BASE}/lib" -lfastjet
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "private" ]]; then
    build_source src/analysis/eec_private.cpp eec_private \
        -I"${FASTJET_BASE}/include" -L"${FASTJET_BASE}/lib" \
        -Wl,-rpath,"${FASTJET_BASE}/lib" -lfastjet
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "generate" ]]; then
    build_source src/generation/pythia_private.cpp pythia_private \
        -I"${PYTHIA8_BASE}/include" -L"${PYTHIA8_BASE}/lib" \
        -Wl,-rpath,"${PYTHIA8_BASE}/lib" -lpythia8 -ltbb
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "plot" ]]; then
    build_source src/plotting/plot_eec.cpp plot_eec
    build_source src/plotting/plot_sideband_subtraction.cpp plot_sideband_subtraction
    build_source src/plotting/plot_jpsi_jet_pt.cpp plot_jpsi_jet_pt
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "cms" || \
      "${TARGET}" == "private" || "${TARGET}" == "fragmentation" || \
      "${TARGET}" == "tools" ]]; then
    build_source src/tools/make_fragmentation_function.cpp make_fragmentation_function
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "tools" ]]; then
    build_source src/tools/countnum.cpp countnum
fi

echo "Build complete: ${OUTPUT_DIR}"
