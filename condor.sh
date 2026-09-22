#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMSSW_PATH="${CMSSW_BASE:-$(cd "${SCRIPT_DIR}/../.." && pwd)}"
MODE="cms"
INPUT_BASE=""
OUTPUT_DIR=""
CHUNKS=100
RADIUS=0.4
JET_PT_MIN=30
JET_ETA_MAX=5
MUON_LEADING_PT=2
MUON_SUBLEADING_PT=2
MASS_MIN=2.9
MASS_MAX=3.3
CONSTITUENT_SCALING="on"
LEVEL="both"
CMS_FILTERS="on"
HOT_ZONE="on"
PROMPT="off"
FLAVOUR="microcentury"
MAX_RUNTIME=""
JOB_IO="on"
AUTO_SUBMIT=0
DATASETS=()
EVENTS=500
SEED_START=100000000
ONIASHOWER="off"
CRMODE=0
JOBTAG="private_pythia"

usage() {
    cat <<USAGE
Usage: $0 --mode generate|private|cms [options]

  --input-base DIR       Parent directory containing datasets
  --output-dir DIR       Destination for ROOT files
  --dataset NAME         Dataset subdirectory; may be repeated
  --chunks N             Number of Condor jobs per dataset (default: 100)
  --radius R             Common anti-kT/association radius (default: 0.4)
  --jet-pt-min X         Common selected-jet threshold (default: 30)
  --jet-eta-max X        Common selected-jet acceptance (default: 5)
  --muon-leading-pt X    Common leading-muon threshold (default: 2)
  --muon-subleading-pt X Common subleading-muon threshold (default: 2)
  --mass-min X           Dimuon mass-window lower edge (default: 2.9)
  --mass-max X           Dimuon mass-window upper edge (default: 3.3)
  --constituent-scaling on|off
  --level gen|reco|both  CMS mode only (default: both)
  --cms-filters on|off   CMS trigger and MET filters (default: on)
  --hot-zone on|off      CMS reco hot-zone filter (default: on)
  --prompt on|off        CMS prompt-J/psi selection (default: off)
  --flavour NAME         HTCondor JobFlavour
  --max-runtime SECONDS  Exact HTCondor runtime limit; overrides --flavour
  --job-io on|off        Write per-job stdout/stderr files (default: on)
  --cmssw DIR            CMSSW release directory
  --submit               Submit; otherwise only generate .sub files

Generation-only options:
  --events N             Accepted J/psi events per job (default: 500)
  --seed-start N         First deterministic Pythia seed (default: 100000000)
  --oniashower on|off    Pythia OniaShower setting (default: off)
  --crmode 0|1           Pythia colour-reconnection mode (default: 0)
  --jobtag NAME          Name used for the generated submit file
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode) MODE="$2"; shift 2 ;;
        --input-base) INPUT_BASE="$2"; shift 2 ;;
        --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
        --dataset) DATASETS+=("$2"); shift 2 ;;
        --chunks) CHUNKS="$2"; shift 2 ;;
        --radius) RADIUS="$2"; shift 2 ;;
        --jet-pt-min) JET_PT_MIN="$2"; shift 2 ;;
        --jet-eta-max) JET_ETA_MAX="$2"; shift 2 ;;
        --muon-leading-pt) MUON_LEADING_PT="$2"; shift 2 ;;
        --muon-subleading-pt) MUON_SUBLEADING_PT="$2"; shift 2 ;;
        --mass-min) MASS_MIN="$2"; shift 2 ;;
        --mass-max) MASS_MAX="$2"; shift 2 ;;
        --constituent-scaling) CONSTITUENT_SCALING="$2"; shift 2 ;;
        --level) LEVEL="$2"; shift 2 ;;
        --cms-filters) CMS_FILTERS="$2"; shift 2 ;;
        --hot-zone) HOT_ZONE="$2"; shift 2 ;;
        --prompt) PROMPT="$2"; shift 2 ;;
        --flavour) FLAVOUR="$2"; shift 2 ;;
        --max-runtime) MAX_RUNTIME="$2"; shift 2 ;;
        --job-io) JOB_IO="$2"; shift 2 ;;
        --cmssw) CMSSW_PATH="$2"; shift 2 ;;
        --events) EVENTS="$2"; shift 2 ;;
        --seed-start) SEED_START="$2"; shift 2 ;;
        --oniashower) ONIASHOWER="$2"; shift 2 ;;
        --crmode) CRMODE="$2"; shift 2 ;;
        --jobtag) JOBTAG="$2"; shift 2 ;;
        --submit) AUTO_SUBMIT=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ "${MODE}" != "cms" && "${MODE}" != "private" && \
      "${MODE}" != "generate" ]]; then
    echo "ERROR: mode must be cms, private, or generate" >&2
    exit 1
