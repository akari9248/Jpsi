#include "EECCommon.h"

#include <TClass.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TKey.h>
#include <TNamed.h>

#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

int makeInDirectory(TDirectory &directory) {
  int made = 0;
  for (const auto &suffix :
       unified_eec::fragmentationJetPtBinSuffixes()) {
    const std::string numeratorName = "fragmentation_numerator" + suffix;
    const std::string denominatorName =
        "inclusive_jet_denominator" + suffix;
    auto *numerator =
        dynamic_cast<TH1D *>(directory.Get(numeratorName.c_str()));
    auto *denominator =
        dynamic_cast<TH1D *>(directory.Get(denominatorName.c_str()));
    if (!numerator || !denominator)
      continue;

    const double denominatorValue = denominator->GetBinContent(1);
    const double denominatorError = denominator->GetBinError(1);
    if (!std::isfinite(denominatorValue) || denominatorValue == 0.0) {
      std::cerr << "WARNING: skipping " << directory.GetName() << "/"
                << suffix.substr(1) << ": zero or invalid inclusive-jet "
                << "denominator\n";
      continue;
    }

    const std::string outputName = "fragmentation_function" + suffix;
    std::unique_ptr<TH1D> fragmentation(
        static_cast<TH1D *>(numerator->Clone(outputName.c_str())));
    fragmentation->Reset("ICES");
    fragmentation->SetDirectory(nullptr);
    fragmentation->SetTitle(
        "J/#psi jet fragmentation function;z_{h};"
        "(1/N_{jet}) dN_{J/#psi jet}/dz_{h}");

    for (int bin = 1; bin <= numerator->GetNbinsX(); ++bin) {
      const double width = numerator->GetXaxis()->GetBinWidth(bin);
      const double numeratorValue = numerator->GetBinContent(bin);
      const double numeratorError = numerator->GetBinError(bin);
      const double value = numeratorValue / (denominatorValue * width);
      const double errorSquared =
          std::pow(numeratorError / (denominatorValue * width), 2) +
          std::pow(numeratorValue * denominatorError /
                       (denominatorValue * denominatorValue * width),
                   2);
      fragmentation->SetBinContent(bin, value);
      fragmentation->SetBinError(bin, std::sqrt(errorSquared));
    }

    directory.cd();
    fragmentation->Write(outputName.c_str(), TObject::kOverwrite);
    ++made;
  }
  if (made > 0) {
    TNamed definition(
        "fragmentation_function_definition",
        "F(z_h,pT)=(1/N_inclusive-jet) dN_J/psi-jet/dz_h; jet-pT bins "
        "50-100,100-150,150-200 GeV; integrated over configured jet eta");
    directory.cd();
    definition.Write(definition.GetName(), TObject::kOverwrite);
  }
  return made;
}

double safeRatio(double numerator, double denominator) {
  return denominator > 0.0 ? numerator / denominator : 0.0;
}

bool makePrivateFeeddownTagMetrics(TFile &file) {
  auto *directory = file.GetDirectory("PrivateFeeddownTagDiagnostics");
  if (!directory)
    return false;
  auto *matrix = dynamic_cast<TH2D *>(
      directory->Get("private_feeddown_tag_migration"));
  if (!matrix)
    return false;

  // x: untagged, chi_c tag, psi(2S) tag
  // y: b, chi_c1/2, psi(2S), other charmonium, direct prompt
  const double chiCorrect = matrix->GetBinContent(2, 2);
  const double psiCorrect = matrix->GetBinContent(3, 3);
  const double directCorrect = matrix->GetBinContent(1, 5);
  const double chiTruth = matrix->Integral(1, 3, 2, 2);
  const double psiTruth = matrix->Integral(1, 3, 3, 3);
  const double directTruth = matrix->Integral(1, 3, 5, 5);
  const double chiTagged = matrix->Integral(2, 2, 1, 5);
  const double psiTagged = matrix->Integral(3, 3, 1, 5);
  const double targetTotal = chiTruth + psiTruth + directTruth;

  TH1D metrics("private_feeddown_tag_metrics",
               ";Metric;Value", 7, 0.5, 7.5);
  metrics.SetDirectory(nullptr);
  const std::array<const char *, 7> labels = {
      "chi efficiency", "chi purity", "psi(2S) efficiency",
      "psi(2S) purity", "direct untagged efficiency",
      "target-class accuracy", "prompt selected yield"};
  const std::array<double, 7> values = {
      safeRatio(chiCorrect, chiTruth), safeRatio(chiCorrect, chiTagged),
      safeRatio(psiCorrect, psiTruth), safeRatio(psiCorrect, psiTagged),
      safeRatio(directCorrect, directTruth),
      safeRatio(chiCorrect + psiCorrect + directCorrect, targetTotal),
      matrix->Integral(1, 3, 2, 5)};
  for (std::size_t bin = 0; bin < values.size(); ++bin) {
    metrics.GetXaxis()->SetBinLabel(static_cast<int>(bin) + 1, labels[bin]);
    metrics.SetBinContent(static_cast<int>(bin) + 1, values[bin]);
  }
  directory->cd();
  metrics.Write(metrics.GetName(), TObject::kOverwrite);
  std::cout << "Private feed-down tag metrics: chi efficiency=" << values[0]
            << ", chi purity=" << values[1]
            << ", psi(2S) efficiency=" << values[2]
            << ", psi(2S) purity=" << values[3]
            << ", direct untagged efficiency=" << values[4]
            << ", target-class accuracy=" << values[5] << '\n';
  return true;
}

} // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " FILE.root\n";
    return 1;
  }

  try {
    std::unique_ptr<TFile> file(TFile::Open(argv[1], "UPDATE"));
    if (!file || file->IsZombie())
      throw std::runtime_error("cannot open ROOT file for update: " +
                               std::string(argv[1]));

    int made = 0;
    TIter next(file->GetListOfKeys());
    while (auto *key = dynamic_cast<TKey *>(next())) {
      TClass *keyClass = TClass::GetClass(key->GetClassName());
      if (!keyClass || !keyClass->InheritsFrom(TDirectory::Class()))
        continue;
      TDirectory *directory = file->GetDirectory(key->GetName());
      if (directory)
        made += makeInDirectory(*directory);
    }
    makePrivateFeeddownTagMetrics(*file);
    file->Close();
    std::cout << "Created or updated " << made
              << " fragmentation-function histogram(s) in " << argv[1]
              << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    return 1;
  }
}
