#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="${SCRIPT_DIR}"
MODE=""
INPUT_DIR=""
OUTPUT_BASE=""
CHUNKS=""
CHUNK_ID=""
CMSSW_PATH=""
FORWARD_ARGS=()
SEED=""
EVENTS="1000"
ONIASHOWER="off"
CRMODE="0"
PYTHIA8_DATA=""
LHAPDF_LIB=""
LHAPDF_DATA=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode) MODE="$2"; shift 2 ;;
        --input) INPUT_DIR="$2"; shift 2 ;;
        --output) OUTPUT_BASE="$2"; shift 2 ;;
        --chunks) CHUNKS="$2"; shift 2 ;;
        --chunk-id) CHUNK_ID="$2"; shift 2 ;;
        --cmssw) CMSSW_PATH="$2"; shift 2 ;;
        --source-dir) SOURCE_DIR="$2"; shift 2 ;;
        --seed) SEED="$2"; shift 2 ;;
        --events) EVENTS="$2"; shift 2 ;;
        --oniashower) ONIASHOWER="$2"; shift 2 ;;
        --crmode) CRMODE="$2"; shift 2 ;;
        --pythia8-data) PYTHIA8_DATA="$2"; shift 2 ;;
        --lhapdf-lib) LHAPDF_LIB="$2"; shift 2 ;;
        --lhapdf-data) LHAPDF_DATA="$2"; shift 2 ;;
        --radius|--jet-pt-min|--jet-eta-max|--muon-leading-pt|--muon-subleading-pt|--mass-min|--mass-max|--constituent-scaling|--level|--cms-filters|--hot-zone|--prompt)
            FORWARD_ARGS+=("$1" "$2")
            shift 2
            ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

if [[ -z "${MODE}" || -z "${OUTPUT_BASE}" || -z "${CHUNK_ID}" || \
      -z "${CMSSW_PATH}" ]]; then
    echo "ERROR: mode, output, chunk-id, and cmssw are required" >&2
    exit 1
fi
if [[ "${MODE}" != "cms" && "${MODE}" != "private" && \
      "${MODE}" != "generate" ]]; then
    echo "ERROR: mode must be cms, private, or generate" >&2
    exit 1
fi
if [[ "${MODE}" != "generate" && ( -z "${INPUT_DIR}" || -z "${CHUNKS}" ) ]]; then
    echo "ERROR: input and chunks are required for analysis modes" >&2
    exit 1
fi
if [[ "${MODE}" == "generate" && -z "${SEED}" ]]; then
    echo "ERROR: seed is required for generation" >&2
    exit 1
fi
if [[ "${MODE}" == "generate" && \
      ( -z "${PYTHIA8_DATA}" || -z "${LHAPDF_LIB}" || -z "${LHAPDF_DATA}" ) ]]; then
    echo "ERROR: Pythia and LHAPDF runtime paths are required for generation" >&2
    exit 1
fi
if [[ ! -d "${SOURCE_DIR}" && -d "${CMSSW_PATH}/src/Jpsi" ]]; then
    SOURCE_DIR="${CMSSW_PATH}/src/Jpsi"
fi
if [[ ! -d "${SOURCE_DIR}" ]]; then
    echo "ERROR: source directory does not exist: ${SOURCE_DIR}" >&2
    exit 1
fi

mkdir -p "$(dirname "${OUTPUT_BASE}")"
if [[ "${MODE}" == "generate" ]]; then
    BINARY="${SOURCE_DIR}/bin/pythia_private"
    if [[ ! -x "${BINARY}" ]]; then
        echo "ERROR: generator binary not found: ${BINARY}" >&2
        exit 1
    fi
    export PYTHIA8DATA="${PYTHIA8_DATA}"
    export LHAPDF_DATA_PATH="${LHAPDF_DATA}"
    export LD_LIBRARY_PATH="${LHAPDF_LIB}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    mkdir -p "${OUTPUT_BASE}"
    echo "Generating private Pythia sample, chunk ${CHUNK_ID}, seed ${SEED}"
    "${BINARY}" \
        --seed "${SEED}" \
        --chunknum "${CHUNK_ID}" \
        --events "${EVENTS}" \
        --outputdir "${OUTPUT_BASE}" \
        --oniashower "${ONIASHOWER}" \
        --crmode "${CRMODE}"
    exit 0
fi

BINARY="${SOURCE_DIR}/bin/eec_${MODE}"
if [[ ! -x "${BINARY}" ]]; then
    echo "ERROR: analysis binary not found: ${BINARY}" >&2
    exit 1
fi
echo "Running unified ${MODE} EEC, chunk ${CHUNK_ID}/${CHUNKS}"
"${BINARY}" \
    --input "${INPUT_DIR}" \
    --output "${OUTPUT_BASE}" \
    --chunks "${CHUNKS}" \
    --chunk-id "${CHUNK_ID}" \
    "${FORWARD_ARGS[@]}"
