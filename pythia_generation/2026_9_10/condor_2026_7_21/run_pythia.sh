#!/bin/bash
set -e

SEED=""
CHUNKNUM=""
EVENTS="100"
OUTPUTDIR="."
ONIASHOWER="off"
CRMODE="0"

while [[ $# -gt 0 ]]; do
    case $1 in
        --seed) SEED="$2"; shift 2 ;;
        --chunknum) CHUNKNUM="$2"; shift 2 ;;
        --events) EVENTS="$2"; shift 2 ;;
        --outputdir) OUTPUTDIR="$2"; shift 2 ;;
        --oniashower) ONIASHOWER="$2"; shift 2 ;;
        --crmode) CRMODE="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; shift ;;
    esac
done

if [ -z "$SEED" ] || [ -z "$CHUNKNUM" ]; then
    echo "ERROR: --seed and --chunknum are required."
    exit 1
fi

# 设置 CMSSW 环境
export SCRAM_ARCH=el9_amd64_gcc12
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /afs/cern.ch/user/s/shuangyu/CMSSW_15_0_14/src
eval `scramv1 runtime -sh`
cd -

# Pythia8 data
PYTHIA8_BASE=$(scram tool info pythia8 | awk -F= '/^PYTHIA8_BASE=/{print $2; exit}')
export PYTHIA8DATA="${PYTHIA8_BASE}/share/Pythia8/xmldoc"

# 运行
./pythia --seed $SEED --chunknum $CHUNKNUM --events $EVENTS --outputdir $OUTPUTDIR --oniashower $ONIASHOWER --crmode $CRMODE

echo "Job finished."
