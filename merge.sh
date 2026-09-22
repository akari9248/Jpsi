#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FRAGMENTATION_TOOL="${SCRIPT_DIR}/bin/make_fragmentation_function"
# This hadd build does not support -L / -Ltype (skip list), so
# hadd_skip_metadata.txt is no longer used. If you later move to a ROOT
# version that supports it, restore:
#   hadd -f -L "${HADD_SKIP_LIST}" -Ltype SkipListed ...
# HADD_SKIP_LIST="${SCRIPT_DIR}/hadd_skip_metadata.txt"

MODE=""
OUTPUT_DIR=""
EXPECTED_CHUNKS=""
COMBINE_OUTPUT=""
CLEANUP=0
HADD_JOBS=8
DATASETS=()

usage() {
    cat <<USAGE
Usage:
  $0 --mode private|cms [options]
  $0 OUTPUT_DIR DATASET [DATASET ...]   (legacy syntax)

Options:
  --output-dir DIR       Directory containing DATASET_ChunkN.root files
  --dataset NAME         Dataset to merge; may be repeated
  --expected-chunks N    Fail if any chunk from 0 through N-1 is missing
  --combine-output FILE  Also combine all merged datasets into one ROOT file
  --hadd-jobs N          Parallel hadd workers (default: 8)
  --cleanup              Delete chunk files after every merge succeeds
  -h, --help             Show this help

With --mode and no --dataset, datasets are discovered automatically from the
chunk filenames. By default each dataset is kept separate.
USAGE
}

# Preserve the original interface: ./merge.sh OUTPUT_DIR DATASET [...]
if [[ $# -gt 0 && "$1" != -* ]]; then
    if [[ $# -lt 2 ]]; then
        usage >&2
        exit 1
    fi
    OUTPUT_DIR="$1"
    shift
    DATASETS=("$@")
else
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --mode) MODE="$2"; shift 2 ;;
            --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
            --dataset) DATASETS+=("$2"); shift 2 ;;
            --expected-chunks) EXPECTED_CHUNKS="$2"; shift 2 ;;
            --combine-output) COMBINE_OUTPUT="$2"; shift 2 ;;
            --hadd-jobs) HADD_JOBS="$2"; shift 2 ;;
            --cleanup) CLEANUP=1; shift ;;
            -h|--help) usage; exit 0 ;;
            *) echo "ERROR: unknown option $1" >&2; usage >&2; exit 1 ;;
        esac
    done
fi

if [[ -n "${MODE}" && "${MODE}" != "private" && "${MODE}" != "cms" ]]; then
    echo "ERROR: mode must be private or cms" >&2
    exit 1
fi
if [[ -z "${OUTPUT_DIR}" ]]; then
    case "${MODE}" in
        private) OUTPUT_DIR="/eos/user/s/shuangyu/public/Jpsi/eec_unified_private" ;;
        cms) OUTPUT_DIR="/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms" ;;
        *) echo "ERROR: provide --mode or --output-dir" >&2; exit 1 ;;
    esac
