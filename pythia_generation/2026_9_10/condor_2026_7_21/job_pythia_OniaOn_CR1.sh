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

# 加载 CMSSW 环境
echo "Setting up CMSSW environment..."
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /afs/cern.ch/user/s/shuangyu/CMSSW_16_0_4/src
eval `scramv1 runtime -sh`
cd -

# 编译
echo "Compiling pythia.cpp ..."
g++ pythia.cpp -o pythia     -pthread -std=c++20 -m64 -I/cvmfs/cms.cern.ch/el9_amd64_gcc13/cms/cmssw/CMSSW_16_0_4/external/el9_amd64_gcc13/bin/../../../../../../../el9_amd64_gcc13/lcg/root/6.36.07-bd71f88d9cd20e5042c0ac03a7e23595/include  -L/cvmfs/cms.cern.ch/el9_amd64_gcc13/cms/cmssw/CMSSW_16_0_4/external/el9_amd64_gcc13/bin/../../../../../../../el9_amd64_gcc13/lcg/root/6.36.07-bd71f88d9cd20e5042c0ac03a7e23595/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lROOTNTuple -lMultiProc -lROOTDataFrame -lROOTNTupleUtil -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/cms/cmssw/CMSSW_16_0_4/external/el9_amd64_gcc13/bin/../../../../../../../el9_amd64_gcc13/lcg/root/6.36.07-bd71f88d9cd20e5042c0ac03a7e23595/lib -pthread -lm -ldl -rdynamic      -I/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet/3.4.1-8c890eb9b147d65c6806a378b53e3e66/include -I/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet-contrib/1.101-2e1f5ed6ed57897bc3ae238cabbc65d0/include -I/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet-contrib/1.101-2e1f5ed6ed57897bc3ae238cabbc65d0 -I/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/pythia8/316-523edd68e5f59a9312532b6d968361d3/include     -L/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet/3.4.1-8c890eb9b147d65c6806a378b53e3e66/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet-contrib/1.101-2e1f5ed6ed57897bc3ae238cabbc65d0/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/pythia8/316-523edd68e5f59a9312532b6d968361d3/lib -L/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/tbb/v2022.3.0-88eb7be4ee320d604a798a914aea6359/lib     -lfastjet -lfastjetplugins -lfastjettools -lsiscone -lsiscone_spherical -lfastjetcontribfragile -lpythia8 -ltbb     -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/cms/cmssw/CMSSW_16_0_4/external/el9_amd64_gcc13/bin/../../../../../../../el9_amd64_gcc13/lcg/root/6.36.07-bd71f88d9cd20e5042c0ac03a7e23595/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet/3.4.1-8c890eb9b147d65c6806a378b53e3e66/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/fastjet-contrib/1.101-2e1f5ed6ed57897bc3ae238cabbc65d0/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/pythia8/316-523edd68e5f59a9312532b6d968361d3/lib -Wl,-rpath,/cvmfs/cms.cern.ch/el9_amd64_gcc13/external/tbb/v2022.3.0-88eb7be4ee320d604a798a914aea6359/lib     -O2 -Wall -Wextra

# 确保输出目录存在
mkdir -p "$OUTPUTDIR"

# 运行
./pythia --seed $SEED --chunknum $CHUNKNUM --events $EVENTS --outputdir $OUTPUTDIR --oniashower $ONIASHOWER --crmode $CRMODE

echo "Job finished."