fi
if [[ -n "${MAX_RUNTIME}" && \
      ( ! "${MAX_RUNTIME}" =~ ^[0-9]+$ || "${MAX_RUNTIME}" -le 0 ) ]]; then
    echo "ERROR: max-runtime must be a positive integer" >&2
    exit 1
fi
if [[ "${JOB_IO}" != "on" && "${JOB_IO}" != "off" ]]; then
    echo "ERROR: job-io must be on or off" >&2
    exit 1
fi

if [[ -n "${MAX_RUNTIME}" ]]; then
    RUNTIME_CLASSAD="+MaxRuntime = ${MAX_RUNTIME}"
else
    RUNTIME_CLASSAD="+JobFlavour = \"${FLAVOUR}\""
fi
# Compile once on the submit host. Batch workers use this prebuilt binary and
# therefore do not depend on SCRAM being able to initialize inside the slot.
echo "Building ${MODE} executable before creating Condor jobs"
"${SCRIPT_DIR}/compile.sh" --cmssw "${CMSSW_PATH}" --target "${MODE}" \
    --output-dir "${SCRIPT_DIR}/bin"

PYTHIA8_DATA=""
LHAPDF_LIB=""
LHAPDF_DATA=""
if [[ "${MODE}" == "generate" ]]; then
    PYTHIA8_BASE="$(
        # shellcheck source=/dev/null
        source /cvmfs/cms.cern.ch/cmsset_default.sh
        cd "${CMSSW_PATH}/src"
        scram tool info pythia8 | awk -F= '/^PYTHIA8_BASE=/{print $2; exit}'
    )"
    LHAPDF_BASE="$(
        # shellcheck source=/dev/null
        source /cvmfs/cms.cern.ch/cmsset_default.sh
        cd "${CMSSW_PATH}/src"
        scram tool info lhapdf | awk -F= '/^LHAPDF_BASE=/{print $2; exit}'
    )"
    PYTHIA8_DATA="${PYTHIA8_BASE}/share/Pythia8/xmldoc"
    LHAPDF_LIB="${LHAPDF_BASE}/lib"
    LHAPDF_DATA="${LHAPDF_BASE}/share/LHAPDF"
    if [[ ! -f "${PYTHIA8_DATA}/Index.xml" || ! -d "${LHAPDF_LIB}" || \
          ! -d "${LHAPDF_DATA}" ]]; then
        echo "ERROR: failed to resolve Pythia/LHAPDF runtime paths" >&2
        exit 1
    fi
fi

TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
GENERATED_DIR="${SCRIPT_DIR}/generated/${MODE}_${TIMESTAMP}"
LOG_DIR="${GENERATED_DIR}/logs"

if [[ "${JOB_IO}" == "on" ]]; then
    JOB_OUTPUT="${LOG_DIR}/\$(Cluster).\$(Process).out"
    JOB_ERROR="${LOG_DIR}/\$(Cluster).\$(Process).err"
else
    JOB_OUTPUT="/dev/null"
    JOB_ERROR="/dev/null"
fi

if [[ "${MODE}" == "generate" ]]; then
    OUTPUT_DIR="${OUTPUT_DIR:-/eos/cms/store/group/phys_smp/ec/shuangyu/Jpsi/HardQCD_Pt15to7000_Unified}"
    mkdir -p "${LOG_DIR}" "${OUTPUT_DIR}"
    submit_file="${GENERATED_DIR}/${JOBTAG}.sub"

    cat > "${submit_file}" <<SUBMIT
universe = vanilla
getenv = True
executable = ${SCRIPT_DIR}/run_job.sh
transfer_executable = False
arguments = --mode generate --output ${OUTPUT_DIR} --chunk-id \$(Chunk) --seed \$(Seed) --events ${EVENTS} --oniashower ${ONIASHOWER} --crmode ${CRMODE} --cmssw ${CMSSW_PATH} --source-dir ${SCRIPT_DIR} --pythia8-data ${PYTHIA8_DATA} --lhapdf-lib ${LHAPDF_LIB} --lhapdf-data ${LHAPDF_DATA}

output = ${JOB_OUTPUT}
error  = ${JOB_ERROR}
log    = ${LOG_DIR}/${JOBTAG}.log

should_transfer_files = NO
+RequiresAFS = True
${RUNTIME_CLASSAD}
max_retries = 2

