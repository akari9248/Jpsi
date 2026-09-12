#include "EECCommon.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TLegend.h>
#include <TLine.h>
#include <TPad.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct Options {
  std::string cmsDirectory = "/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms_prompt";
  std::string privateDirectory =
      "/eos/user/s/shuangyu/public/Jpsi/eec_unified_private";
  std::string outputDirectory = "plots_eec_prompt";
  bool drawPositiveHalf = true;
};

struct Source {
  std::string file;
  std::string directory;
};

struct Sample {
  std::string label;
  std::vector<Source> sources;
};

void usage(const char *program) {
  std::cerr << "Usage: " << program
            << " [--cms-dir DIR] [--private-dir DIR] [--output-dir DIR]"
               " [--positive-half on|off]\n";
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
    const std::string argument = argv[i];
    auto value = [&]() -> std::string {
      if (i + 1 >= argc)
        throw std::invalid_argument("missing value after " + argument);
      return argv[++i];
    };
    if (argument == "--cms-dir")
      options.cmsDirectory = value();
    else if (argument == "--private-dir")
      options.privateDirectory = value();
    else if (argument == "--output-dir")
      options.outputDirectory = value();
    else if (argument == "--positive-half")
      options.drawPositiveHalf = parseOnOff(value());
    else if (argument == "-h" || argument == "--help") {
      usage(argv[0]);
      std::exit(0);
    } else {
      throw std::invalid_argument("unknown option " + argument);
    }
  }
  return options;
}

std::unique_ptr<TH1D> loadAndAdd(const Sample &sample,
                                 const std::string &histogramName,
                                 const std::string &cloneName) {
  static std::map<std::string, std::unique_ptr<TFile>> openFiles;
  std::unique_ptr<TH1D> result;
  for (const auto &source : sample.sources) {
    auto found = openFiles.find(source.file);
    if (found == openFiles.end()) {
      std::unique_ptr<TFile> opened(TFile::Open(source.file.c_str(), "READ"));
      if (!opened || opened->IsZombie())
        throw std::runtime_error("cannot open " + source.file);
      found = openFiles.emplace(source.file, std::move(opened)).first;
    }
    TFile &file = *found->second;
    const std::string path = source.directory + "/" + histogramName;
    TH1D *input = nullptr;
    file.GetObject(path.c_str(), input);
    if (!input)
      throw std::runtime_error("missing " + path + " in " + source.file);
    if (!result) {
      result.reset(static_cast<TH1D *>(input->Clone(cloneName.c_str())));
      result->SetDirectory(nullptr);
    } else if (!result->Add(input)) {
      throw std::runtime_error("incompatible histogram " + path);
    }
  }
  return result;
}

std::string ptDescription(const std::string &suffix) {
  if (suffix.empty())
    return "inclusive J/#psi p_{T}";
  const auto &suffixes = unified_eec::jpsiPtBinSuffixes();
  const auto &edges = unified_eec::jpsiPtBinEdges();
  const auto found = std::find(suffixes.begin(), suffixes.end(), suffix);
  if (found == suffixes.end())
    return suffix;
  const std::size_t index = found - suffixes.begin();
  if (index + 1 == suffixes.size())
    return "J/#psi p_{T} #geq 200 GeV";
  return "J/#psi p_{T} " + std::to_string(static_cast<int>(edges[index])) +
         "-" + std::to_string(static_cast<int>(edges[index + 1])) + " GeV";
}

void prepareShape(TH1D &histogram, bool positiveHalf) {
  if (positiveHalf) {
    for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
      if (histogram.GetXaxis()->GetBinCenter(bin) < 0.0) {
        histogram.SetBinContent(bin, 0.0);
        histogram.SetBinError(bin, 0.0);
      }
    }
  }
  const int firstBin = positiveHalf ? histogram.GetXaxis()->FindFixBin(0.0) : 1;
  const double integral = histogram.Integral(firstBin, histogram.GetNbinsX());
  if (!std::isfinite(integral) || integral <= 0.0)
    throw std::runtime_error("histogram has non-positive integral: " +
                             std::string(histogram.GetName()));
  histogram.Scale(1.0 / integral);
}

