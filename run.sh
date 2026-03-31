#!/bin/bash
# run_eec_parallel.sh
# Simplified version, closer to your needs
kill $(pgrep -a eec| cut -d " " -f 1)
# Configuration parameters
CODENAME="eec"
INPUT_BASE="/eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK8"
OUTPUT_DIR="/eos/user/s/shuangyu/public/Jpsi/AK8"
BACKUP_DIR="${OUTPUT_DIR}/backup"
LOG_DIR="${OUTPUT_DIR}/log"
CHUNKNUM=15

# Dataset list
PATHS=(
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
)

# Compile the program
echo "Compiling ${CODENAME}.cpp..."
g++ ${CODENAME}.cpp -o ${CODENAME} $(root-config --cflags --glibs) \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-71044756ca7b67339b95a884df339811/include \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0/include \
  -I/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0 \
  -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-71044756ca7b67339b95a884df339811/lib \
  -L/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0/lib \
  -lfastjet -lfastjetplugins -lfastjettools -lsiscone -lsiscone_spherical \
  -lfastjetcontribfragile

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed!"
    exit 1
fi

echo "Compilation successful. Executable created: ./${CODENAME}"

# Set library path for runtime
export LD_LIBRARY_PATH=/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet/3.4.1-71044756ca7b67339b95a884df339811/lib:/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fastjet-contrib/1.051-6b2cbf7b2399385490e165663710d5e0/lib:$LD_LIBRARY_PATH

# Create directories
mkdir -p "${OUTPUT_DIR}"
mkdir -p "${BACKUP_DIR}"
mkdir -p "${LOG_DIR}"

echo "Processing ${#PATHS[@]} datasets"
echo "Using ${CHUNKNUM} parallel chunks per dataset"
echo "=========================================="

# Process each dataset
for ((i=0; i<${#PATHS[@]}; i++)); do
    CURRENT_PATH="${INPUT_BASE}/${PATHS[$i]}"
    
    # Generate safe filename
    SAFE_NAME=$(echo "${PATHS[$i]}" | sed 's/[\/]/_/g; s/[^a-zA-Z0-9_-]/_/g')
    OUTPUT_FILE="${OUTPUT_DIR}/${SAFE_NAME}.root"
    
    echo ""
    echo "======================================================================="
    echo "Processing: [$((i+1))/${#PATHS[@]}] ${PATHS[$i]}"
    echo "Input path: ${CURRENT_PATH}"
    echo "Output file: ${OUTPUT_FILE}"
    echo "======================================================================="
    
    # Check if input path exists
    if [ ! -d "${CURRENT_PATH}" ]; then
        echo "Error: Path does not exist: ${CURRENT_PATH}"
        continue
    fi
    
    # Clean up temporary files
    rm -f "${BACKUP_DIR}/"*.root
    rm -f "${LOG_DIR}/${SAFE_NAME}_"*.log
    
    echo "Starting ${CHUNKNUM} parallel chunks..."
    
    # Process chunks in parallel
    CHUNK_PIDS=()
    for ((CHUNK_INDEX=0; CHUNK_INDEX<${CHUNKNUM}; CHUNK_INDEX++)); do
        LOG_FILE="${LOG_DIR}/${SAFE_NAME}_chunk${CHUNK_INDEX}.log"
        
        nohup ./${CODENAME} \
            -i "${CURRENT_PATH}" \
            -o "${BACKUP_DIR}/${SAFE_NAME}_chunk${CHUNK_INDEX}" \
            -n ${CHUNKNUM} \
            -e ${CHUNK_INDEX} \
            > "${LOG_FILE}" 2>&1 &
        
        PID=$!
        CHUNK_PIDS+=(${PID})
        echo "  Chunk ${CHUNK_INDEX} started (PID: ${PID})"
    done
    
    echo "Waiting for all chunks to complete..."
    
    # Wait for all chunks to finish
    FAILED=0
    for PID in "${CHUNK_PIDS[@]}"; do
        if wait ${PID}; then
            echo "  Chunk ${PID} completed successfully"
        else
            echo "  Chunk ${PID} failed"
            FAILED=1
        fi
    done
    
    if [ ${FAILED} -eq 1 ]; then
        echo "Warning: Some chunks failed, check log files"
    fi
    
    # Check for output files
    PART_FILES=($(ls "${BACKUP_DIR}/${SAFE_NAME}_"*.root 2>/dev/null))
    
    if [ ${#PART_FILES[@]} -eq 0 ]; then
        echo "Error: No output files generated!"
        continue
    fi
    
    echo "Found ${#PART_FILES[@]} output files, starting merge..."
    
    # Create file list
    FILE_LIST="${LOG_DIR}/filelist_${SAFE_NAME}.txt"
    > "${FILE_LIST}"
    for FILE in "${PART_FILES[@]}"; do
        echo "${FILE}" >> "${FILE_LIST}"
    done
    
    # Merge output files
    hadd -f "${OUTPUT_FILE}" @"${FILE_LIST}"
    HADD_STATUS=$?
    
    # Clean up
    rm -f "${FILE_LIST}"
    
    if [ ${HADD_STATUS} -eq 0 ] && [ -f "${OUTPUT_FILE}" ]; then
        FILESIZE=$(stat -c%s "${OUTPUT_FILE}")
        echo "Successfully merged: ${OUTPUT_FILE} ($((FILESIZE/1024/1024)) MB)"
        rm -f "${BACKUP_DIR}/${SAFE_NAME}_"*.root
    else
        echo "Error: Merge failed!"
        echo "Temporary files kept in: ${BACKUP_DIR}/"
    fi
    
    echo "Completed processing: ${PATHS[$i]}"
done

echo ""
echo "=========================================="
echo "All datasets processed successfully!"
echo "Results saved in: ${OUTPUT_DIR}/"
echo "=========================================="
