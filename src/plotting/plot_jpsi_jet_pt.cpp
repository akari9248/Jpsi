#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TLine.h>
#include <TLatex.h>
#include <TPad.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Sample {
  std::string label;
  std::vector<std::string> files;
  std::vector<std::string> directories = {};
};
struct Observable {
  const char *name;
  const char *axis;
  int rebin;
  double fullMin, fullMax, zoomMin, zoomMax, logMin;
};
const std::vector<Observable> observables = {
    {"jpsi_pt", "p_{T}(J/#psi) [GeV]", 4, 0, 200, 0, 80, 1e-7},
    {"jpsijet_pt", "p_{T}(J/#psi jet) [GeV]", 10, 30, 600, 30, 160, 1e-8},
};

std::string path(const std::string &dir, const std::string &file) {
  return dir + "/" + file;
}
std::unique_ptr<TH1D> load(const Sample &sample, const std::string &directory,
                           const Observable &observable,
                           const std::string &histogramName, int index) {
  std::unique_ptr<TH1D> result;
  const std::vector<std::string> directories =
      sample.directories.empty() ? std::vector<std::string>{directory}
                                 : sample.directories;
  for (const auto &filename : sample.files) {
    std::unique_ptr<TFile> file(TFile::Open(filename.c_str(), "READ"));
    if (!file || file->IsZombie())
      throw std::runtime_error("Cannot open " + filename);
    for (const auto &sourceDirectory : directories) {
      auto addInput = [&](TH1D *input, const std::string &key) {
        if (!input)
          throw std::runtime_error("Missing " + key + " in " + filename);
        if (!result) {
          result.reset(static_cast<TH1D *>(input->Clone(
              (histogramName + "_" + std::to_string(index)).c_str())));
          result->SetDirectory(nullptr);
        } else if (!result->Add(input)) {
          throw std::runtime_error("Incompatible " + key + " in " + filename);
        }
      };
      const std::string key = sourceDirectory + "/" + histogramName;
      TH1D *input = nullptr;
      file->GetObject(key.c_str(), input);
      if (input) {
        addInput(input, key);
        continue;
      }
      // Older merged files split the 0-12 GeV bin into five sub-bins.
      const std::string suffix = "_jpsipt_0_12";
      if (histogramName.size() < suffix.size() ||
          histogramName.compare(histogramName.size() - suffix.size(),
                                suffix.size(), suffix) != 0)
        throw std::runtime_error("Missing " + key + " in " + filename);
      const std::string stem =
          histogramName.substr(0, histogramName.size() - suffix.size());
      for (const char *oldSuffix : {"_jpsipt_0_2", "_jpsipt_2_4",
                                    "_jpsipt_4_6", "_jpsipt_6_8",
                                    "_jpsipt_8_12"}) {
        const std::string oldKey = sourceDirectory + "/" + stem + oldSuffix;
        TH1D *oldInput = nullptr;
        file->GetObject(oldKey.c_str(), oldInput);
        addInput(oldInput, oldKey);
      }
    }
  }
  if (std::string(observable.name) == "jpsijet_pt") {
    // Preserve the 30 GeV jet threshold while using 20 GeV bins above it.
    std::vector<double> edges = {0.0, 30.0};
    for (double edge = 50.0; edge < 600.0; edge += 20.0)
      edges.push_back(edge);
    edges.push_back(600.0);
    auto *rebinned = result->Rebin(
        static_cast<int>(edges.size()) - 1,
        (std::string(result->GetName()) + "_rebinned").c_str(), edges.data());
    result.reset(static_cast<TH1D *>(rebinned));
    result->SetDirectory(nullptr);
  } else {
    if (result->GetNbinsX() % observable.rebin)
      throw std::runtime_error("Rebin factor does not divide histogram bins");
    result->Rebin(observable.rebin);
  }
  const double integral = result->Integral(1, result->GetNbinsX());
  if (!std::isfinite(integral) || integral <= 0)
    throw std::runtime_error("Nonpositive integral for " + sample.label);
  result->Scale(1.0 / integral, "width");
  return result;
}