fi
if [[ -n "${EXPECTED_CHUNKS}" && \
      ! "${EXPECTED_CHUNKS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERROR: --expected-chunks must be a positive integer" >&2
    exit 1
fi
if [[ ! "${HADD_JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERROR: --hadd-jobs must be a positive integer" >&2
    exit 1
fi
if [[ ! -d "${OUTPUT_DIR}" ]]; then
    echo "ERROR: output directory does not exist: ${OUTPUT_DIR}" >&2
    exit 1
fi

HADD_PARALLEL_ARGS=()
HADD_LOCAL_TEMP="$(mktemp -d /tmp/jpsi-hadd-XXXXXX)"
if (( HADD_JOBS > 1 )); then
    HADD_PARALLEL_ARGS=(-j "${HADD_JOBS}" -d "${HADD_LOCAL_TEMP}")
    echo "Parallel hadd: ${HADD_JOBS} workers"
fi
echo "Local merge directory: ${HADD_LOCAL_TEMP}"

cleanup_hadd_local_temp() {
    if [[ -n "${HADD_LOCAL_TEMP}" && -d "${HADD_LOCAL_TEMP}" && \
          "${HADD_LOCAL_TEMP}" == /tmp/jpsi-hadd-* ]]; then
        rm -rf -- "${HADD_LOCAL_TEMP}"
    fi
}
trap cleanup_hadd_local_temp EXIT

publish_root_file() {
    local source="$1"
    local destination="$2"
    local staged
    if [[ "${destination}" == *.root ]]; then
        staged="${destination%.root}.copy.$$.root"
    else
        staged="${destination}.copy.$$.root"
    fi
    if ! cp -f -- "${source}" "${staged}"; then
        rm -f -- "${staged}"
        return 1
    fi
    if ! rootls "${staged}" >/dev/null; then
        rm -f -- "${staged}"
        return 1
    fi
    mv -f -- "${staged}" "${destination}"
}

# If no datasets were named, infer them from DATASET_ChunkN.root.
if [[ ${#DATASETS[@]} -eq 0 ]]; then
    declare -A discovered=()
    shopt -s nullglob
    chunkFiles=("${OUTPUT_DIR}"/*_Chunk*.root)
    shopt -u nullglob
    for path in "${chunkFiles[@]}"; do
        filename="${path##*/}"
        if [[ "${filename}" =~ ^(.+)_Chunk([0-9]+)\.root$ ]]; then
            discovered["${BASH_REMATCH[1]}"]=1
        fi
    done
    if [[ ${#discovered[@]} -gt 0 ]]; then
        mapfile -t DATASETS < <(printf '%s\n' "${!discovered[@]}" | sort)
    fi
fi
if [[ ${#DATASETS[@]} -eq 0 ]]; then
    echo "ERROR: no DATASET_ChunkN.root files found in ${OUTPUT_DIR}" >&2
    exit 1
fi

MERGED_FILES=()
for dataset in "${DATASETS[@]}"; do
    shopt -s nullglob
    files=("${OUTPUT_DIR}/${dataset}_Chunk"*.root)
    shopt -u nullglob
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "ERROR: no chunks found for ${dataset}" >&2
        exit 1
    fi

    # Remove only temporary outputs left by an interrupted earlier merge.
    shopt -s nullglob
    staleTemporaryFiles=("${OUTPUT_DIR}/${dataset}.tmp."*.root
                         "${OUTPUT_DIR}/${dataset}.copy."*.root)
    shopt -u nullglob
    if [[ ${#staleTemporaryFiles[@]} -gt 0 ]]; then
        echo "Removing ${#staleTemporaryFiles[@]} stale temporary merge file(s) for ${dataset}"
        rm -- "${staleTemporaryFiles[@]}"
    fi

    # Check chunk completeness when the expected job count is known.
    if [[ -n "${EXPECTED_CHUNKS}" ]]; then
        declare -A present=()
        prefix="${OUTPUT_DIR}/${dataset}_Chunk"
        for file in "${files[@]}"; do
            chunk="${file#"${prefix}"}"
            chunk="${chunk%.root}"
            if [[ "${chunk}" =~ ^[0-9]+$ ]]; then
                present["${chunk}"]=1
            fi
        done
        missing=()
        for ((chunk = 0; chunk < EXPECTED_CHUNKS; ++chunk)); do
            if [[ -z "${present[${chunk}]:-}" ]]; then
                missing+=("${chunk}")
            fi
        done
        if [[ ${#missing[@]} -gt 0 ]]; then
            echo "ERROR: ${dataset} is missing ${#missing[@]} expected chunks:" >&2
            printf ' %s' "${missing[@]}" >&2
            printf '\n' >&2
            exit 1
        fi
    fi

    target="${OUTPUT_DIR}/${dataset}.root"
    echo "Merging ${#files[@]} chunks for ${dataset} -> ${target}"
    # Do not use hadd -k: a corrupt input should stop the final merge.
    # If metadata conflicts cause hadd to fail, switch to "hadd -f -k".
    temporary="${HADD_LOCAL_TEMP}/${dataset}.tmp.$$.root"
    if ! hadd "${HADD_PARALLEL_ARGS[@]}" -f "${temporary}" "${files[@]}"; then
        rm -f -- "${temporary}"
        echo "ERROR: hadd failed for ${dataset}; chunk files were kept" >&2
        exit 1
    fi
    if ! rootls "${temporary}" >/dev/null; then
        rm -f -- "${temporary}"
        echo "ERROR: merged ROOT validation failed for ${dataset}; chunk files were kept" >&2
        exit 1
    fi
    if [[ ! -x "${FRAGMENTATION_TOOL}" ]]; then
        rm -f -- "${temporary}"
        echo "ERROR: missing ${FRAGMENTATION_TOOL}; run ./compile.sh --target fragmentation" >&2
        exit 1
    fi
    if ! "${FRAGMENTATION_TOOL}" "${temporary}"; then
        rm -f -- "${temporary}"
        echo "ERROR: fragmentation-function construction failed for ${dataset}" >&2
        exit 1
    fi
    if ! publish_root_file "${temporary}" "${target}"; then
        rm -f -- "${temporary}"
        echo "ERROR: failed to publish validated ROOT file for ${dataset}" >&2
        exit 1
    fi
    rm -f -- "${temporary}"
    MERGED_FILES+=("${target}")
done

if [[ -n "${COMBINE_OUTPUT}" ]]; then
    if [[ "${COMBINE_OUTPUT}" != /* ]]; then
        COMBINE_OUTPUT="${OUTPUT_DIR}/${COMBINE_OUTPUT}"
    fi
    echo "Combining ${#MERGED_FILES[@]} datasets -> ${COMBINE_OUTPUT}"
    combineName="${COMBINE_OUTPUT##*/}"
    temporary="${HADD_LOCAL_TEMP}/${combineName%.root}.tmp.$$.root"
    if ! hadd "${HADD_PARALLEL_ARGS[@]}" -f "${temporary}" "${MERGED_FILES[@]}" || \
       ! rootls "${temporary}" >/dev/null; then
        rm -f -- "${temporary}"
        echo "ERROR: combined ROOT creation failed; chunk files were kept" >&2
        exit 1
    fi
    if ! "${FRAGMENTATION_TOOL}" "${temporary}"; then
        rm -f -- "${temporary}"
        echo "ERROR: fragmentation-function construction failed for combined output" >&2
        exit 1
    fi
    if ! publish_root_file "${temporary}" "${COMBINE_OUTPUT}"; then
        rm -f -- "${temporary}"
        echo "ERROR: failed to publish validated combined ROOT file" >&2
        exit 1
    fi
    rm -f -- "${temporary}"
    echo "Merge complete: ${#MERGED_FILES[@]} dataset files plus ${COMBINE_OUTPUT}"
else
    echo "Merge complete: kept ${#MERGED_FILES[@]} dataset result(s) separate."
fi

if [[ ${CLEANUP} -eq 1 ]]; then
    removed=0
    for dataset in "${DATASETS[@]}"; do
        shopt -s nullglob
        files=("${OUTPUT_DIR}/${dataset}_Chunk"*.root)
        shopt -u nullglob
        if [[ ${#files[@]} -gt 0 ]]; then
            echo "Removing ${#files[@]} merged chunks for ${dataset}"
            rm -- "${files[@]}"
            removed=$((removed + ${#files[@]}))
        fi
    done
    echo "Cleanup complete: removed ${removed} chunk file(s)."
fi
