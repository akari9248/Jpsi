#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TLegend.h>
#include <TPad.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// Example:
// root -l -q 'compare.C("private.root","PrivateGen","cms.root","CmsGen")'
void compare(const char *firstFile, const char *firstDirectory,
             const char *secondFile, const char *secondDirectory,
             const char *histogramName = "eec_alljets_all",
             const char *outputName = "eec_comparison.pdf",
             bool normalize = true) {
  std::unique_ptr<TFile> first(TFile::Open(firstFile));
  std::unique_ptr<TFile> second(TFile::Open(secondFile));
  if (!first || first->IsZombie() || !second || second->IsZombie())
    throw std::runtime_error("failed to open an input ROOT file");

  const std::string firstPath =
      std::string(firstDirectory) + "/" + histogramName;
  const std::string secondPath =
      std::string(secondDirectory) + "/" + histogramName;
  TH1 *firstInput = nullptr;
  TH1 *secondInput = nullptr;
  first->GetObject(firstPath.c_str(), firstInput);
  second->GetObject(secondPath.c_str(), secondInput);
  if (!firstInput || !secondInput)
    throw std::runtime_error("missing comparable histogram");

  std::unique_ptr<TH1> firstHist(
      static_cast<TH1 *>(firstInput->Clone("first_comparison")));
  std::unique_ptr<TH1> secondHist(
      static_cast<TH1 *>(secondInput->Clone("second_comparison")));
  firstHist->SetDirectory(nullptr);
  secondHist->SetDirectory(nullptr);
  if (normalize) {
    if (firstHist->Integral() != 0.0)
      firstHist->Scale(1.0 / firstHist->Integral());
    if (secondHist->Integral() != 0.0)
      secondHist->Scale(1.0 / secondHist->Integral());
  }

  firstHist->SetLineColor(kBlue + 1);
  firstHist->SetMarkerColor(kBlue + 1);
  secondHist->SetLineColor(kRed + 1);
  secondHist->SetMarkerColor(kRed + 1);

  TCanvas canvas("comparison", "comparison", 800, 800);
  TPad upper("upper", "upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("lower", "lower", 0.0, 0.0, 1.0, 0.30);
  upper.SetBottomMargin(0.02);
  lower.SetTopMargin(0.03);
  lower.SetBottomMargin(0.30);
  upper.Draw();
  lower.Draw();

  upper.cd();
  firstHist->SetTitle(histogramName);
  firstHist->Draw("E1");
  secondHist->Draw("E1 SAME");
  TLegend legend(0.60, 0.75, 0.88, 0.88);
  legend.AddEntry(firstHist.get(), firstDirectory, "lep");
  legend.AddEntry(secondHist.get(), secondDirectory, "lep");
  legend.Draw();

  lower.cd();
  std::unique_ptr<TH1> ratio(
      static_cast<TH1 *>(secondHist->Clone("comparison_ratio")));
  ratio->Divide(firstHist.get());
  ratio->SetTitle("");
  ratio->GetYaxis()->SetTitle("second/first");
  ratio->GetYaxis()->SetNdivisions(505);
  ratio->GetYaxis()->SetTitleSize(0.10);
  ratio->GetYaxis()->SetLabelSize(0.09);
  ratio->GetXaxis()->SetTitle("cos#chi");
  ratio->GetXaxis()->SetTitleSize(0.12);
  ratio->GetXaxis()->SetLabelSize(0.10);
  ratio->Draw("E1");

  canvas.SaveAs(outputName);
  std::cout << "Wrote " << outputName << '\n';
}
