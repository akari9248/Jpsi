#include "EECCommon.h"
#include "include/MCJetsAndDaughters.h"

#include <TChain.h>
#include <TFile.h>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using unified_eec::Candidate;
using unified_eec::Histograms;
using unified_eec::Jet;
using unified_eec::Particle;
using unified_eec::Settings;

namespace {

struct Options {
  std::string inputDirectory;
  std::string outputBase;
  std::string level = "both";
  int chunks = 1;
  int chunkIndex = 0;
  bool applyCmsFilters = true;
  bool applyHotZone = true;
  bool requirePrompt = true;
  Settings settings;
};

void usage(const char *program) {
  std::cerr
      << "Usage: " << program
      << " -i INPUT_DIR -o OUTPUT_BASE [-n CHUNKS] [-e CHUNK] [-r RADIUS]\n"
      << "       [--level gen|reco|both] [--cms-filters on|off]\n"
      << "       [--hot-zone on|off] [--prompt on|off] [--jet-pt-min X]\n"
      << "       [--jet-eta-max X] [--muon-leading-pt X]\n"
      << "       [--muon-subleading-pt X] [--mass-min X] [--mass-max X]\n"
      << "       [--constituent-scaling on|off]\n";
}

bool parseOnOff(const std::string &value) {
  if (value == "on" || value == "true" || value == "1")
    return true;
  if (value == "off" || value == "false" || value == "0")
    return false;
  throw std::invalid_argument("expected on/off, got " + value);
}

double candidateTargetMass(const Settings &settings) {
  constexpr double nominalJpsiMass = 3.0969;
  if (settings.massMin <= nominalJpsiMass &&
      nominalJpsiMass <= settings.massMax)
    return nominalJpsiMass;
  return 0.5 * (settings.massMin + settings.massMax);
}

Options parseOptions(int argc, char **argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto value = [&]() -> std::string {
      if (i + 1 >= argc)
        throw std::invalid_argument("missing value after " + arg);
      return argv[++i];
    };

    if (arg == "-i" || arg == "--input")
      options.inputDirectory = value();
    else if (arg == "-o" || arg == "--output")
      options.outputBase = value();
    else if (arg == "-n" || arg == "--chunks")
      options.chunks = std::stoi(value());
    else if (arg == "-e" || arg == "--chunk-id")
      options.chunkIndex = std::stoi(value());
    else if (arg == "-r" || arg == "--radius")
      options.settings.radius = std::stod(value());
    else if (arg == "--level")
      options.level = value();
    else if (arg == "--cms-filters")
      options.applyCmsFilters = parseOnOff(value());
    else if (arg == "--hot-zone")
      options.applyHotZone = parseOnOff(value());
    else if (arg == "--prompt")
      options.requirePrompt = parseOnOff(value());
    else if (arg == "--jet-pt-min")
      options.settings.jetPtMin = std::stod(value());
    else if (arg == "--jet-eta-max")
      options.settings.jetAbsEtaMax = std::stod(value());
    else if (arg == "--muon-leading-pt")
      options.settings.leadingMuonPtMin = std::stod(value());
    else if (arg == "--muon-subleading-pt")
      options.settings.subleadingMuonPtMin = std::stod(value());
    else if (arg == "--mass-min")
      options.settings.massMin = std::stod(value());
    else if (arg == "--mass-max")
      options.settings.massMax = std::stod(value());
    else if (arg == "--constituent-scaling")
      options.settings.scaleConstituentsToJetPt = parseOnOff(value());
    else if (arg == "-h" || arg == "--help") {
      usage(argv[0]);
      std::exit(0);
    } else {
      throw std::invalid_argument("unknown option " + arg);
    }
  }
  if (options.inputDirectory.empty() || options.outputBase.empty())
    throw std::invalid_argument("input and output are required");
  if (options.level != "gen" && options.level != "reco" &&
      options.level != "both")
    throw std::invalid_argument("level must be gen, reco, or both");
  if (!std::isfinite(options.settings.massMin) ||
      !std::isfinite(options.settings.massMax) ||
      options.settings.massMin < 0.0 ||
      options.settings.massMin >= options.settings.massMax)
    throw std::invalid_argument(
        "mass window must satisfy 0 <= mass-min < mass-max");
  return options;
}

