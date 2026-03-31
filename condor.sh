#!/bin/bash
# submit_and_merge.sh
# Compile, submit condor jobs, and generate merge script

source /cvmfs/cms.cern.ch/cmsset_default.sh
eval `scramv1 runtime -sh`

# Configuration
INPUT_BASE="/eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK8"
OUTPUT_BASE="/eos/user/s/shuangyu/public/Jpsi/AK8"
CODENAME="eec"
CHUNKS=50
MEMORY=4096
FLAVOUR="microcentury"

# Dataset list - modify as needed
DATASETS=(
    # "JetMET0_Run2024C-MINIv6NANOv15"
    # "JetMET0_Run2024D-MINIv6NANOv15"
    # "JetMET0_Run2024E-MINIv6NANOv15"
    # "JetMET0_Run2024F-MINIv6NANOv15"
    # "JetMET0_Run2024G-MINIv6NANOv15"
    # "JetMET0_Run2024H-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass1_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass2_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass3_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass4_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass5_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass6_Run2024G-MINIv6NANOv15"
    # "ParkingDoubleMuonLowMass7_Run2024G-MINIv6NANOv15"
    # "Jpsito2Mu_Bin-PTJpsi-8_TuneCP5_13p6TeV_pythia8-RunIIISummer24"
    "QCD_Bin-PT-600to800_pythia8-RunIIISummer24"
    # "QCD_Bin-PT-15to7000_flat2022_pythia8-RunIIISummer24"
    # "QCD_Bin-PT-15to7000_Par-PT-Flat_TuneCH3_13p6TeV_herwig7"
)

# Create the executable script for condor jobs
cat > run_job.sh << 'EOF'
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
EOF

chmod +x run_job.sh

# Create directories
mkdir -p logs
mkdir -p ${OUTPUT_BASE}

# Submit jobs
echo "Submitting ${#DATASETS[@]} datasets with ${CHUNKS} chunks each"

for DATASET in "${DATASETS[@]}"; do
    INPUT_PATH="${INPUT_BASE}/${DATASET}"
    
    condor_submit << EOF
universe = vanilla
executable = run_job.sh
getenv = True
arguments = -i ${INPUT_PATH} -o ${OUTPUT_BASE}/${DATASET} -n ${CHUNKS} -e \$(Process)
transfer_input_files = eec.cpp, run_job.sh, include
output = logs/${DATASET}_\$(Process).out
error = logs/${DATASET}_\$(Process).err
log = logs/${DATASET}.log
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
+JobFlavour = "${FLAVOUR}"
queue ${CHUNKS}
EOF
    
    echo "Submitted ${DATASET}"
done

# Generate merge script
cat > merge.sh << EOF
#!/bin/bash
# merge.sh
# Generated for datasets submitted in the last run

OUTPUT_BASE="${OUTPUT_BASE}"
CHUNKS=${CHUNKS}

# Dataset list from last run
DATASETS=(
EOF

for DATASET in "${DATASETS[@]}"; do
    echo "    \"${DATASET}\"" >> merge.sh
done

cat >> merge.sh << 'EOF'
)

echo "Merging results for ${#DATASETS[@]} datasets"

for DATASET in "${DATASETS[@]}"; do
    OUTPUT_DIR="${OUTPUT_BASE}"
    CHUNK_FILES="${OUTPUT_DIR}/${DATASET}_Chunk*_Part*.root"
    MERGED_FILE="${OUTPUT_DIR}/${DATASET}.root"
    
    # Count chunk files
    NUM_FILES=$(ls ${CHUNK_FILES} 2>/dev/null | wc -l)
    
    echo ""
    echo "Dataset: ${DATASET}"
    echo "  Found ${NUM_FILES} chunk files"
    
    if [ ${NUM_FILES} -eq 0 ]; then
        echo "  WARNING: No chunk files found"
        continue
    fi
    
    if [ -f "${MERGED_FILE}" ]; then
        echo "  Merged file already exists, overwriting"
    fi
    
    # Merge files
    echo "  Merging to ${MERGED_FILE}"
    hadd -f -k ${MERGED_FILE} ${CHUNK_FILES}
    
    if [ $? -eq 0 ]; then
        echo "  Success"
        echo "  Size: $(du -h ${MERGED_FILE} | cut -f1)"
	echo "  Remove chunk/part files"
	rm ${CHUNK_FILES}
    else
        echo "  Failed"
    fi
done

echo ""
echo "Merge completed"
EOF

chmod +x merge.sh

echo ""
echo "========================================"
echo "Submission completed"
echo ""
echo "To check job status:"
echo "  condor_q"
echo ""
echo "To merge results after jobs complete:"
echo "  ./merge.sh"
echo ""
echo "Dataset list:"
for DATASET in "${DATASETS[@]}"; do
    echo "  ${DATASET}"
done
echo "========================================"
