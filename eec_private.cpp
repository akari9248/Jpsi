#include "EECCommon.h"
#include "include/CharmoniumInfoPrivate.h"
#include "include/MotherCategory.h"

#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

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

constexpr double tagPhotonPtMin = 0.5;
constexpr double tagPionPtMin = 0.4;
constexpr double tagAbsEtaMax = 2.5;
constexpr double chiDeltaMMin = 0.38;
constexpr double chiDeltaMMax = 0.50;
constexpr double psi2SDeltaMMin = 0.55;
constexpr double psi2SDeltaMMax = 0.63;

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

enum PrivateFeeddownTag { kUntagged = 0, kChiC = 1, kPsi2S = 2 };

struct FeeddownTagResult {
  int tag = kUntagged;
  double bestChiDeltaM = -1.0;
  double bestPsi2SDeltaM = -1.0;
};

TLorentzVector stableParticle(const CharmoniumInfo &event, std::size_t index) {
  TLorentzVector result;
  result.SetPtEtaPhiE(event.hadron_pt->at(index), event.hadron_eta->at(index),
                      event.hadron_phi->at(index), event.hadron_e->at(index));
  return result;
}

double normalizedMassPull(double deltaM, double center, double minimum,
                          double maximum) {
  const double scale = deltaM < center ? center - minimum : maximum - center;
  if (scale <= 0.0)
    return std::numeric_limits<double>::max();
  return std::abs(deltaM - center) / scale;
}

FeeddownTagResult reconstructFeeddown(const CharmoniumInfo &event,
                                      const Candidate &candidate,
                                      double maximumDeltaR) {
  FeeddownTagResult result;
  constexpr double chiCenter = 0.435;
  constexpr double psi2SCenter = 0.589;
  double bestChiPull = std::numeric_limits<double>::max();
  double bestPsi2SPull = std::numeric_limits<double>::max();
  std::vector<std::size_t> positivePions;
  std::vector<std::size_t> negativePions;

  for (std::size_t i = 0; i < event.hadron_pt->size(); ++i) {
    const int pdgId = event.hadron_pdgid->at(i);
    const TLorentzVector particle = stableParticle(event, i);
    if (candidate.jpsi.DeltaR(particle) >= maximumDeltaR)
      continue;
    if (pdgId == 22 && event.hadron_pt->at(i) >= tagPhotonPtMin &&
        std::abs(event.hadron_eta->at(i)) <= tagAbsEtaMax) {
      const double deltaM =
          (candidate.jpsi + particle).M() - candidate.jpsi.M();
      const double pull = normalizedMassPull(
          deltaM, chiCenter, chiDeltaMMin, chiDeltaMMax);
      if (pull < bestChiPull) {
        bestChiPull = pull;
        result.bestChiDeltaM = deltaM;
      }
    }
    if (std::abs(pdgId) != 211 ||
        event.hadron_pt->at(i) < tagPionPtMin ||
        std::abs(event.hadron_eta->at(i)) > tagAbsEtaMax)
      continue;
    (pdgId > 0 ? positivePions : negativePions).push_back(i);
  }

  for (const std::size_t positive : positivePions)
    for (const std::size_t negative : negativePions) {
      const double deltaM =
          (candidate.jpsi + stableParticle(event, positive) +
           stableParticle(event, negative))
              .M() -
          candidate.jpsi.M();
      const double pull = normalizedMassPull(
          deltaM, psi2SCenter, psi2SDeltaMMin, psi2SDeltaMMax);
      if (pull < bestPsi2SPull) {
        bestPsi2SPull = pull;
        result.bestPsi2SDeltaM = deltaM;
      }
    }

  const bool chiMatched = bestChiPull <= 1.0;
  const bool psi2SMatched = bestPsi2SPull <= 1.0;
  if (chiMatched && psi2SMatched)
    result.tag = bestChiPull <= bestPsi2SPull ? kChiC : kPsi2S;
  else if (chiMatched)
    result.tag = kChiC;
  else if (psi2SMatched)
    result.tag = kPsi2S;
  return result;
}

bool isBOrigin(const CharmoniumInfo &event) {
  const int category = jpsi_origin::category(event.mother_pdgid);
  return category == 1 ||
         (category == 2 &&
          jpsi_origin::isBHadron(event.grandmother_pdgid));
}

