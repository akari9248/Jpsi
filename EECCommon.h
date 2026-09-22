#ifndef JPSI_EEC_UNIFIED_COMMON_H
#define JPSI_EEC_UNIFIED_COMMON_H

#include <TDirectory.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLorentzVector.h>
#include <TString.h>

#include <algorithm>
#include <cmath>
#include <glob.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace unified_eec {

struct Settings {
  double radius = 0.4;
  double jetPtMin = 30.0;
  double jetAbsEtaMax = 5.0;
  double jpsiAbsEtaMax = 2.4;
  double muonAbsEtaMax = 2.4;
  double leadingMuonPtMin = 2.0;
  double subleadingMuonPtMin = 2.0;
  double massMin = 2.9;
  double massMax = 3.3;
  double muonMatchDR = 0.01;
  double muonMatchMaxRelPt = 0.5;
  bool scaleConstituentsToJetPt = true;
};

struct Particle {
  TLorentzVector p4;
  int charge = 0;
  int pdgId = 0;
  int sourceIndex = -1;
  bool isSelectedJpsiDaughter = false;
};

struct Jet {
  TLorentzVector p4;
  int sourceIndex = -1;
  std::vector<Particle> constituents;
};

struct Candidate {
  TLorentzVector jpsi;
  TLorentzVector muPlus;
  TLorentzVector muMinus;
};

inline bool candidatePassesKinematics(const Candidate &candidate,
                                      const Settings &settings) {
  if (candidate.jpsi.M() < settings.massMin ||
      candidate.jpsi.M() > settings.massMax ||
      std::abs(candidate.jpsi.Eta()) > settings.jpsiAbsEtaMax)
    return false;

  if (std::abs(candidate.muPlus.Eta()) > settings.muonAbsEtaMax ||
      std::abs(candidate.muMinus.Eta()) > settings.muonAbsEtaMax)
    return false;

  const double leadingPt =
      std::max(candidate.muPlus.Pt(), candidate.muMinus.Pt());
  const double subleadingPt =
      std::min(candidate.muPlus.Pt(), candidate.muMinus.Pt());
  return leadingPt >= settings.leadingMuonPtMin &&
         subleadingPt >= settings.subleadingMuonPtMin;
}

inline bool matchesMuon(const TLorentzVector &particle, int charge, int pdgId,
                        const TLorentzVector &muon, int expectedCharge,
                        int expectedPdgId, const Settings &settings) {
  if (charge != expectedCharge || pdgId != expectedPdgId)
    return false;
  if (particle.DeltaR(muon) > settings.muonMatchDR)
    return false;
  if (muon.Pt() <= 0.0)
    return false;
  return std::abs(particle.Pt() - muon.Pt()) / muon.Pt() <=
         settings.muonMatchMaxRelPt;
}

inline double constituentScale(const TLorentzVector &jet,
                               const TLorentzVector &daughterSum,
                               const Settings &settings) {
  if (!settings.scaleConstituentsToJetPt || daughterSum.Pt() <= 0.0)
    return 1.0;
  return jet.Pt() / daughterSum.Pt();
}

inline void scaleConstituents(Jet &jet, double scale) {
  if (!std::isfinite(scale) || scale <= 0.0)
    throw std::runtime_error("invalid constituent scale");
  for (auto &particle : jet.constituents)
    particle.p4 *= scale;
}

inline int findJpsiJet(const Candidate &candidate, const std::vector<Jet> &jets,
                       const Settings &settings) {
  for (std::size_t i = 0; i < jets.size(); ++i) {
    const Jet &jet = jets[i];
    if (candidate.jpsi.DeltaR(jet.p4) >= settings.radius)
      continue;

    bool hasPlus = false;
    bool hasMinus = false;
    for (const auto &particle : jet.constituents) {
      if (!particle.isSelectedJpsiDaughter)
        continue;
      hasPlus = hasPlus || particle.charge > 0;
      hasMinus = hasMinus || particle.charge < 0;
    }
    if (hasPlus && hasMinus)
      return static_cast<int>(i);
  }
  return -1;
}

