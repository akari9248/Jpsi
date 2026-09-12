#!/bin/bash
set -euo pipefail

SEED=""
CHUNKNUM=""
EVENTS="1000"
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
    echo "ERROR: --seed and --chunknum are required." >&2
    exit 1
fi

# 加载 CMSSW 环境
echo "Setting up CMSSW environment..."
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /afs/cern.ch/user/s/shuangyu/CMSSW_14_0_19/src
eval "$(scramv1 runtime -sh)"
cd -

# 编译（如果已预编译则跳过）
if [ ! -x "pythia" ]; then
    echo "Compiling pythia.cpp ..."
    g++ "pythia.cpp" -o "pythia" \
        -pthread -std=c++17 -m64 -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_0_19/external/el9_amd64_gcc12/bin/../../../../../../../el9_amd64_gcc12/lcg/root/6.30.03-ca7ca986842b225f6fc22ae84d705ed8/include  -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_0_19/external/el9_amd64_gcc12/bin/../../../../../../../el9_amd64_gcc12/lcg/root/6.30.03-ca7ca986842b225f6fc22ae84d705ed8/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_0_19/external/el9_amd64_gcc12/bin/../../../../../../../el9_amd64_gcc12/lcg/root/6.30.03-ca7ca986842b225f6fc22ae84d705ed8/lib -pthread -lm -ldl -rdynamic  \
        -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-60250dbc57e73d9c09d81a3bd414eb4f/include -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-fc86a1f4a81c1f234ddde002a8c76c6f/include -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-fc86a1f4a81c1f234ddde002a8c76c6f -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/pythia8/309-315ecb590794cc881e4b029bda922a92/include \
        -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-60250dbc57e73d9c09d81a3bd414eb4f/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-fc86a1f4a81c1f234ddde002a8c76c6f/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/pythia8/309-315ecb590794cc881e4b029bda922a92/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/tbb/v2021.9.0-d33db04d4520c6ff791eab900054e986/lib \
        -lfastjet -lfastjetplugins -lfastjettools -lsiscone -lsiscone_spherical -lfastjetcontribfragile -lpythia8 -ltbb \
        -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_0_19/external/el9_amd64_gcc12/bin/../../../../../../../el9_amd64_gcc12/lcg/root/6.30.03-ca7ca986842b225f6fc22ae84d705ed8/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-60250dbc57e73d9c09d81a3bd414eb4f/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-fc86a1f4a81c1f234ddde002a8c76c6f/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/pythia8/309-315ecb590794cc881e4b029bda922a92/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/tbb/v2021.9.0-d33db04d4520c6ff791eab900054e986/lib \
        -O2 -Wall -Wextra
else
    echo "Using pre-compiled binary."
fi

# 确保输出目录存在
mkdir -p "$OUTPUTDIR"

# 运行
./pythia --seed "$SEED" --chunknum "$CHUNKNUM" --events "$EVENTS" --outputdir "$OUTPUTDIR" --oniashower "$ONIASHOWER" --crmode "$CRMODE"

echo "Job finished."