int truthFeeddownLabel(const CharmoniumInfo &event) {
  const int category = jpsi_origin::category(event.mother_pdgid);
  if (isBOrigin(event))
    return 0; // b decay
  const int mother = std::abs(event.mother_pdgid);
  if (mother == 20443 || mother == 445)
    return 1; // chi_c1 or chi_c2
  if (mother == 100443)
    return 2; // psi(2S)
  if (category == 2)
    return 3; // other charmonium feed-down
  return 4;   // direct prompt candidate
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
                           const std::vector<Histograms *> &subHistograms) {
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
    for (Histograms *subHistogram : subHistograms)
      subHistogram->fillConstituentScale(scale, eventWeight);
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
    const std::array<const char *, 3> feeddownTagNames = {
        "PrivateFeeddownTag_untagged", "PrivateFeeddownTag_chi_c",
        "PrivateFeeddownTag_psi_2S"};
    std::array<std::unique_ptr<Histograms>, 3> feeddownTagHistograms;
    for (std::size_t tag = 0; tag < feeddownTagHistograms.size(); ++tag)
      feeddownTagHistograms[tag] =
          std::make_unique<Histograms>(feeddownTagNames[tag]);
    TH1D bestChiDeltaM("private_feeddown_tag_best_chi_delta_m",
                       ";best M(J/#psi#gamma)-M(J/#psi) [GeV];Events", 120,
                       0.0, 1.2);
    TH1D bestPsi2SDeltaM(
        "private_feeddown_tag_best_psi2s_delta_m",
        ";best M(J/#psi#pi^{+}#pi^{-})-M(J/#psi) [GeV];Events", 120, 0.3,
        0.9);
    TH2D migration("private_feeddown_tag_migration",
                   ";Private feed-down tag;Truth origin;Weighted events", 3,
                   -0.5, 2.5, 5, -0.5, 4.5);
    TH2D migrationUnweighted(
        "private_feeddown_tag_migration_unweighted",
        ";Private feed-down tag;Truth origin;Unweighted events", 3, -0.5,
        2.5, 5, -0.5, 4.5);
    TH1D originBeforeBVeto(
        "private_feeddown_tag_origin_before_b_veto",
        ";Truth origin before feed-down B veto;Weighted selected events", 2,
        -0.5, 1.5);
    TH1D originBeforeBVetoUnweighted(
        "private_feeddown_tag_origin_before_b_veto_unweighted",
        ";Truth origin before feed-down B veto;Unweighted selected events", 2,
        -0.5, 1.5);
    originBeforeBVeto.GetXaxis()->SetBinLabel(1, "b origin");
    originBeforeBVeto.GetXaxis()->SetBinLabel(2, "prompt");
    originBeforeBVetoUnweighted.GetXaxis()->SetBinLabel(1, "b origin");
    originBeforeBVetoUnweighted.GetXaxis()->SetBinLabel(2, "prompt");
    migration.GetXaxis()->SetBinLabel(1, "untagged");
    migration.GetXaxis()->SetBinLabel(2, "chi_c tag");
    migration.GetXaxis()->SetBinLabel(3, "psi(2S) tag");
    migration.GetYaxis()->SetBinLabel(1, "b decay");
    migration.GetYaxis()->SetBinLabel(2, "chi_c1/2");
    migration.GetYaxis()->SetBinLabel(3, "psi(2S)");
    migration.GetYaxis()->SetBinLabel(4, "other charmonium");
    migration.GetYaxis()->SetBinLabel(5, "direct prompt");
    for (int bin = 1; bin <= migration.GetXaxis()->GetNbins(); ++bin)
      migrationUnweighted.GetXaxis()->SetBinLabel(
          bin, migration.GetXaxis()->GetBinLabel(bin));
    for (int bin = 1; bin <= migration.GetYaxis()->GetNbins(); ++bin)
      migrationUnweighted.GetYaxis()->SetBinLabel(
          bin, migration.GetYaxis()->GetBinLabel(bin));
    std::array<std::unique_ptr<Histograms>, jpsi_origin::numberOfCategories()>
        sourceHistograms;
    for (int category = 0; category < jpsi_origin::numberOfCategories();
         ++category) {
      if (category == 0 || category == 2)
        continue; // Cat0 is empty; cat2 is reconstructed from its components.
      sourceHistograms.at(category) = std::make_unique<Histograms>(
          jpsi_origin::directoryName(category));
    }
    std::array<std::unique_ptr<Histograms>,
               jpsi_origin::numberOfCharmoniumComponents()>
        charmoniumComponentHistograms;
    for (int component = 0;
         component < jpsi_origin::numberOfCharmoniumComponents(); ++component)
      charmoniumComponentHistograms.at(component) =
          std::make_unique<Histograms>(
              jpsi_origin::charmoniumComponentDirectoryName(component));
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
        std::vector<Histograms *> subHistograms;
        if (sourceCategory == 2) {
          const int component = jpsi_origin::charmoniumComponent(
              event.mother_pdgid, event.grandmother_pdgid);
          subHistograms.push_back(
              charmoniumComponentHistograms.at(component).get());
        } else if (sourceCategory != 0)
          subHistograms.push_back(sourceHistograms.at(sourceCategory).get());
        histograms.fillCut(1, weight);
        for (Histograms *subHistogram : subHistograms)
          subHistogram->fillCut(1, weight);

        Candidate candidate;
        if (!selectCandidate(event, candidate))
          continue;
        FeeddownTagResult feeddownTag;
        const bool bOrigin = isBOrigin(event);
        const bool runFeeddownTagger = !bOrigin;
        if (runFeeddownTagger) {
          feeddownTag =
              reconstructFeeddown(event, candidate, options.settings.radius);
          Histograms *feeddownTagHistogram =
              feeddownTagHistograms.at(feeddownTag.tag).get();
          feeddownTagHistogram->fillCut(1, weight);
          subHistograms.push_back(feeddownTagHistogram);
        }
        histograms.fillCut(2, weight);
        for (Histograms *subHistogram : subHistograms)
          subHistogram->fillCut(2, weight);
        if (!unified_eec::candidatePassesKinematics(candidate,
                                                    options.settings))
          continue;
        if (runFeeddownTagger && feeddownTag.bestChiDeltaM >= 0.0)
          bestChiDeltaM.Fill(feeddownTag.bestChiDeltaM, weight);
        if (runFeeddownTagger && feeddownTag.bestPsi2SDeltaM >= 0.0)
          bestPsi2SDeltaM.Fill(feeddownTag.bestPsi2SDeltaM, weight);

        const auto jets = buildJets(event, options.settings, histograms, weight,
                                    subHistograms);
        for (const auto &jet : jets) {
          histograms.fillInclusiveJet(jet, weight);
          for (Histograms *subHistogram : subHistograms)
            subHistogram->fillInclusiveJet(jet, weight);
        }
        const bool isAccepted = unified_eec::analyzeEvent(
            candidate, jets, weight, options.settings, histograms);
        for (Histograms *subHistogram : subHistograms)
          unified_eec::analyzeEvent(candidate, jets, weight, options.settings,
                                    *subHistogram);
        if (isAccepted) {
          originBeforeBVeto.Fill(bOrigin ? 0 : 1, weight);
          originBeforeBVetoUnweighted.Fill(bOrigin ? 0 : 1);
          if (runFeeddownTagger) {
            migration.Fill(feeddownTag.tag, truthFeeddownLabel(event), weight);
            migrationUnweighted.Fill(feeddownTag.tag,
                                     truthFeeddownLabel(event));
          }
          ++accepted;
        }
      }
    }

    const std::string outputName =
        unified_eec::outputFileName(options.outputBase, options.chunkIndex);
    std::unique_ptr<TFile> output(TFile::Open(outputName.c_str(), "RECREATE"));
    if (!output || output->IsZombie())
      throw std::runtime_error("cannot create " + outputName);
    histograms.write(*output);
    for (auto &source : sourceHistograms)
      if (source)
        source->write(*output);
    for (auto &component : charmoniumComponentHistograms)
      component->write(*output);
    for (auto &tagged : feeddownTagHistograms)
      tagged->write(*output);
    output->mkdir("PrivateFeeddownTagDiagnostics");
    output->cd("PrivateFeeddownTagDiagnostics");
    bestChiDeltaM.Write();
    bestPsi2SDeltaM.Write();
    migration.Write();
    migrationUnweighted.Write();
    originBeforeBVeto.Write();
    originBeforeBVetoUnweighted.Write();
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