// Light-cone longitudinal momentum fraction used in arXiv:1702.03287:
// z_h = p^+_{J/psi} / p^+_{jet}, with p^+ = E + p_parallel and the
// longitudinal axis chosen along the jet direction.
inline double longitudinalMomentumFraction(const TLorentzVector &particle,
                                           const TLorentzVector &jet) {
  if (jet.Vect().Mag2() <= 0.0)
    return std::numeric_limits<double>::quiet_NaN();

  const double numerator =
      particle.E() + particle.Vect().Dot(jet.Vect().Unit());
  const double denominator = jet.E() + jet.P();
  if (!std::isfinite(numerator) || !std::isfinite(denominator) ||
      denominator <= 0.0)
    return std::numeric_limits<double>::quiet_NaN();
  return numerator / denominator;
}

inline const std::vector<double> &jpsiPtBinEdges() {
  static const std::vector<double> edges = {0,  2,  4,  6,  8,   12,
                                            16, 20, 30, 50, 100, 200};
  return edges;
}

inline const std::vector<std::string> &jpsiPtBinSuffixes() {
  static const std::vector<std::string> suffixes = {
      "_jpsipt_0_2",   "_jpsipt_2_4",    "_jpsipt_4_6",     "_jpsipt_6_8",
      "_jpsipt_8_12",  "_jpsipt_12_16",  "_jpsipt_16_20",   "_jpsipt_20_30",
      "_jpsipt_30_50", "_jpsipt_50_100", "_jpsipt_100_200", "_jpsipt_200_Inf"};
  return suffixes;
}

inline int jpsiPtBin(double pt) {
  const auto &edges = jpsiPtBinEdges();
  if (!std::isfinite(pt) || pt < edges.front())
    return -1;
  const auto upper = std::upper_bound(edges.begin(), edges.end(), pt);
  return std::min(static_cast<int>(upper - edges.begin()) - 1,
                  static_cast<int>(edges.size()) - 1);
}

inline std::string jpsiPtSuffix(double pt) {
  const int bin = jpsiPtBin(pt);
  return bin < 0 ? "" : jpsiPtBinSuffixes().at(bin);
}

// Jet-pT bins used for the z = pT(J/psi) / pT(jet) and z_h distributions.
// The entries are lower bin edges; the final bin has no upper bound.
inline const std::vector<double> &momentumFractionJetPtBinEdges() {
  static const std::vector<double> edges = {30.0,  50.0,  100.0, 150.0,
                                            200.0, 300.0, 400.0, 600.0};
  return edges;
}

inline const std::vector<std::string> &momentumFractionJetPtBinSuffixes() {
  static const std::vector<std::string> suffixes = {
      "_jetpt_30_50",   "_jetpt_50_100",  "_jetpt_100_150",
      "_jetpt_150_200", "_jetpt_200_300", "_jetpt_300_400",
      "_jetpt_400_600", "_jetpt_600_Inf"};
  return suffixes;
}

inline int momentumFractionJetPtBin(double pt) {
  const auto &edges = momentumFractionJetPtBinEdges();
  if (!std::isfinite(pt) || pt < edges.front())
    return -1;
  const auto upper = std::upper_bound(edges.begin(), edges.end(), pt);
  return std::min(static_cast<int>(upper - edges.begin()) - 1,
                  static_cast<int>(edges.size()) - 1);
}

inline std::string momentumFractionJetPtSuffix(double pt) {
  const int bin = momentumFractionJetPtBin(pt);
  return bin < 0 ? "" : momentumFractionJetPtBinSuffixes().at(bin);
}

// Jet-pT intervals used for F^{J/psi}(z_h, pT) in arXiv:1702.03287.
// The upper edge is exclusive, so a 200 GeV jet is outside these bins.
inline const std::vector<double> &fragmentationJetPtBinEdges() {
  static const std::vector<double> edges = {50.0, 100.0, 150.0, 200.0};
  return edges;
}

inline const std::vector<std::string> &fragmentationJetPtBinSuffixes() {
  static const std::vector<std::string> suffixes = {
      "_jetpt_50_100", "_jetpt_100_150", "_jetpt_150_200"};
  return suffixes;
}

inline int fragmentationJetPtBin(double pt) {
  const auto &edges = fragmentationJetPtBinEdges();
  if (!std::isfinite(pt) || pt < edges.front() || pt >= edges.back())
    return -1;
  return static_cast<int>(std::upper_bound(edges.begin(), edges.end(), pt) -
                          edges.begin()) -
         1;
}

inline std::string fragmentationJetPtSuffix(double pt) {
  const int bin = fragmentationJetPtBin(pt);
  return bin < 0 ? "" : fragmentationJetPtBinSuffixes().at(bin);
}