void drawComparison(std::vector<std::unique_ptr<TH1D>> input,
                    const std::vector<std::string> &labels,
                    const std::string &title, const std::string &output,
                    bool positiveHalf) {
  if (input.size() != labels.size() || input.empty())
    throw std::runtime_error("invalid plotting inputs");

  for (auto &histogram : input)
    prepareShape(*histogram, positiveHalf);

  const std::vector<int> colors = {kBlack,     kAzure + 1,   kRed + 1,
                                   kGreen + 2, kMagenta + 1, kOrange + 7};
  const std::vector<int> markers = {20, 21, 22, 23, 33, 34};
  double maximum = 0.0;
  double minimumPositive = std::numeric_limits<double>::max();
  for (std::size_t index = 0; index < input.size(); ++index) {
    auto &histogram = *input[index];
    histogram.SetLineColor(colors[index % colors.size()]);
    histogram.SetMarkerColor(colors[index % colors.size()]);
    histogram.SetMarkerStyle(markers[index % markers.size()]);
    histogram.SetMarkerSize(0.8);
    histogram.SetLineWidth(2);
    maximum = std::max(maximum, histogram.GetMaximum());
    for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
      const double content = histogram.GetBinContent(bin);
      if (content > 0.0)
        minimumPositive = std::min(minimumPositive, content);
    }
  }

  TCanvas canvas("eec_canvas", "eec_canvas", 850, 850);
  TPad upper("upper", "upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("lower", "lower", 0.0, 0.0, 1.0, 0.30);
  upper.SetBottomMargin(0.025);
  upper.SetLeftMargin(0.14);
  upper.SetRightMargin(0.04);
  upper.SetLogy();
  lower.SetTopMargin(0.035);
  lower.SetBottomMargin(0.32);
  lower.SetLeftMargin(0.14);
  lower.SetRightMargin(0.04);
  upper.Draw();
  lower.Draw();

  upper.cd();
  input.front()->SetTitle(title.c_str());
  input.front()->GetYaxis()->SetTitle("Normalized EEC");
  input.front()->GetYaxis()->SetTitleOffset(1.35);
  input.front()->GetXaxis()->SetLabelSize(0.0);
  input.front()->SetMinimum(std::max(1e-8, minimumPositive * 0.2));
  input.front()->SetMaximum(maximum * 30.0);
  if (positiveHalf)
    input.front()->GetXaxis()->SetRangeUser(0.0, 1.0);
  input.front()->Draw("E1");
  for (std::size_t index = 1; index < input.size(); ++index)
    input[index]->Draw("E1 SAME");

  TLegend legend(0.48, 0.67, 0.94, 0.90);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetTextSize(0.032);
  for (std::size_t index = 0; index < input.size(); ++index)
    legend.AddEntry(input[index].get(), labels[index].c_str(), "lep");
  legend.Draw();

  lower.cd();
  std::vector<std::unique_ptr<TH1D>> ratios;
  for (std::size_t index = 1; index < input.size(); ++index) {
    auto ratio = std::unique_ptr<TH1D>(static_cast<TH1D *>(
        input[index]->Clone(("ratio_" + std::to_string(index)).c_str())));
    ratio->SetDirectory(nullptr);
    ratio->Divide(input.front().get());
    ratio->SetTitle("");
    ratio->GetYaxis()->SetTitle("Ratio");
    ratio->GetXaxis()->SetTitle("cos#chi");
    ratio->GetYaxis()->SetRangeUser(0.0, 2.0);
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetTitleSize(0.10);
    ratio->GetYaxis()->SetTitleOffset(0.55);
    ratio->GetYaxis()->SetLabelSize(0.085);
    ratio->GetXaxis()->SetTitleSize(0.12);
    ratio->GetXaxis()->SetLabelSize(0.10);
    ratio->GetXaxis()->SetTitleOffset(1.05);
    if (positiveHalf)
      ratio->GetXaxis()->SetRangeUser(0.0, 1.0);
    ratio->Draw(index == 1 ? "E1" : "E1 SAME");
    ratios.push_back(std::move(ratio));
  }
  TLine unity(positiveHalf ? 0.0 : -1.0, 1.0, 1.0, 1.0);
  unity.SetLineStyle(2);
  unity.Draw();

  canvas.SaveAs(output.c_str());
  std::cout << "Wrote " << output << '\n';
}

std::vector<std::string> allPtSuffixes() {
  std::vector<std::string> result = {""};
  const auto &binned = unified_eec::jpsiPtBinSuffixes();
  result.insert(result.end(), binned.begin(), binned.end());
  return result;
}

std::vector<std::unique_ptr<TH1D>>
loadSamples(const std::vector<Sample> &samples,
            const std::string &histogramName, const std::string &namePrefix) {
  std::vector<std::unique_ptr<TH1D>> histograms;
  for (std::size_t index = 0; index < samples.size(); ++index)
    histograms.push_back(loadAndAdd(samples[index], histogramName,
                                    namePrefix + std::to_string(index)));
  return histograms;
}

std::vector<std::string> labels(const std::vector<Sample> &samples) {
  std::vector<std::string> result;
  for (const auto &sample : samples)
    result.push_back(sample.label);
  return result;
}

