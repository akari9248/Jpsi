#include "EECCommon.h"
#include "include/CharmoniumInfoPrivate.h"
#include "include/MotherCategory.h"

#include <TChain.h>
#include <TFile.h>

#include <fastjet/ClusterSequence.hh>
#include <fastjet/PseudoJet.hh>

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
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
  int chunks = 1;
  int chunkIndex = 0;
  Settings settings;
};

void usage(const char *program) {
  std::cerr
      << "Usage: " << program
      << " -i INPUT_DIR -o OUTPUT_BASE [-n CHUNKS] [-e CHUNK] [-r RADIUS]\n"
      << "       [--jet-pt-min X] [--jet-eta-max X] [--muon-leading-pt X]\n"
      << "       [--muon-subleading-pt X] [--constituent-scaling on|off]\n";
}

bool parseOnOff(const std::string &value) {
  if (value == "on" || value == "true" || value == "1")
    return true;
  if (value == "off" || value == "false" || value == "0")
    return false;
  throw std::invalid_argument("expected on/off, got " + value);
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
    else if (arg == "--jet-pt-min")
      options.settings.jetPtMin = std::stod(value());
    else if (arg == "--jet-eta-max")
      options.settings.jetAbsEtaMax = std::stod(value());
    else if (arg == "--muon-leading-pt")
      options.settings.leadingMuonPtMin = std::stod(value());
    else if (arg == "--muon-subleading-pt")
      options.settings.subleadingMuonPtMin = std::stod(value());
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
  return options;
}

bool selectCandidate(const CharmoniumInfo &event, Candidate &candidate) {
  double bestMassDifference = std::numeric_limits<double>::max();
  bool found = false;
  const std::size_t size = event.hadron_pt->size();
  for (std::size_t i = 0; i < size; ++i) {
    if (!event.hadron_from_jpsi->at(i) ||
        std::abs(event.hadron_pdgid->at(i)) != 13)
      continue;
    for (std::size_t j = i + 1; j < size; ++j) {
      if (!event.hadron_from_jpsi->at(j) ||
          std::abs(event.hadron_pdgid->at(j)) != 13)
        continue;
      const int chargeI =
          static_cast<int>(std::lround(event.hadron_charge->at(i)));
      const int chargeJ =
          static_cast<int>(std::lround(event.hadron_charge->at(j)));
      if (chargeI + chargeJ != 0)
        continue;

      TLorentzVector first;
      TLorentzVector second;
      first.SetPtEtaPhiE(event.hadron_pt->at(i), event.hadron_eta->at(i),
                         event.hadron_phi->at(i), event.hadron_e->at(i));
      second.SetPtEtaPhiE(event.hadron_pt->at(j), event.hadron_eta->at(j),
                          event.hadron_phi->at(j), event.hadron_e->at(j));
      const TLorentzVector dimuon = first + second;
      const double difference = std::abs(dimuon.M() - 3.0969);
      if (difference >= bestMassDifference)
        continue;

      bestMassDifference = difference;
      candidate.jpsi = dimuon;
      candidate.muPlus = chargeI > 0 ? first : second;
      candidate.muMinus = chargeI < 0 ? first : second;
      found = true;
    }
  }
  return found;
}

