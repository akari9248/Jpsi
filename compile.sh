#!/bin/bash
# Compatibility entry point; implementation lives in scripts/.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${SCRIPT_DIR}/scripts/compile.sh" "$@"