void drawFamily(const std::vector<Sample> &samples,
                const std::string &outputPrefix,
                const std::string &descriptionPrefix,
                const std::string &outputDirectory, bool positiveHalf) {
  for (const auto &suffix : allPtSuffixes()) {
    const std::string histogramName = "eec_alljets_all" + suffix;
    try {
      auto histograms =
          loadSamples(samples, histogramName, outputPrefix + suffix + "_");
      const std::string title =
          descriptionPrefix + ", " + ptDescription(suffix);
      drawComparison(std::move(histograms), labels(samples), title,
                     outputDirectory + "/" + outputPrefix + suffix + ".pdf",
                     false);
      if (positiveHalf) {
        auto positive = loadSamples(samples, histogramName,
                                    outputPrefix + suffix + "_positive_");
        drawComparison(
            std::move(positive), labels(samples), title + ", cos#chi > 0",
            outputDirectory + "/" + outputPrefix + suffix + "_positive.pdf",
            true);
      }
    } catch (const std::exception &error) {
      std::cerr << "SKIP " << outputPrefix << suffix << ": " << error.what()
                << '\n';
    }
  }
}

std::string path(const std::string &directory, const std::string &file) {
  return directory + "/" + file;
}

} // namespace

int main(int argc, char **argv) {
  try {
    const Options options = parseOptions(argc, argv);
    if (gSystem->mkdir(options.outputDirectory.c_str(), true) != 0 &&
        gSystem->AccessPathName(options.outputDirectory.c_str()))
      throw std::runtime_error("cannot create " + options.outputDirectory);
    gStyle->SetOptStat(0);
    gStyle->SetTitleBorderSize(0);

    const std::string cms = options.cmsDirectory;
    const std::vector<Sample> cmsSamples = {
        {"Data 2024G",
         {{path(cms, "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15.root"),
           "CmsReco"}}},
        {"Soft QCD",
         {{path(cms, "JPsiMuMu_Fil-JPsiNo-2MuPtEta_TuneCP5_13p6TeV_pythia8-"
                     "evtgen-RunIIISummer24.root"),
           "CmsReco"}}},
        {"Hard QCD, Onia off (nominal+ext1)",
         {{path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-"
                     "RunIIISummer24_Private.root"),
           "CmsReco"},
          {path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-"
                     "RunIIISummer24_ext1_Private.root"),
           "CmsReco"}}},
        {"Hard QCD, Onia on (nominal+ext1)",
         {{path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-"
                     "OniaOn-CR1_pythia8311-RunIIISummer24_Private.root"),
           "CmsReco"},
          {path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-"
                     "OniaOn-CR1_pythia8311-RunIIISummer24_ext1_Private.root"),
           "CmsReco"}}},
        {"Prompt charmonium",
         {{path(cms, "Jpsito2Mu_Bin-PTJpsi-8_TuneCP5_13p6TeV_pythia8-"
                     "RunIIISummer24.root"),
           "CmsReco"}}}};
    drawFamily(cmsSamples, "cms_reco_samples", "CMS reco all jets",
               options.outputDirectory, options.drawPositiveHalf);

    const std::string privateDir = options.privateDirectory;
    const std::string oniaOff8309 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8309.root");
    const std::string oniaOff8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8311.root");
    const std::string oniaOn8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaon_CR1_PYTHIA8311.root");
    const std::vector<Sample> privateSamples = {
        {"Onia off, Pythia 8.309", {{oniaOff8309, "PrivateGen"}}},
        {"Onia off, Pythia 8.311", {{oniaOff8311, "PrivateGen"}}},
        {"Onia on CR1, Pythia 8.311", {{oniaOn8311, "PrivateGen"}}}};
    drawFamily(privateSamples, "private_samples", "Private generation all jets",
               options.outputDirectory, options.drawPositiveHalf);

    const std::vector<Sample> categorySamples = {
        {"b-hadron", {{oniaOn8311, "PrivateGen_cat1"}}},
        {"charmonium feed-down", {{oniaOn8311, "PrivateGen_cat2"}}},
        {"gluon/proton", {{oniaOn8311, "PrivateGen_cat3"}}},
        {"quark", {{oniaOn8311, "PrivateGen_cat4"}}},
        {"Octet (cat5+cat6+cat7)",
         {{oniaOn8311, "PrivateGen_cat5"},
          {oniaOn8311, "PrivateGen_cat6"},
          {oniaOn8311, "PrivateGen_cat7"}}}};
    drawFamily(categorySamples, "private_oniaon_categories",
               "Private OniaOn CR1 categories, all jets",
               options.outputDirectory, options.drawPositiveHalf);

    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    usage(argv[0]);
    return 1;
  }
}