void draw(const std::string &group, const std::string &directory,
          const std::vector<Sample> &samples, const Observable &observable,
          const std::string &outputDir, bool zoom,
          const std::string &histogramSuffix = "",
          const std::string &outputStem = "",
          const std::string &description = "",
          bool omitEmpty = false) {
  const std::vector<int> colors = {
      TColor::GetColor("#292929"), TColor::GetColor("#1683FF"),
      TColor::GetColor("#00B86B"), TColor::GetColor("#FF8A00"),
      TColor::GetColor("#C83EFF"), TColor::GetColor("#F43F5E")};
  const std::vector<int> markers = {20, 21, 22, 23, 33, 34};
  std::vector<std::unique_ptr<TH1D>> histograms;
  std::vector<std::string> labels;
  double maximum = 0.0;
  for (std::size_t i = 0; i < samples.size(); ++i) {
    std::unique_ptr<TH1D> histogram;
    try {
      histogram = load(samples[i], directory, observable,
                       std::string(observable.name) + histogramSuffix, i);
    } catch (const std::runtime_error &error) {
      if (!omitEmpty)
        throw;
      std::cerr << "OMIT " << samples[i].label << ": " << error.what() << '\n';
      continue;
    }
    histogram->SetTitle("");
    const std::size_t colorIndex = histograms.size();
    histogram->SetLineColor(colors.at(colorIndex));
    histogram->SetMarkerColor(colors.at(colorIndex));
    histogram->SetMarkerStyle(markers.at(colorIndex));
    histogram->SetMarkerSize(1.15);
    histogram->SetLineWidth(3);
    maximum = std::max(maximum, histogram->GetMaximum());
    histograms.push_back(std::move(histogram));
    labels.push_back(samples[i].label);
  }
  if (histograms.size() < 2)
    throw std::runtime_error("Fewer than two nonempty curves for " +
                             std::string(observable.name) + histogramSuffix);

  double xMin = zoom ? observable.zoomMin : observable.fullMin;
  double xMax = zoom ? observable.zoomMax : observable.fullMax;
  if (zoom && std::string(observable.name) == "jpsijet_pt") {
    if (histogramSuffix == "_jpsipt_100_200") {
      xMin = 100.0;
      xMax = 300.0;
    } else if (histogramSuffix == "_jpsipt_200_Inf") {
      xMin = 200.0;
      xMax = 500.0;
    }
  }
  TCanvas canvas("eec_canvas", "eec_canvas", 1000, 1000);
  TPad upper("upper", "upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("lower", "lower", 0.0, 0.0, 1.0, 0.30);
  upper.SetBottomMargin(0.025);
  upper.SetLeftMargin(0.14);
  upper.SetRightMargin(0.04);
  upper.SetTopMargin(0.13);
  upper.SetLogy();
  lower.SetTopMargin(0.035);
  lower.SetBottomMargin(0.32);
  lower.SetLeftMargin(0.14);
  lower.SetRightMargin(0.04);
  upper.Draw();
  lower.Draw();

  upper.cd();
  auto &first = *histograms.front();
  first.GetYaxis()->SetTitle("(1/N) dN/dp_{T} [GeV^{-1}]");
  first.GetYaxis()->SetTitleSize(0.055);
  first.GetYaxis()->SetTitleOffset(1.15);
  first.GetYaxis()->SetLabelSize(0.048);
  first.GetXaxis()->SetLabelSize(0.0);
  first.GetXaxis()->SetRangeUser(xMin, xMax);
  first.SetMinimum(zoom ? 1e-4 : observable.logMin);
  first.SetMaximum(maximum * 6.0);
  first.Draw("E1");
  for (std::size_t i = 1; i < histograms.size(); ++i)
    histograms[i]->Draw("E1 SAME");
  TLatex title;
  title.SetNDC();
  title.SetTextFont(42);
  title.SetTextAlign(23);
  const std::string zoomDescription = !zoom ? "" :
      (histogramSuffix == "_jpsipt_100_200" ||
       histogramSuffix == "_jpsipt_200_Inf") ? ", high jet p_{T} detail" :
      ", low p_{T} detail";
  const std::string titleText = description.empty()
      ? std::string(group == "cms" ? "CMS reco" : "Private generation") +
            ", selected J/#psi events" + zoomDescription
      : description + zoomDescription;
  title.SetTextSize(titleText.size() > 55 ? 0.034 : 0.042);
  title.DrawLatex(0.50, 0.965, titleText.c_str());
  TLegend legend(0.34, histograms.size() >= 5 ? 0.65 : 0.70, 0.96, 0.86);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetNColumns(2);
  legend.SetTextSize(0.038);
  legend.SetMargin(0.15);
  for (std::size_t i = 0; i < histograms.size(); ++i)
    legend.AddEntry(histograms[i].get(), labels[i].c_str(), "lep");
  legend.Draw();

  lower.cd();
  std::vector<std::unique_ptr<TH1D>> ratios;
  for (std::size_t i = 1; i < histograms.size(); ++i) {
    auto ratio = std::unique_ptr<TH1D>(static_cast<TH1D *>(
        histograms[i]->Clone(("ratio_" + std::to_string(i)).c_str())));
    ratio->SetDirectory(nullptr);
    ratio->Divide(histograms.front().get());
    ratio->SetTitle("");
    ratio->GetYaxis()->SetTitle(group == "cms" ? "MC / data" : "Ratio");
    ratio->GetXaxis()->SetTitle(observable.axis);
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetTitleSize(0.115);
    ratio->GetYaxis()->SetTitleOffset(0.55);
    ratio->GetYaxis()->SetLabelSize(0.10);
    ratio->GetXaxis()->SetTitleSize(0.135);
    ratio->GetXaxis()->SetLabelSize(0.115);
    ratio->GetXaxis()->SetTitleOffset(0.95);
    ratio->GetXaxis()->CenterTitle();
    ratio->GetXaxis()->SetRangeUser(xMin, xMax);
    const double ratioLow = group == "cms" ? 0.25 :
                            group == "private" ? 0.85 : 0.0;
    const double ratioHigh = group == "cms" ? 2.25 :
                             group == "private" ? 1.15 : 3.0;
    ratio->GetYaxis()->SetRangeUser(ratioLow, ratioHigh);
    ratio->Draw(i == 1 ? "E1" : "E1 SAME");
    ratios.push_back(std::move(ratio));
  }
  TLine unity(xMin, 1.0, xMax, 1.0);
  unity.SetLineStyle(2);
  unity.Draw();

  const std::string stem = outputStem.empty()
      ? group + "_" + observable.name + histogramSuffix
      : outputStem;
  const std::string output = outputDir + "/" + stem +
                             (zoom ? "_zoom" : "") + ".pdf";
  std::filesystem::create_directories(std::filesystem::path(output).parent_path());
  canvas.SaveAs(output.c_str());
}

std::vector<Sample> cmsSamples(const std::string &dir) {
  return {
    {"Data 2024G", {path(dir, "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15.root")}},
    {"Soft QCD", {path(dir, "JPsiMuMu_Fil-JPsiNo-2MuPtEta_TuneCP5_13p6TeV_pythia8-evtgen-RunIIISummer24.root")}},
    {"Hard QCD off CR0", {
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-RunIIISummer24_Private.root"),
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-RunIIISummer24_ext1_Private.root")}},
    {"Hard QCD on CR0", {
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR0_pythia8311-RunIIISummer24_Private.root"),
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR0_pythia8311-RunIIISummer24_ext1_Private.root")}},
    {"Hard QCD on CR1", {
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR1_pythia8311-RunIIISummer24_Private.root"),
      path(dir, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-OniaOn-CR1_pythia8311-RunIIISummer24_ext1_Private.root")}},
    {"Prompt charmonium", {path(dir, "Jpsito2Mu_Bin-PTJpsi-8_TuneCP5_13p6TeV_pythia8-RunIIISummer24.root")}},
  };
}
std::vector<Sample> privateSamples(const std::string &dir) {
  return {
    {"Onia off CR0 (8.311)", {path(dir, "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8311.root")}},
    {"Onia off CR1 (8.311)", {path(dir, "HardQCD_Pt15to7000_Oniaoff_CR1_PYTHIA8311.root")}},
    {"Onia on CR0 (8.311)", {path(dir, "HardQCD_Pt15to7000_Oniaon_CR0_PYTHIA8311.root")}},
    {"Onia on CR1 (8.311)", {path(dir, "HardQCD_Pt15to7000_Oniaon_CR1_PYTHIA8311.root")}},
  };
}
struct Category {
  const char *key;
  const char *label;
  std::vector<std::string> directories;
};
const std::vector<Category> categories = {
    {"b_hadron", "b-hadron", {"PrivateGen_cat1"}},
    {"charmonium_from_b", "b-hadron charmonium",
     {"PrivateGen_cat2_from_b_hadron"}},
    {"non_b_charmonium", "non-B charmonium",
     {"PrivateGen_cat2_psi_2S", "PrivateGen_cat2_chi_c1",
      "PrivateGen_cat2_chi_c2", "PrivateGen_cat2_others"}},
    {"quark", "quark", {"PrivateGen_cat4"}},
    {"octet", "octet",
     {"PrivateGen_cat5", "PrivateGen_cat6", "PrivateGen_cat7"}},
};
struct PtBin {
  const char *suffix;
  const char *description;
};
const std::vector<PtBin> jpsiPtBins = {
    {"_jpsipt_0_12", "J/#psi p_{T} 0-12 GeV"},
    {"_jpsipt_12_16", "J/#psi p_{T} 12-16 GeV"},
    {"_jpsipt_16_20", "J/#psi p_{T} 16-20 GeV"},
    {"_jpsipt_20_30", "J/#psi p_{T} 20-30 GeV"},
    {"_jpsipt_30_50", "J/#psi p_{T} 30-50 GeV"},
    {"_jpsipt_50_100", "J/#psi p_{T} 50-100 GeV"},
    {"_jpsipt_100_200", "J/#psi p_{T} 100-200 GeV"},
    {"_jpsipt_200_Inf", "J/#psi p_{T} #geq 200 GeV"},
};
const std::vector<std::string> privateKeys = {
    "oniaoff_cr0", "oniaoff_cr1", "oniaon_cr0", "oniaon_cr1"};

bool categoryAvailable(std::size_t sampleIndex, const std::string &key) {
  if (sampleIndex >= 2)
    return true;
  if (key == "b_hadron" || key == "charmonium_from_b")
    return true;
  return sampleIndex == 1 && key == "quark";
}

std::vector<Sample> categoriesForSample(const Sample &sample,
                                        std::size_t sampleIndex) {
  std::vector<Sample> result;
  for (const auto &category : categories)
    if (categoryAvailable(sampleIndex, category.key))
      result.push_back({category.label, sample.files, category.directories});
  return result;
}

std::vector<Sample> samplesForCategory(const std::vector<Sample> &samples,
                                       const Category &category) {
  std::vector<Sample> result;
  for (std::size_t i = 0; i < samples.size(); ++i)
    if (categoryAvailable(i, category.key))
      result.push_back(
          {samples[i].label, samples[i].files, category.directories});
  return result;
}

void tryDraw(const std::string &group, const std::string &directory,
             const std::vector<Sample> &samples, const Observable &observable,
             const std::string &outputDir, bool zoom,
             const std::string &histogramSuffix,
             const std::string &outputStem,
             const std::string &description) {
  try {
    draw(group, directory, samples, observable, outputDir, zoom,
         histogramSuffix, outputStem, description, true);
  } catch (const std::exception &error) {
    std::cerr << "SKIP " << outputStem << (zoom ? "_zoom" : "")
              << ": " << error.what() << '\n';
  }
}

void drawFamilies(const std::string &cmsDir, const std::string &privateDir,
                  const std::string &outputDir) {
  const auto cms = cmsSamples(cmsDir);
  const auto privateSet = privateSamples(privateDir);
  const auto &jet = observables.at(1);

  // Inclusive sample comparisons retain their original output paths.
  for (const auto &observable : observables)
    for (bool zoom : {false, true}) {
      draw("cms", "CmsReco", cms, observable, outputDir, zoom);
      draw("private", "PrivateGen", privateSet, observable, outputDir, zoom);
    }

  // Jet pT at fixed J/psi pT, compared across CMS or private samples.
  for (const auto &bin : jpsiPtBins)
    for (bool zoom : {false, true}) {
      const std::string suffix = bin.suffix;
      tryDraw("cms", "CmsReco", cms, jet, outputDir, zoom, suffix,
              "sample_by_jpsipt/cms_jpsijet_pt" + suffix,
              "CMS reco samples, " + std::string(bin.description));
      tryDraw("private", "PrivateGen", privateSet, jet, outputDir, zoom,
              suffix, "sample_by_jpsipt/private_jpsijet_pt" + suffix,
              "Private samples, " + std::string(bin.description));
    }

  // Within each private sample, compare the same source categories as plot_eec.
  for (std::size_t i = 0; i < privateSet.size(); ++i) {
    const auto sources = categoriesForSample(privateSet[i], i);
    const std::string prefix = "private_categories/private_" + privateKeys[i];
    for (const auto &observable : observables)
      for (bool zoom : {false, true})
        tryDraw("categories", "PrivateGen", sources, observable, outputDir,
                zoom, "", prefix + "_" + observable.name + "_categories",
                privateSet[i].label + " source categories");
    for (const auto &bin : jpsiPtBins)
      for (bool zoom : {false, true})
        tryDraw("categories", "PrivateGen", sources, jet, outputDir, zoom,
                bin.suffix,
                "private_categories_by_jpsipt/private_" + privateKeys[i] +
                    "_jpsijet_pt_categories" + bin.suffix,
                privateSet[i].label + " categories, " + bin.description);
  }

  // Fix the source and compare available PYTHIA configurations.
  for (const auto &category : categories) {
    const auto selected = samplesForCategory(privateSet, category);
    const std::string prefix =
        "category_across_samples/private_" + std::string(category.key);
    for (const auto &observable : observables)
      for (bool zoom : {false, true})
        tryDraw("category_samples", "PrivateGen", selected, observable,
                outputDir, zoom, "",
                prefix + "_" + observable.name + "_samples",
                std::string(category.label) + " across private samples");
    for (const auto &bin : jpsiPtBins)
      for (bool zoom : {false, true})
        tryDraw("category_samples", "PrivateGen", selected, jet,
                outputDir, zoom, bin.suffix,
                "category_across_samples_by_jpsipt/private_" +
                    std::string(category.key) + "_jpsijet_pt_samples" + bin.suffix,
                std::string(category.label) + ", " + bin.description);
  }

  // Resolve non-B charmonium feed-down modes for the OniaShower samples.
  const std::vector<Sample> feeddown = {
      {"#psi(2S)", privateSet[3].files, {"PrivateGen_cat2_psi_2S"}},
      {"#chi_{c1}", privateSet[3].files, {"PrivateGen_cat2_chi_c1"}},
      {"#chi_{c2}", privateSet[3].files, {"PrivateGen_cat2_chi_c2"}},
  };
  for (const auto &observable : observables)
    for (bool zoom : {false, true})
      tryDraw("categories", "PrivateGen", feeddown, observable, outputDir,
              zoom, "", "private_categories/private_oniaon_cr1_" +
                            std::string(observable.name) + "_non_b_charmonium",
              "Onia on CR1 non-B charmonium feed-down");
}

} // namespace

int main(int argc, char **argv) {
  try {
    std::string cmsDir = "/eos/user/s/shuangyu/public/Jpsi/eec_cms";
    std::string privateDir = "/eos/user/s/shuangyu/public/Jpsi/eec_private";
    std::string outputDir = "plots_jpsi_jet_pt";
    for (int i = 1; i < argc; ++i) {
      if (i + 1 >= argc)
        throw std::invalid_argument("Missing value after " + std::string(argv[i]));
      const std::string option = argv[i];
      const std::string value = argv[++i];
      if (option == "--cms-dir") cmsDir = value;
      else if (option == "--private-dir") privateDir = value;
      else if (option == "--output-dir") outputDir = value;
      else throw std::invalid_argument("Unknown option " + option);
    }
    gStyle->SetOptStat(0);
    gStyle->SetTitleBorderSize(0);
    gStyle->SetLegendFont(42);
    gStyle->SetTitleFontSize(0.045);
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gSystem->mkdir(outputDir.c_str(), true);
    drawFamilies(cmsDir, privateDir, outputDir);
  } catch (const std::exception &error) {
    std::cerr << "plot_jpsi_jet_pt: " << error.what() << '\n';
    return 1;
  }
}