queue Seed, Chunk from (
SUBMIT

    for ((index = 0; index < CHUNKS; ++index)); do
        seed=$((SEED_START + index))
        if ((seed < 1 || seed > 900000000)); then
            echo "ERROR: generated Pythia seed ${seed} is outside [1,900000000]" >&2
            exit 1
        fi
        echo "${seed} ${index}" >> "${submit_file}"
    done
    echo ")" >> "${submit_file}"

    echo "Created ${submit_file}"
    echo "Generation: ${CHUNKS} jobs x ${EVENTS} events, OniaShower=${ONIASHOWER}, CR=${CRMODE}"
    echo "Output: ${OUTPUT_DIR}"
    if [[ ${AUTO_SUBMIT} -eq 1 ]]; then
        condor_submit "${submit_file}"
    else
        echo "No jobs submitted. Inspect the file, then run: condor_submit ${submit_file}"
    fi
    exit 0
fi

if [[ "${MODE}" == "cms" ]]; then
    INPUT_BASE="${INPUT_BASE:-/eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK4}"
    OUTPUT_DIR="${OUTPUT_DIR:-/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms}"
    if [[ ${#DATASETS[@]} -eq 0 ]]; then
        DATASETS=(
            "Jpsito2Mu_Bin-PTJpsi-8_TuneCP5_13p6TeV_pythia8-RunIIISummer24"
            "JPsiMuMu_Fil-JPsiNo-2MuPtEta_TuneCP5_13p6TeV_pythia8-evtgen-RunIIISummer24"
            "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15"
            # "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR1_pythia8311-RunIIISummer24_ext1_Private"
            # "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR1_pythia8311-RunIIISummer24_Private"
            # "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-RunIIISummer24_ext1_Private"
            # "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-RunIIISummer24_Private"
        )
    fi
else
    INPUT_BASE="${INPUT_BASE:-/eos/cms/store/group/phys_smp/ec/shuangyu/Jpsi}"
    OUTPUT_DIR="${OUTPUT_DIR:-/eos/user/s/shuangyu/public/Jpsi/eec_unified_private}"
    if [[ ${#DATASETS[@]} -eq 0 ]]; then
        DATASETS=(
            "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8309"
            "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8311"
            "HardQCD_Pt15to7000_Oniaoff_CR1_PYTHIA8311"
            "HardQCD_Pt15to7000_Oniaon_CR0_PYTHIA8311"
            "HardQCD_Pt15to7000_Oniaon_CR1_PYTHIA8311"
        )
    fi
fi

mkdir -p "${LOG_DIR}" "${OUTPUT_DIR}"

COMMON_ARGUMENTS="--radius ${RADIUS} --jet-pt-min ${JET_PT_MIN} --jet-eta-max ${JET_ETA_MAX} --muon-leading-pt ${MUON_LEADING_PT} --muon-subleading-pt ${MUON_SUBLEADING_PT} --constituent-scaling ${CONSTITUENT_SCALING}"
if [[ "${MODE}" == "cms" ]]; then
    COMMON_ARGUMENTS+=" --level ${LEVEL} --cms-filters ${CMS_FILTERS} --hot-zone ${HOT_ZONE} --prompt ${PROMPT} --mass-min ${MASS_MIN} --mass-max ${MASS_MAX}"
fi

echo "Mode=${MODE}, R=${RADIUS}, jet pT>${JET_PT_MIN}, |eta|<${JET_ETA_MAX}"
echo "Muon pT: ${MUON_LEADING_PT}/${MUON_SUBLEADING_PT}; constituent scaling=${CONSTITUENT_SCALING}"
if [[ "${MODE}" == "cms" ]]; then
    echo "Dimuon mass window: ${MASS_MIN}-${MASS_MAX} GeV"
fi
echo "Generated files: ${GENERATED_DIR}"

for dataset in "${DATASETS[@]}"; do
    input_dir="${INPUT_BASE}/${dataset}"
    output_base="${OUTPUT_DIR}/${dataset}"
    submit_file="${GENERATED_DIR}/${dataset}.sub"
    if [[ ! -d "${input_dir}" ]]; then
        echo "WARNING: skipping missing dataset ${input_dir}" >&2
        continue
    fi

    cat > "${submit_file}" <<SUBMIT
universe = vanilla
getenv = True
executable = ${SCRIPT_DIR}/run_job.sh
transfer_executable = False
arguments = --mode ${MODE} --input ${input_dir} --output ${output_base} --chunks ${CHUNKS} --chunk-id \$(Process) --cmssw ${CMSSW_PATH} --source-dir ${SCRIPT_DIR} ${COMMON_ARGUMENTS}

output = ${LOG_DIR}/${dataset}_\$(Process).out
error  = ${LOG_DIR}/${dataset}_\$(Process).err
log    = ${LOG_DIR}/${dataset}.log

should_transfer_files = NO
+RequiresAFS = True
+JobFlavour = "${FLAVOUR}"
max_retries = 2
queue ${CHUNKS}
SUBMIT

    echo "Created ${submit_file}"
    if [[ ${AUTO_SUBMIT} -eq 1 ]]; then
        condor_submit "${submit_file}"
    fi
done

if [[ ${AUTO_SUBMIT} -eq 0 ]]; then
    echo "No jobs submitted. Inspect the files, then run:"
    echo "  for f in ${GENERATED_DIR}/*.sub; do condor_submit \"\${f}\"; done"
fi
