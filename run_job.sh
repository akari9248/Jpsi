#!/bin/bash
# Condor executable script: sets up environment, compiles, and runs

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -i|--input) INPUT_PATH="$2"; shift 2 ;;
        -o|--output) OUTPUT_BASE="$2"; shift 2 ;;
        -n|--chunks) TOTAL_CHUNKS="$2"; shift 2 ;;
        -e|--chunk-id) CHUNK_ID="$2"; shift 2 ;;
        *) shift ;;
    esac
done

# Set up CMS environment
source /cvmfs/cms.cern.ch/cmsset_default.sh
eval `scramv1 runtime -sh`

# Compile
CODENAME="eec"
g++ ${CODENAME}.cpp -o ${CODENAME} $(root-config --cflags --glibs) \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-71044756ca7b67339b95a884df339811/include \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0/include \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0 \
  -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-71044756ca7b67339b95a884df339811/lib \
  -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0/lib \
  -lfastjet -lfastjetplugins -lfastjettools -lsiscone -lsiscone_spherical \
  -lfastjetcontribfragile

# Run the executable
./${CODENAME} -i "${INPUT_PATH}" -o "${OUTPUT_BASE}" -n ${TOTAL_CHUNKS} -e ${CHUNK_ID}