bool selectRecoCandidate(const MCJetsAndDaughters &event,
                         const Options &options, Candidate &candidate) {
  double bestMassDifference = std::numeric_limits<double>::max();
  const double targetMass = candidateTargetMass(options.settings);
  bool found = false;
  for (std::size_t i = 0; i < event.RecoDiMuonMass->size(); ++i) {
    const double mass = event.RecoDiMuonMass->at(i);
    if (mass < options.settings.massMin || mass > options.settings.massMax)
      continue;
    if (event.RecoDiMuonvProb->at(i) < 0.01)
      continue;
    if (options.requirePrompt &&
        std::abs(event.RecoDiMuonppdlPV->at(i) * 1e4) > 50.0)
      continue;

    const int firstIndex = event.RecoDiMuonDaughter1->at(i);
    const int secondIndex = event.RecoDiMuonDaughter2->at(i);
    if (firstIndex < 0 || secondIndex < 0 ||
        firstIndex >= static_cast<int>(event.RecoMuonPt->size()) ||
        secondIndex >= static_cast<int>(event.RecoMuonPt->size()))
      continue;
    if (!event.RecoMuonSoftID->at(firstIndex) ||
        !event.RecoMuonSoftID->at(secondIndex))
      continue;
    if (event.RecoMuonCharge->at(firstIndex) +
            event.RecoMuonCharge->at(secondIndex) !=
        0)
      continue;

    TLorentzVector first;
    TLorentzVector second;
    first.SetPtEtaPhiE(event.RecoMuonPt->at(firstIndex),
                       event.RecoMuonEta->at(firstIndex),
                       event.RecoMuonPhi->at(firstIndex),
                       event.RecoMuonEnergy->at(firstIndex));
    second.SetPtEtaPhiE(event.RecoMuonPt->at(secondIndex),
                        event.RecoMuonEta->at(secondIndex),
                        event.RecoMuonPhi->at(secondIndex),
                        event.RecoMuonEnergy->at(secondIndex));

    const double difference = std::abs(mass - targetMass);
    if (difference >= bestMassDifference)
      continue;
    bestMassDifference = difference;
    candidate.jpsi.SetPtEtaPhiE(
        event.RecoDiMuonPt->at(i), event.RecoDiMuonEta->at(i),
        event.RecoDiMuonPhi->at(i), event.RecoDiMuonEnergy->at(i));
    candidate.muPlus =
        event.RecoMuonCharge->at(firstIndex) > 0 ? first : second;
    candidate.muMinus =
        event.RecoMuonCharge->at(firstIndex) < 0 ? first : second;
    found = true;
  }
  return found;
}

bool selectGenCandidate(const MCJetsAndDaughters &event,
                        const Options &options, Candidate &candidate) {
  double bestMassDifference = std::numeric_limits<double>::max();
  const double targetMass = candidateTargetMass(options.settings);
  bool found = false;
  for (std::size_t i = 0; i < event.GenMuonPt->size(); ++i) {
    for (std::size_t j = i + 1; j < event.GenMuonPt->size(); ++j) {
      if (event.GenMuonCharge->at(i) + event.GenMuonCharge->at(j) != 0)
        continue;
      TLorentzVector first;
      TLorentzVector second;
      first.SetPtEtaPhiE(event.GenMuonPt->at(i), event.GenMuonEta->at(i),
                         event.GenMuonPhi->at(i), event.GenMuonEnergy->at(i));
      second.SetPtEtaPhiE(event.GenMuonPt->at(j), event.GenMuonEta->at(j),
                          event.GenMuonPhi->at(j), event.GenMuonEnergy->at(j));
      const TLorentzVector dimuon = first + second;
      if (dimuon.M() < options.settings.massMin ||
          dimuon.M() > options.settings.massMax)
        continue;
      const double difference = std::abs(dimuon.M() - targetMass);
      if (difference >= bestMassDifference)
        continue;
      bestMassDifference = difference;
      candidate.jpsi = dimuon;
      candidate.muPlus = event.GenMuonCharge->at(i) > 0 ? first : second;
      candidate.muMinus = event.GenMuonCharge->at(i) < 0 ? first : second;
      found = true;
    }
  }
  return found;
}

