#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMSSW_PATH="${CMSSW_BASE:-$(cd "${SCRIPT_DIR}/../.." && pwd)}"
TARGET="all"
OUTPUT_DIR="${SCRIPT_DIR}/bin"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --cmssw) CMSSW_PATH="$2"; shift 2 ;;
        --target) TARGET="$2"; shift 2 ;;
        --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: $0 [--cmssw CMSSW_BASE] [--target cms|private|generate|plot|all] [--output-dir DIR]"
            exit 0
            ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

if [[ ! -d "${CMSSW_PATH}/src" ]]; then
    echo "ERROR: invalid CMSSW path: ${CMSSW_PATH}" >&2
    exit 1
fi
if [[ "${TARGET}" != "cms" && "${TARGET}" != "private" && \
      "${TARGET}" != "generate" && "${TARGET}" != "plot" && \
      "${TARGET}" != "all" ]]; then
    echo "ERROR: target must be cms, private, generate, plot, or all" >&2
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

build_one() {
    local mode="$1"
    local source_file="${SCRIPT_DIR}/eec_${mode}.cpp"
    local output_file="${OUTPUT_DIR}/eec_${mode}"
    echo "Compiling ${source_file} -> ${output_file}"
    g++ -std=c++17 -O2 -Wall -Wextra \
        "${ROOT_CFLAGS[@]}" \
        -I"${FASTJET_BASE}/include" \
        "${source_file}" -o "${output_file}" \
        -L"${FASTJET_BASE}/lib" -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64" \
        -Wl,--disable-new-dtags \
        -Wl,-rpath,"${ROOT_LIBDIR}" -Wl,-rpath,"${FASTJET_BASE}/lib" \
        -Wl,-rpath,"${TBB_BASE}/lib" -Wl,-rpath,"${GCC_BASE}/lib64" \
        -lfastjet "${ROOT_LIBS[@]}"
}

build_generator() {
    local source_file="${SCRIPT_DIR}/pythia_private.cpp"
    local output_file="${OUTPUT_DIR}/pythia_private"
    echo "Compiling ${source_file} -> ${output_file}"
    g++ -std=c++17 -O2 -Wall -Wextra \
        "${ROOT_CFLAGS[@]}" \
        -I"${PYTHIA8_BASE}/include" \
        "${source_file}" -o "${output_file}" \
        -L"${PYTHIA8_BASE}/lib" -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64" \
        -Wl,--disable-new-dtags \
        -Wl,-rpath,"${ROOT_LIBDIR}" -Wl,-rpath,"${PYTHIA8_BASE}/lib" \
        -Wl,-rpath,"${TBB_BASE}/lib" -Wl,-rpath,"${GCC_BASE}/lib64" \
        -lpythia8 -ltbb "${ROOT_LIBS[@]}"
}

build_plot() {
    local source_file="${SCRIPT_DIR}/plot_eec.cpp"
    local output_file="${OUTPUT_DIR}/plot_eec"
    echo "Compiling ${source_file} -> ${output_file}"
    g++ -std=c++17 -O2 -Wall -Wextra \
        "${ROOT_CFLAGS[@]}" \
        "${source_file}" -o "${output_file}" \
        -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64" \
        -Wl,--disable-new-dtags \
        -Wl,-rpath,"${ROOT_LIBDIR}" -Wl,-rpath,"${TBB_BASE}/lib" \
        -Wl,-rpath,"${GCC_BASE}/lib64" \
        "${ROOT_LIBS[@]}"
}

build_sideband_plot() {
    local source_file="${SCRIPT_DIR}/plot_sideband_subtraction.cpp"
    local output_file="${OUTPUT_DIR}/plot_sideband_subtraction"
    echo "Compiling ${source_file} -> ${output_file}"
    g++ -std=c++17 -O2 -Wall -Wextra \
        "${ROOT_CFLAGS[@]}" \
        "${source_file}" -o "${output_file}" \
        -L"${TBB_BASE}/lib" -L"${GCC_BASE}/lib64" \
        -Wl,--disable-new-dtags \
        -Wl,-rpath,"${ROOT_LIBDIR}" -Wl,-rpath,"${TBB_BASE}/lib" \
        -Wl,-rpath,"${GCC_BASE}/lib64" \
        "${ROOT_LIBS[@]}"
}

if [[ "${TARGET}" == "all" || "${TARGET}" == "cms" ]]; then
    build_one cms
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "private" ]]; then
    build_one private
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "generate" ]]; then
    build_generator
fi
if [[ "${TARGET}" == "all" || "${TARGET}" == "plot" ]]; then
    build_plot
    build_sideband_plot
fi

echo "Build complete: ${OUTPUT_DIR}"