class Histograms {
public:
  explicit Histograms(std::string label, const Settings &settings = Settings{})
      : label_(std::move(label)) {
    cutflow_ = make("cutflow", "cutflow", 6, 0.5, 6.5);
    const char *cutLabels[] = {
        "input",         "candidate", "candidate kinematics",
        "selected jets", "J/psi jet", "EEC filled"};
    for (int i = 0; i < 6; ++i)
      cutflow_->GetXaxis()->SetBinLabel(i + 1, cutLabels[i]);

    eventWeight_ = make("event_weight", "event weight", 200, -10.0, 10.0);
    jpsiPt_ = make("jpsi_pt", "J/#psi p_{T}", 200, 0.0, 200.0);
    const double massHistogramMin = std::min(2.6, settings.massMin - 0.1);
    const double massHistogramMax = std::max(3.6, settings.massMax + 0.1);
    jpsiMass_ = make("jpsi_mass", "J/#psi mass", 100, massHistogramMin,
                     massHistogramMax);
    jpsiJetPt_ = make("jpsijet_pt", "J/#psi jet p_{T}", 300, 0.0, 600.0);
    zPt_ = make("z_pt", "p_{T}(J/#psi)/p_{T}(jet)", 60, 0.0, 1.2);
    zH_ = make("z_h", "J/#psi longitudinal momentum fraction;z_{h};weighted "
                      "J/#psi jets",
               60, 0.0, 1.2);
    nSelectedJets_ =
        make("n_selected_jets", "selected jet multiplicity", 20, -0.5, 19.5);
    constituentScale_ =
        make("constituent_scale", "constituent energy scale", 200, 0.0, 2.0);

    addEecSet("alljets");
    addEecSet("jpsijet");

    for (const auto &suffix : jpsiPtBinSuffixes()) {
      jpsiPtBinned_[suffix] =
          make("jpsi_pt" + suffix, "J/#psi p_{T}", 200, 0.0, 200.0);
      jpsiMassBinned_[suffix] =
          make("jpsi_mass" + suffix, "J/#psi mass", 100, massHistogramMin,
               massHistogramMax);
      jpsiJetPtBinned_[suffix] =
          make("jpsijet_pt" + suffix, "J/#psi jet p_{T}", 300, 0.0, 600.0);
    }
    for (const auto &suffix : momentumFractionJetPtBinSuffixes()) {
      zPtBinned_[suffix] =
          make("z_pt" + suffix, "p_{T}(J/#psi)/p_{T}(jet)", 60, 0.0, 1.2);
      zHBinned_[suffix] =
          make("z_h" + suffix,
               "J/#psi longitudinal momentum fraction;z_{h};weighted J/#psi "
               "jets",
               60, 0.0, 1.2);
    }

    // These are intentionally kept as additive (hadd-safe) ingredients.
    // make_fragmentation_function converts them to
    // (1 / N_inclusive-jet) dN_J/psi-jet / dz_h after chunk merging.
    for (const auto &suffix : fragmentationJetPtBinSuffixes()) {
      fragmentationNumerator_[suffix] =
          make("fragmentation_numerator" + suffix,
               "J/#psi-jet numerator;z_{h};weighted J/#psi jets", 60, 0.0,
               1.2);
      inclusiveJetDenominator_[suffix] =
          make("inclusive_jet_denominator" + suffix,
               "inclusive-jet denominator;count;weighted jets", 1, 0.5,
               1.5);
    }
  }

  void fillCut(int bin, double weight = 1.0) { cutflow_->Fill(bin, weight); }

  void fillEvent(const Candidate &candidate, const std::vector<Jet> &jets,
                 double weight) {
    eventWeight_->Fill(weight);
    jpsiPt_->Fill(candidate.jpsi.Pt(), weight);
    jpsiMass_->Fill(candidate.jpsi.M(), weight);
    nSelectedJets_->Fill(jets.size(), weight);
    const std::string suffix = jpsiPtSuffix(candidate.jpsi.Pt());
    if (!suffix.empty()) {
      jpsiPtBinned_.at(suffix)->Fill(candidate.jpsi.Pt(), weight);
      jpsiMassBinned_.at(suffix)->Fill(candidate.jpsi.M(), weight);
    }
  }

  void fillConstituentScale(double scale, double weight) {
    constituentScale_->Fill(scale, weight);
  }