std::vector<Jet> buildJets(const std::vector<double> &jetPt,
                           const std::vector<double> &jetEta,
                           const std::vector<double> &jetPhi,
                           const std::vector<double> &jetEnergy,
                           const std::vector<double> &daughterPt,
                           const std::vector<double> &daughterEta,
                           const std::vector<double> &daughterPhi,
                           const std::vector<double> &daughterEnergy,
                           const std::vector<int> &daughterJetId,
                           const std::vector<int> &daughterCharge,
                           const std::vector<int> &daughterPdgId,
                           const std::vector<bool> *passHotZone,
                           const Candidate &candidate, const Settings &settings,
                           Histograms &histograms, double eventWeight) {
  std::vector<Jet> result;
  for (std::size_t i = 0; i < jetPt.size(); ++i) {
    if (jetPt.at(i) < settings.jetPtMin ||
        std::abs(jetEta.at(i)) > settings.jetAbsEtaMax)
      continue;
    if (passHotZone && !passHotZone->at(i))
      continue;

    Jet jet;
    jet.sourceIndex = static_cast<int>(i);
    jet.p4.SetPtEtaPhiE(jetPt.at(i), jetEta.at(i), jetPhi.at(i),
                        jetEnergy.at(i));
    TLorentzVector daughterSum;
    for (std::size_t j = 0; j < daughterPt.size(); ++j) {
      if (daughterJetId.at(j) != static_cast<int>(i))
        continue;
      Particle particle;
      particle.sourceIndex = static_cast<int>(j);
      particle.charge = daughterCharge.at(j);
      particle.pdgId = daughterPdgId.at(j);
      particle.p4.SetPtEtaPhiE(daughterPt.at(j), daughterEta.at(j),
                               daughterPhi.at(j), daughterEnergy.at(j));
      particle.isSelectedJpsiDaughter =
          unified_eec::matchesMuon(particle.p4, particle.charge, particle.pdgId,
                                   candidate.muPlus, +1, -13, settings) ||
          unified_eec::matchesMuon(particle.p4, particle.charge, particle.pdgId,
                                   candidate.muMinus, -1, +13, settings);
      daughterSum += particle.p4;
      jet.constituents.push_back(particle);
    }

    const double scale =
        unified_eec::constituentScale(jet.p4, daughterSum, settings);
    unified_eec::scaleConstituents(jet, scale);
    histograms.fillConstituentScale(scale, eventWeight);
    result.push_back(std::move(jet));
  }
  std::sort(result.begin(), result.end(),
            [](const Jet &left, const Jet &right) {
              return left.p4.Pt() > right.p4.Pt();
            });
  return result;
}

bool passesCmsFilters(const MCJetsAndDaughters &event) {
  if (!event.TriggerBits || event.TriggerBits->size() < 3 ||
      !event.TriggerBits->at(event.TriggerBits->size() - 3))
    return false;
  if (!event.MetFilterBits)
    return false;
  return std::all_of(event.MetFilterBits->begin(), event.MetFilterBits->end(),
                     [](bool pass) { return pass; });
}

} // namespace