std::vector<Jet> buildJets(const CharmoniumInfo &event,
                           const Settings &settings, Histograms &histograms,
                           double eventWeight,
                           Histograms *sourceHistograms = nullptr) {
  std::vector<fastjet::PseudoJet> inputs;
  inputs.reserve(event.hadron_pt->size());
  for (std::size_t i = 0; i < event.hadron_pt->size(); ++i) {
    TLorentzVector p4;
    p4.SetPtEtaPhiE(event.hadron_pt->at(i), event.hadron_eta->at(i),
                    event.hadron_phi->at(i), event.hadron_e->at(i));
    fastjet::PseudoJet particle(p4.Px(), p4.Py(), p4.Pz(), p4.E());
    particle.set_user_index(static_cast<int>(i));
    inputs.push_back(particle);
  }

  const fastjet::JetDefinition definition(fastjet::antikt_algorithm,
                                          settings.radius);
  const fastjet::ClusterSequence sequence(inputs, definition);
  const auto clustered =
      fastjet::sorted_by_pt(sequence.inclusive_jets(settings.jetPtMin));

  std::vector<Jet> result;
  for (const auto &inputJet : clustered) {
    if (std::abs(inputJet.eta()) > settings.jetAbsEtaMax)
      continue;

    Jet jet;
    jet.p4.SetPxPyPzE(inputJet.px(), inputJet.py(), inputJet.pz(),
                      inputJet.e());
    TLorentzVector daughterSum;
    for (const auto &inputParticle : inputJet.constituents()) {
      const int index = inputParticle.user_index();
      Particle particle;
      particle.p4.SetPxPyPzE(inputParticle.px(), inputParticle.py(),
                             inputParticle.pz(), inputParticle.e());
      particle.sourceIndex = index;
      particle.charge =
          static_cast<int>(std::lround(event.hadron_charge->at(index)));
      particle.pdgId = event.hadron_pdgid->at(index);
      particle.isSelectedJpsiDaughter =
          event.hadron_from_jpsi->at(index) && std::abs(particle.pdgId) == 13;
      daughterSum += particle.p4;
      jet.constituents.push_back(particle);
    }

    const double scale =
        unified_eec::constituentScale(jet.p4, daughterSum, settings);
    unified_eec::scaleConstituents(jet, scale);
    histograms.fillConstituentScale(scale, eventWeight);
    if (sourceHistograms)
      sourceHistograms->fillConstituentScale(scale, eventWeight);
    result.push_back(std::move(jet));
  }
  return result;
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

    Histograms histograms("PrivateGen");
    std::array<std::unique_ptr<Histograms>, jpsi_origin::numberOfCategories()>
        sourceHistograms;
    for (int category = 0; category < jpsi_origin::numberOfCategories();
         ++category)
      sourceHistograms.at(category) = std::make_unique<Histograms>(
          jpsi_origin::directoryName(category));
    long long processed = 0;
    long long accepted = 0;
    for (int fileIndex = begin; fileIndex < end; ++fileIndex) {
      TChain chain("CharmoniumInfo");
      chain.Add(files.at(fileIndex).c_str());
      CharmoniumInfo event(&chain);
      const Long64_t entries = chain.GetEntries();
      for (Long64_t entry = 0; entry < entries; ++entry) {
        chain.GetEntry(entry);
        ++processed;
        const double weight = event.generatorweight;
        const int sourceCategory = jpsi_origin::category(event.mother_pdgid);
        Histograms &source = *sourceHistograms.at(sourceCategory);
        histograms.fillCut(1, weight);
        source.fillCut(1, weight);

        Candidate candidate;
        if (!selectCandidate(event, candidate))
          continue;
        histograms.fillCut(2, weight);
        source.fillCut(2, weight);
        if (!unified_eec::candidatePassesKinematics(candidate,
                                                    options.settings))
          continue;

        const auto jets = buildJets(event, options.settings, histograms, weight,
                                    &source);
        const bool isAccepted = unified_eec::analyzeEvent(
            candidate, jets, weight, options.settings, histograms);
        unified_eec::analyzeEvent(candidate, jets, weight, options.settings,
                                  source);
        if (isAccepted)
          ++accepted;
      }
    }

    const std::string outputName =
        unified_eec::outputFileName(options.outputBase, options.chunkIndex);
    std::unique_ptr<TFile> output(TFile::Open(outputName.c_str(), "RECREATE"));
    if (!output || output->IsZombie())
      throw std::runtime_error("cannot create " + outputName);
    histograms.write(*output);
    for (auto &source : sourceHistograms)
      source->write(*output);
    output->Close();
    std::cout << "Processed " << processed << ", accepted " << accepted
              << ", wrote " << outputName << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    usage(argv[0]);
    return 1;
  }
}