  void fillInclusiveJet(const Jet &jet, double weight) {
    const std::string suffix = fragmentationJetPtSuffix(jet.p4.Pt());
    if (!suffix.empty())
      inclusiveJetDenominator_.at(suffix)->Fill(1.0, weight);
  }

  void fillInclusiveJet(double jetPt, double weight) {
    const std::string suffix = fragmentationJetPtSuffix(jetPt);
    if (!suffix.empty())
      inclusiveJetDenominator_.at(suffix)->Fill(1.0, weight);
  }

  void fillJpsiJet(const Candidate &candidate, const Jet &jet, double weight) {
    jpsiJetPt_->Fill(jet.p4.Pt(), weight);
    const std::string jpsiPtSuffixValue = jpsiPtSuffix(candidate.jpsi.Pt());
    if (!jpsiPtSuffixValue.empty())
      jpsiJetPtBinned_.at(jpsiPtSuffixValue)->Fill(jet.p4.Pt(), weight);
    const std::string momentumFractionSuffix =
        momentumFractionJetPtSuffix(jet.p4.Pt());
    if (jet.p4.Pt() > 0.0) {
      const double zPt = candidate.jpsi.Pt() / jet.p4.Pt();
      zPt_->Fill(zPt, weight);
      if (!momentumFractionSuffix.empty())
        zPtBinned_.at(momentumFractionSuffix)->Fill(zPt, weight);
    }
    const double zH = longitudinalMomentumFraction(candidate.jpsi, jet.p4);
    if (std::isfinite(zH)) {
      zH_->Fill(zH, weight);
      if (!momentumFractionSuffix.empty())
        zHBinned_.at(momentumFractionSuffix)->Fill(zH, weight);
      const std::string jetPtSuffix =
          fragmentationJetPtSuffix(jet.p4.Pt());
      if (!jetPtSuffix.empty())
        fragmentationNumerator_.at(jetPtSuffix)->Fill(zH, weight);
    }
  }

  bool fillParticle(const std::string &scope, const Particle &particle,
                    const Candidate &candidate, double eventWeight) {
    if (particle.isSelectedJpsiDaughter || candidate.jpsi.M() <= 0.0 ||
        candidate.jpsi.Vect().Mag2() <= 0.0)
      return false;

    TLorentzVector rest = particle.p4;
    rest.Boost(-candidate.jpsi.BoostVector());
    if (rest.Vect().Mag2() <= 0.0 || !std::isfinite(rest.E()))
      return false;

    double cosChi = rest.Vect().Unit().Dot(candidate.jpsi.Vect().Unit());
    cosChi = std::clamp(cosChi, -1.0, 1.0);
    const double energyWeight = eventWeight * rest.E() / candidate.jpsi.M();
    if (!std::isfinite(energyWeight))
      return false;

    eec(scope, "all")->Fill(cosChi, energyWeight);
    const std::string chargeType = particle.charge == 0 ? "neutral" : "charged";
    eec(scope, chargeType)->Fill(cosChi, energyWeight);
    const std::string suffix = jpsiPtSuffix(candidate.jpsi.Pt());
    if (!suffix.empty()) {
      eec(scope, "all", suffix)->Fill(cosChi, energyWeight);
      eec(scope, chargeType, suffix)->Fill(cosChi, energyWeight);
    }
    return true;
  }

  void write(TFile &output) {
    TDirectory *directory = output.mkdir(label_.c_str());
    if (!directory)
      throw std::runtime_error("failed to create output directory " + label_);
    directory->cd();
    for (auto &hist : owned_)
      hist->Write();
    output.cd();
  }

private:
  TH1D *make(const std::string &name, const std::string &title, int bins,
             double low, double high) {
    auto hist =
        std::make_unique<TH1D>(name.c_str(), title.c_str(), bins, low, high);
    hist->SetDirectory(nullptr);
    hist->Sumw2();
    TH1D *result = hist.get();
    owned_.push_back(std::move(hist));
    return result;
  }

  void addEecSet(const std::string &scope) {
    for (const std::string type : {"all", "charged", "neutral"}) {
      addEecHistogram(scope, type, "");
      for (const auto &suffix : jpsiPtBinSuffixes())
        addEecHistogram(scope, type, suffix);
    }
  }

  void addEecHistogram(const std::string &scope, const std::string &type,
                       const std::string &suffix) {
    const std::string key = scope + ":" + type + suffix;
    eec_[key] = make("eec_" + scope + "_" + type + suffix,
                     "energy-weighted EEC;cos#chi;#Sigma E_{i}^{rest}/M", 20,
                     -1.0, 1.0);
  }

