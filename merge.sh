#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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
if [[ ! -d "${OUTPUT_DIR}" ]]; then
    echo "ERROR: output directory does not exist: ${OUTPUT_DIR}" >&2
    exit 1
fi

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
    staleTemporaryFiles=("${OUTPUT_DIR}/${dataset}.tmp."*.root)
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
    temporary="${target%.root}.tmp.$$.root"
    if ! hadd -f "${temporary}" "${files[@]}"; then
        rm -f -- "${temporary}"
        echo "ERROR: hadd failed for ${dataset}; chunk files were kept" >&2
        exit 1
    fi
    if ! rootls "${temporary}" >/dev/null; then
        rm -f -- "${temporary}"
        echo "ERROR: merged ROOT validation failed for ${dataset}; chunk files were kept" >&2
        exit 1
    fi
    mv -f -- "${temporary}" "${target}"
    MERGED_FILES+=("${target}")
done

if [[ -n "${COMBINE_OUTPUT}" ]]; then
    if [[ "${COMBINE_OUTPUT}" != /* ]]; then
        COMBINE_OUTPUT="${OUTPUT_DIR}/${COMBINE_OUTPUT}"
    fi
    echo "Combining ${#MERGED_FILES[@]} datasets -> ${COMBINE_OUTPUT}"
    if [[ "${COMBINE_OUTPUT}" == *.root ]]; then
        temporary="${COMBINE_OUTPUT%.root}.tmp.$$.root"
    else
        temporary="${COMBINE_OUTPUT}.tmp.$$.root"
    fi
    if ! hadd -f "${temporary}" "${MERGED_FILES[@]}" || \
       ! rootls "${temporary}" >/dev/null; then
        rm -f -- "${temporary}"
        echo "ERROR: combined ROOT creation failed; chunk files were kept" >&2
        exit 1
    fi
    mv -f -- "${temporary}" "${COMBINE_OUTPUT}"
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