int main(int argc, char **argv) {
  try {
    const Options options = parseOptions(argc, argv);
    const auto files = unified_eec::rootFiles(options.inputDirectory);
    if (files.empty())
      throw std::runtime_error("no ROOT files found in " +
                               options.inputDirectory);
    const auto [begin, end] = unified_eec::chunkRange(
        static_cast<int>(files.size()), options.chunks, options.chunkIndex);
    if (begin == end) {
      std::cout << "Chunk " << options.chunkIndex << " has no files\n";
      return 0;
    }

    TChain probe("JetsAndDaughters");
    probe.Add(files.at(begin).c_str());
    const bool isMc = probe.GetBranch("GeneratorWeight") != nullptr;
    const bool runGen =
        isMc && (options.level == "gen" || options.level == "both");
    const bool runReco = options.level == "reco" || options.level == "both";
    if (options.level == "gen" && !isMc)
      throw std::runtime_error("--level gen requested for a data-like tree");

    std::unique_ptr<Histograms> genHistograms;
    std::unique_ptr<Histograms> recoHistograms;
    if (runGen)
      genHistograms =
          std::make_unique<Histograms>("CmsGen", options.settings);
    if (runReco)
      recoHistograms =
          std::make_unique<Histograms>("CmsReco", options.settings);

    long long processed = 0;
    long long acceptedGen = 0;
    long long acceptedReco = 0;
    for (int fileIndex = begin; fileIndex < end; ++fileIndex) {
      TChain chain("JetsAndDaughters");
      chain.Add(files.at(fileIndex).c_str());
      MCJetsAndDaughters event(&chain);
      const Long64_t entries = chain.GetEntries();
      for (Long64_t entry = 0; entry < entries; ++entry) {
        chain.GetEntry(entry);
        ++processed;
        const double weight = isMc ? event.GeneratorWeight : 1.0;
        if (genHistograms)
          genHistograms->fillCut(1, weight);
        if (recoHistograms)
          recoHistograms->fillCut(1, weight);
        if (options.applyCmsFilters && !passesCmsFilters(event))
          continue;

        if (genHistograms) {
          Candidate candidate;
          if (selectGenCandidate(event, options, candidate)) {
            genHistograms->fillCut(2, weight);
            if (unified_eec::candidatePassesKinematics(candidate,
                                                       options.settings)) {
              const auto jets = buildJets(
                  *event.GenJetPt, *event.GenJetEta, *event.GenJetPhi,
                  *event.GenJetEnergy, *event.GenDaughterPt,
                  *event.GenDaughterEta, *event.GenDaughterPhi,
                  *event.GenDaughterEnergy, *event.GenDaughterJetId,
                  *event.GenDaughterCharge, *event.GenDaughterPdgId, nullptr,
                  candidate, options.settings, *genHistograms, weight);
              if (unified_eec::analyzeEvent(candidate, jets, weight,
                                            options.settings, *genHistograms))
                ++acceptedGen;
            }
          }
        }

        if (recoHistograms) {
          Candidate candidate;
          if (selectRecoCandidate(event, options, candidate)) {
            recoHistograms->fillCut(2, weight);
            if (unified_eec::candidatePassesKinematics(candidate,
                                                       options.settings)) {
              const std::vector<bool> *hotZone =
                  options.applyHotZone ? event.RecoJetPassHotZone : nullptr;
              const auto jets = buildJets(
                  *event.RecoJetPt, *event.RecoJetEta, *event.RecoJetPhi,
                  *event.RecoJetEnergy, *event.RecoDaughterPt,
                  *event.RecoDaughterEta, *event.RecoDaughterPhi,
                  *event.RecoDaughterEnergy, *event.RecoDaughterJetId,
                  *event.RecoDaughterCharge, *event.RecoDaughterPdgId, hotZone,
                  candidate, options.settings, *recoHistograms, weight);
              if (unified_eec::analyzeEvent(candidate, jets, weight,
                                            options.settings, *recoHistograms))
                ++acceptedReco;
            }
          }
        }
      }
    }

    const std::string outputName =
        unified_eec::outputFileName(options.outputBase, options.chunkIndex);
    std::unique_ptr<TFile> output(TFile::Open(outputName.c_str(), "RECREATE"));
    if (!output || output->IsZombie())
      throw std::runtime_error("cannot create " + outputName);
    if (genHistograms)
      genHistograms->write(*output);
    if (recoHistograms)
      recoHistograms->write(*output);
    output->Close();
    std::cout << "Processed " << processed << ", accepted Gen " << acceptedGen
              << ", accepted Reco " << acceptedReco << ", wrote " << outputName
              << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    usage(argv[0]);
    return 1;
  }
}
