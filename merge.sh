#!/bin/bash
# merge.sh
# Generated for datasets submitted in the last run

OUTPUT_BASE="/eos/user/s/shuangyu/public/Jpsi/AK8"
CHUNKS=50

# Dataset list from last run
DATASETS=(
    "QCD_Bin-PT-600to800_pythia8-RunIIISummer24"
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
