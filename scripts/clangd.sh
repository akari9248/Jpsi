#!/bin/bash
# clangd needs the same runtime libraries as the CMSSW compiler it queries.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
CMSSW_PATH="${CMSSW_BASE:-$(cd "${PROJECT_DIR}/../.." && pwd)}"

if [[ ! -d "${CMSSW_PATH}/src" ]]; then
    echo "ERROR: invalid CMSSW path: ${CMSSW_PATH}" >&2
    exit 1
fi
# shellcheck source=/dev/null
source /cvmfs/cms.cern.ch/cmsset_default.sh
pushd "${CMSSW_PATH}/src" >/dev/null
eval "$(scramv1 runtime -sh)"
popd >/dev/null

exec /usr/bin/clangd "$@"