  TH1D *eec(const std::string &scope, const std::string &type,
            const std::string &suffix = "") {
    return eec_.at(scope + ":" + type + suffix);
  }
  std::string label_;
  std::vector<std::unique_ptr<TH1D>> owned_;
  std::map<std::string, TH1D *> eec_;
  std::map<std::string, TH1D *> jpsiPtBinned_;
  std::map<std::string, TH1D *> jpsiMassBinned_;
  std::map<std::string, TH1D *> jpsiJetPtBinned_;
  std::map<std::string, TH1D *> zPtBinned_;
  std::map<std::string, TH1D *> zHBinned_;
  std::map<std::string, TH1D *> fragmentationNumerator_;
  std::map<std::string, TH1D *> inclusiveJetDenominator_;
  TH1D *cutflow_ = nullptr;
  TH1D *eventWeight_ = nullptr;
  TH1D *jpsiPt_ = nullptr;
  TH1D *jpsiMass_ = nullptr;
  TH1D *jpsiJetPt_ = nullptr;
  TH1D *zPt_ = nullptr;
  TH1D *zH_ = nullptr;
  TH1D *nSelectedJets_ = nullptr;
  TH1D *constituentScale_ = nullptr;
};

inline bool analyzeEvent(const Candidate &candidate,
                         const std::vector<Jet> &jets, double weight,
                         const Settings &settings, Histograms &histograms) {
  if (!candidatePassesKinematics(candidate, settings))
    return false;
  histograms.fillCut(3, weight);
  if (jets.empty())
    return false;
  histograms.fillCut(4, weight);

  const int jpsiJetIndex = findJpsiJet(candidate, jets, settings);
  if (jpsiJetIndex < 0)
    return false;
  histograms.fillCut(5, weight);
  histograms.fillEvent(candidate, jets, weight);
  histograms.fillJpsiJet(candidate, jets.at(jpsiJetIndex), weight);

  bool filled = false;
  for (const auto &jet : jets)
    for (const auto &particle : jet.constituents)
      filled =
          histograms.fillParticle("alljets", particle, candidate, weight) ||
          filled;

  for (const auto &particle : jets.at(jpsiJetIndex).constituents)
    filled = histograms.fillParticle("jpsijet", particle, candidate, weight) ||
             filled;

  if (filled)
    histograms.fillCut(6, weight);
  return filled;
}

inline std::vector<std::string> rootFiles(const std::string &inputDirectory) {
  glob_t matches{};
  const bool isPrivateSample =
      inputDirectory.find("Private") != std::string::npos;
  const std::string pattern =
      inputDirectory + (isPrivateSample ? "/jetsinfo*.root" : "/*.root");
  const int status = glob(pattern.c_str(), GLOB_TILDE, nullptr, &matches);
  std::vector<std::string> files;
  if (status == 0) {
    files.reserve(matches.gl_pathc);
    for (std::size_t i = 0; i < matches.gl_pathc; ++i)
      files.emplace_back(matches.gl_pathv[i]);
  }
  globfree(&matches);
  std::sort(files.begin(), files.end());
  return files;
}

inline std::pair<int, int> chunkRange(int numberOfFiles, int numberOfChunks,
                                      int chunkIndex) {
  if (numberOfChunks <= 0 || chunkIndex < 0 || chunkIndex >= numberOfChunks)
    throw std::invalid_argument("invalid chunk configuration");
  const int filesPerChunk =
      (numberOfFiles + numberOfChunks - 1) / numberOfChunks;
  const int begin = std::min(chunkIndex * filesPerChunk, numberOfFiles);
  const int end = std::min(begin + filesPerChunk, numberOfFiles);
  return {begin, end};
}

inline std::string outputFileName(const std::string &outputBase,
                                  int chunkIndex) {
  const std::string suffix = ".root";
  if (outputBase.size() >= suffix.size() &&
      outputBase.compare(outputBase.size() - suffix.size(), suffix.size(),
                         suffix) == 0) {
    return outputBase.substr(0, outputBase.size() - suffix.size()) + "_Chunk" +
           std::to_string(chunkIndex) + suffix;
  }
  return outputBase + "_Chunk" + std::to_string(chunkIndex) + suffix;
}

} // namespace unified_eec

#endif
