#include "EECCommon.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TLine.h>
#include <TPad.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
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

constexpr const char *dataFileName =
    "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15.root";
constexpr int cosChiRebin = 2;

struct Options {
  std::string signalFile =
      "/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms/" +
      std::string(dataFileName);
  std::string lowerFile =
      "/eos/user/s/shuangyu/public/Jpsi/eec_data_sideband_2p7_2p9/" +
      std::string(dataFileName);
  std::string upperFile =
      "/eos/user/s/shuangyu/public/Jpsi/eec_data_sideband_3p3_3p5/" +
      std::string(dataFileName);
  std::string mcDirectory =
      "/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms";
  std::string outputDirectory = "plots_sideband_subtraction";
  std::string rootOutput = "sideband_subtraction.root";
  std::string rootDirectory = "CmsReco";
  double signalWidth = 0.4;
  double lowerWidth = 0.2;
  double upperWidth = 0.2;
  bool drawPositiveHalf = true;
};

struct Sample {
  std::string label;
  std::vector<std::string> files;
};

void usage(const char *program) {
  std::cerr
      << "Usage: " << program << " [options]\n"
      << "  --signal-file FILE       Data in the 2.9-3.3 GeV window\n"
      << "  --lower-file FILE        Data in the 2.7-2.9 GeV sideband\n"
      << "  --upper-file FILE        Data in the 3.3-3.5 GeV sideband\n"
      << "  --mc-dir DIR             Directory containing merged CMS MC files\n"
      << "  --output-dir DIR         Plot destination\n"
      << "  --root-output NAME       ROOT output filename inside output-dir\n"
      << "  --directory NAME         ROOT directory (default: CmsReco)\n"
      << "  --signal-width X         Signal-window width (default: 0.4)\n"
      << "  --lower-width X          Lower-sideband width (default: 0.2)\n"
      << "  --upper-width X          Upper-sideband width (default: 0.2)\n"
      << "  --positive-half on|off   Also draw cos(chi)>0 plots (default: on)\n";
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
    if (argument == "--signal-file")
      options.signalFile = value();
    else if (argument == "--lower-file")
      options.lowerFile = value();
    else if (argument == "--upper-file")
      options.upperFile = value();
    else if (argument == "--mc-dir")
      options.mcDirectory = value();
    else if (argument == "--output-dir")
      options.outputDirectory = value();
    else if (argument == "--root-output")
      options.rootOutput = value();
    else if (argument == "--directory")
      options.rootDirectory = value();
    else if (argument == "--signal-width")
      options.signalWidth = std::stod(value());
    else if (argument == "--lower-width")
      options.lowerWidth = std::stod(value());
    else if (argument == "--upper-width")
      options.upperWidth = std::stod(value());
    else if (argument == "--positive-half")
      options.drawPositiveHalf = parseOnOff(value());
    else if (argument == "-h" || argument == "--help") {
      usage(argv[0]);
      std::exit(0);
    } else {
      throw std::invalid_argument("unknown option " + argument);
    }
  }
  if (!std::isfinite(options.signalWidth) ||
      !std::isfinite(options.lowerWidth) ||
      !std::isfinite(options.upperWidth) || options.signalWidth <= 0.0 ||
      options.lowerWidth <= 0.0 || options.upperWidth <= 0.0)
    throw std::invalid_argument("all mass-window widths must be positive");
  return options;
}

std::string path(const std::string &directory, const std::string &file) {
  return directory + "/" + file;
}

std::unique_ptr<TH1D> loadHistogram(const std::string &fileName,
                                    const std::string &directory,
                                    const std::string &histogramName,
                                    const std::string &cloneName) {
  static std::map<std::string, std::unique_ptr<TFile>> openFiles;
  auto found = openFiles.find(fileName);
  if (found == openFiles.end()) {
    std::unique_ptr<TFile> file(TFile::Open(fileName.c_str(), "READ"));
    if (!file || file->IsZombie())
      throw std::runtime_error("cannot open " + fileName);
    found = openFiles.emplace(fileName, std::move(file)).first;
  }
  TH1D *input = nullptr;
  found->second->GetObject((directory + "/" + histogramName).c_str(), input);
  if (!input)
    throw std::runtime_error("missing " + directory + "/" + histogramName +
                             " in " + fileName);
  auto result = std::unique_ptr<TH1D>(
      static_cast<TH1D *>(input->Clone(cloneName.c_str())));
  result->SetDirectory(nullptr);
  return result;
}

std::unique_ptr<TH1D> loadSample(const Sample &sample,
                                 const std::string &directory,
                                 const std::string &histogramName,
                                 const std::string &cloneName) {
  std::unique_ptr<TH1D> result;
  for (std::size_t i = 0; i < sample.files.size(); ++i) {
    auto input = loadHistogram(sample.files[i], directory, histogramName,
                               cloneName + "_part" + std::to_string(i));
    if (!result) {
      input->SetName(cloneName.c_str());
      result = std::move(input);
    } else if (!result->Add(input.get())) {
      throw std::runtime_error("incompatible MC histogram in " +
                               sample.files[i]);
    }
  }
  if (!result)
    throw std::runtime_error("empty MC sample " + sample.label);
  return result;
}

std::unique_ptr<TH1D> cloneHistogram(const TH1D &input,
                                     const std::string &name) {
  auto result = std::unique_ptr<TH1D>(
      static_cast<TH1D *>(input.Clone(name.c_str())));
  result->SetDirectory(nullptr);
  return result;
}

void keepPositiveHalf(TH1D &histogram) {
  for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
    if (histogram.GetXaxis()->GetBinCenter(bin) < 0.0) {
      histogram.SetBinContent(bin, 0.0);
      histogram.SetBinError(bin, 0.0);
    }
  }
}

void normalize(TH1D &histogram, bool positiveHalf) {
  if (positiveHalf)
    keepPositiveHalf(histogram);
  const int first = positiveHalf ? histogram.GetXaxis()->FindFixBin(0.0) : 1;
  const double integral = histogram.Integral(first, histogram.GetNbinsX());
  if (!std::isfinite(integral) || integral <= 0.0)
    throw std::runtime_error("non-positive integral for " +
                             std::string(histogram.GetName()));
  histogram.Scale(1.0 / integral);
}

void drawOverlay(std::vector<std::unique_ptr<TH1D>> histograms,
                 const std::vector<std::string> &labels,
                 const std::string &title, const std::string &output,
                 const std::string &yTitle, bool normalizeShapes,
                 bool positiveHalf, bool logarithmic,
                 double ratioUpperLimit = 5.0) {
  if (histograms.empty() || histograms.size() != labels.size())
    throw std::runtime_error("invalid plotting inputs");
  for (auto &histogram : histograms) {
    if (histogram->GetNbinsX() % cosChiRebin != 0)
      throw std::runtime_error("coschi rebin factor does not divide bins");
    histogram->Rebin(cosChiRebin);
  }
  if (normalizeShapes)
    for (auto &histogram : histograms)
      normalize(*histogram, positiveHalf);
  else if (positiveHalf)
    for (auto &histogram : histograms)
      keepPositiveHalf(*histogram);

  const std::vector<int> colors = {kBlack, kBlue + 1, kRed + 1,
                                   kGreen + 2, kMagenta + 1, kOrange + 7};
  const std::vector<int> markers = {20, 21, 22, 23, 33, 34};
  double maximum = -std::numeric_limits<double>::infinity();
  double minimum = std::numeric_limits<double>::infinity();
  double minimumPositive = std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < histograms.size(); ++i) {
    TH1D &histogram = *histograms[i];
    histogram.SetLineColor(colors[i % colors.size()]);
    histogram.SetMarkerColor(colors[i % colors.size()]);
    histogram.SetMarkerStyle(markers[i % markers.size()]);
    histogram.SetMarkerSize(0.75);
    histogram.SetLineWidth(2);
    for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
      if (positiveHalf && histogram.GetXaxis()->GetBinCenter(bin) < 0.0)
        continue;
      const double content = histogram.GetBinContent(bin);
      maximum = std::max(maximum, content);
      minimum = std::min(minimum, content);
      if (content > 0.0)
        minimumPositive = std::min(minimumPositive, content);
    }
  }
  if (!std::isfinite(maximum))
    throw std::runtime_error("no finite bins to draw");

  TCanvas canvas("sideband_canvas", "sideband_canvas", 900, 850);
  TPad upper("upper", "upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("lower", "lower", 0.0, 0.0, 1.0, 0.30);
  upper.SetBottomMargin(0.025);
  upper.SetLeftMargin(0.14);
  upper.SetRightMargin(0.04);
  lower.SetTopMargin(0.035);
  lower.SetBottomMargin(0.32);
  lower.SetLeftMargin(0.14);
  lower.SetRightMargin(0.04);
  upper.Draw();
  lower.Draw();

  upper.cd();
  if (logarithmic && std::isfinite(minimumPositive)) {
    upper.SetLogy();
    histograms.front()->SetMinimum(std::max(1e-10, minimumPositive * 0.25));
    histograms.front()->SetMaximum(maximum * 25.0);
  } else {
    const double span = std::max(maximum - std::min(0.0, minimum), 1e-12);
    histograms.front()->SetMinimum(std::min(0.0, minimum) - 0.15 * span);
    histograms.front()->SetMaximum(maximum + 0.30 * span);
  }
  histograms.front()->SetTitle(title.c_str());
  histograms.front()->GetXaxis()->SetLabelSize(0.0);
  histograms.front()->GetYaxis()->SetTitle(yTitle.c_str());
  histograms.front()->GetYaxis()->SetTitleOffset(1.35);
  if (positiveHalf)
    histograms.front()->GetXaxis()->SetRangeUser(0.0, 1.0);
  histograms.front()->Draw("E1");
  for (std::size_t i = 1; i < histograms.size(); ++i)
    histograms[i]->Draw("E1 SAME");

  TLegend legend(0.48, 0.66, 0.94, 0.90);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetTextSize(0.040);
  for (std::size_t i = 0; i < histograms.size(); ++i)
    legend.AddEntry(histograms[i].get(), labels[i].c_str(), "lep");
  legend.Draw();

  lower.cd();
  std::vector<std::unique_ptr<TH1D>> ratios;
  double ratioMinimum = std::numeric_limits<double>::infinity();
  double ratioMaximum = -std::numeric_limits<double>::infinity();
  for (std::size_t i = 1; i < histograms.size(); ++i) {
    auto ratio = cloneHistogram(*histograms[i],
                                "sideband_ratio_" + std::to_string(i));
    ratio->Divide(histograms.front().get());
    for (int bin = 1; bin <= ratio->GetNbinsX(); ++bin) {
      if (positiveHalf && ratio->GetXaxis()->GetBinCenter(bin) < 0.0)
        continue;
      if (histograms.front()->GetBinContent(bin) == 0.0)
        continue;
      const double content = ratio->GetBinContent(bin);
      const double error = ratio->GetBinError(bin);
      if (!std::isfinite(content) || !std::isfinite(error))
        continue;
      ratioMinimum = std::min(ratioMinimum, content - error);
      ratioMaximum = std::max(ratioMaximum, content + error);
    }
    ratio->SetTitle("");
    ratio->GetYaxis()->SetTitle("Ratio to first");
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetTitleSize(0.10);
    ratio->GetYaxis()->SetTitleOffset(0.55);
    ratio->GetYaxis()->SetLabelSize(0.085);
    ratio->GetXaxis()->SetTitle("cos#chi");
    ratio->GetXaxis()->SetTitleSize(0.12);
    ratio->GetXaxis()->SetTitleOffset(1.05);
    ratio->GetXaxis()->SetLabelSize(0.10);
    if (positiveHalf)
      ratio->GetXaxis()->SetRangeUser(0.0, 1.0);
    ratios.push_back(std::move(ratio));
  }
  if (!std::isfinite(ratioMinimum) || !std::isfinite(ratioMaximum)) {
    ratioMinimum = 0.0;
    ratioMaximum = 2.0;
  }
  ratioMinimum = std::min(ratioMinimum, 1.0);
  ratioMaximum = std::max(ratioMaximum, 1.0);
  const double ratioSpan = std::max(ratioMaximum - ratioMinimum, 0.10);
  const double ratioLow = std::max(-2.0, ratioMinimum - 0.15 * ratioSpan);
  const double ratioHigh =
      std::min(ratioUpperLimit, ratioMaximum + 0.15 * ratioSpan);
  for (std::size_t i = 0; i < ratios.size(); ++i) {
    ratios[i]->GetYaxis()->SetRangeUser(ratioLow, ratioHigh);
    ratios[i]->Draw(i == 0 ? "E1" : "E1 SAME");
  }
  TLine unity(positiveHalf ? 0.0 : -1.0, 1.0, 1.0, 1.0);
  unity.SetLineStyle(2);
  unity.Draw();

  canvas.SaveAs(output.c_str());
  std::cout << "Wrote " << output << '\n';
}

void drawMcComparison(std::vector<std::unique_ptr<TH1D>> histograms,
                      const std::vector<std::string> &labels,
                      const std::string &title, const std::string &output,
                      bool positiveHalf) {
  if (histograms.empty() || histograms.size() != labels.size())
    throw std::runtime_error("invalid MC comparison inputs");
  for (auto &histogram : histograms) {
    if (histogram->GetNbinsX() % cosChiRebin != 0)
      throw std::runtime_error("coschi rebin factor does not divide bins");
    histogram->Rebin(cosChiRebin);
    normalize(*histogram, positiveHalf);
  }

  const std::vector<int> colors = {kBlack,     kAzure + 1,   kRed + 1,
                                   kGreen + 2, kMagenta + 1, kOrange + 7};
  const std::vector<int> markers = {20, 21, 22, 23, 33, 34};
  double maximum = 0.0;
  double minimumPositive = std::numeric_limits<double>::max();
  for (std::size_t index = 0; index < histograms.size(); ++index) {
    TH1D &histogram = *histograms[index];
    histogram.SetLineColor(colors[index % colors.size()]);
    histogram.SetMarkerColor(colors[index % colors.size()]);
    histogram.SetMarkerStyle(markers[index % markers.size()]);
    histogram.SetMarkerSize(1.0);
    histogram.SetLineWidth(2);
    maximum = std::max(maximum, histogram.GetMaximum());
    for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
      const double content = histogram.GetBinContent(bin);
      if (content > 0.0)
        minimumPositive = std::min(minimumPositive, content);
    }
  }

  TCanvas canvas("mc_comparison_canvas", "mc_comparison_canvas", 950, 900);
  TPad upper("mc_upper", "mc_upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("mc_lower", "mc_lower", 0.0, 0.0, 1.0, 0.30);
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
  histograms.front()->SetTitle(title.c_str());
  histograms.front()->GetYaxis()->SetTitle("Normalized EEC");
  histograms.front()->GetYaxis()->SetTitleOffset(1.35);
  histograms.front()->GetXaxis()->SetLabelSize(0.0);
  histograms.front()->SetMinimum(std::max(1e-8, minimumPositive * 0.2));
  histograms.front()->SetMaximum(maximum * 30.0);
  if (positiveHalf)
    histograms.front()->GetXaxis()->SetRangeUser(0.0, 1.0);
  histograms.front()->Draw("E1");
  for (std::size_t index = 1; index < histograms.size(); ++index)
    histograms[index]->Draw("E1 SAME");

  TLegend legend(0.38, 0.62, 0.94, 0.90);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetTextSize(0.047);
  for (std::size_t index = 0; index < histograms.size(); ++index)
    legend.AddEntry(histograms[index].get(), labels[index].c_str(), "lep");
  legend.Draw();

  lower.cd();
  std::vector<std::unique_ptr<TH1D>> ratios;
  for (std::size_t index = 1; index < histograms.size(); ++index) {
    auto ratio = cloneHistogram(*histograms[index],
                                "mc_ratio_" + std::to_string(index));
    ratio->Divide(histograms.front().get());
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

std::vector<std::string> ptSuffixes() {
  std::vector<std::string> result = {""};
  const auto &binned = unified_eec::jpsiPtBinSuffixes();
  result.insert(result.end(), binned.begin(), binned.end());
  return result;
}

std::string outputSuffix(bool positiveHalf) {
  return positiveHalf ? "_positive.pdf" : ".pdf";
}

void processHistogram(const Options &options, const std::vector<Sample> &mc,
                      const std::string &scope, const std::string &suffix,
                      TFile &rootOutput) {
  const std::string histogramName = "eec_" + scope + "_all" + suffix;
  auto signal = loadHistogram(options.signalFile, options.rootDirectory,
                              histogramName, "signal_input");
  auto lower = loadHistogram(options.lowerFile, options.rootDirectory,
                             histogramName, "lower_input");
  auto upper = loadHistogram(options.upperFile, options.rootDirectory,
                             histogramName, "upper_input");

  auto background = cloneHistogram(*lower, "background_input");
  if (!background->Add(upper.get()))
    throw std::runtime_error("incompatible lower/upper sideband histograms");
  const double backgroundScale =
      options.signalWidth / (options.lowerWidth + options.upperWidth);
  background->Scale(backgroundScale);

  auto extracted = cloneHistogram(*signal, "extracted_input");
  if (!extracted->Add(background.get(), -1.0))
    throw std::runtime_error("incompatible signal/sideband histograms");

  const std::string key = scope + suffix;
  rootOutput.cd();
  auto writeCopy = [&](const TH1D &input, const std::string &prefix) {
    auto copy = cloneHistogram(input, prefix + "_" + key);
    copy->Write();
  };
  writeCopy(*signal, "data_signal_window_raw");
  writeCopy(*lower, "data_lower_sideband_raw");
  writeCopy(*upper, "data_upper_sideband_raw");
  writeCopy(*background, "background_estimate_raw");
  writeCopy(*extracted, "data_signal_extracted_raw");

  const std::string description = scope == "alljets" ? "all selected jets"
                                                       : "J/#psi jet";
  const std::string title = description + ", " + ptDescription(suffix);
  for (const bool positiveHalf :
       options.drawPositiveHalf ? std::vector<bool>{false, true}
                                : std::vector<bool>{false}) {
    std::vector<std::unique_ptr<TH1D>> windows;
    windows.push_back(cloneHistogram(*signal, "window_signal"));
    windows.push_back(cloneHistogram(*lower, "window_lower"));
    windows.push_back(cloneHistogram(*upper, "window_upper"));
    windows.push_back(cloneHistogram(*background, "window_background"));
    drawOverlay(std::move(windows),
                {"Data 2.9-3.3", "Lower sideband 2.7-2.9",
                 "Upper sideband 3.3-3.5", "Combined background estimate"},
                "Mass-window EEC shapes, " + title,
                path(options.outputDirectory,
                     "window_shapes_" + key + outputSuffix(positiveHalf)),
                "Normalized EEC", true, positiveHalf, true);

    std::vector<std::unique_ptr<TH1D>> subtraction;
    subtraction.push_back(cloneHistogram(*signal, "raw_signal"));
    subtraction.push_back(cloneHistogram(*background, "raw_background"));
    subtraction.push_back(cloneHistogram(*extracted, "raw_extracted"));
    drawOverlay(std::move(subtraction),
                {"Data 2.9-3.3", "Sideband background estimate",
                 "Extracted signal"},
                "Raw sideband subtraction, " + title,
                path(options.outputDirectory,
                     "raw_subtraction_" + key + outputSuffix(positiveHalf)),
                "Raw weighted entries", false, positiveHalf, true, 1.0);

    std::vector<std::unique_ptr<TH1D>> normalizedSubtraction;
    normalizedSubtraction.push_back(
        cloneHistogram(*signal, "normalized_signal"));
    normalizedSubtraction.push_back(
        cloneHistogram(*background, "normalized_background"));
    normalizedSubtraction.push_back(
        cloneHistogram(*extracted, "normalized_extracted"));
    drawOverlay(
        std::move(normalizedSubtraction),
        {"Data 2.9-3.3", "Sideband background estimate", "Extracted signal"},
        "Normalized sideband-subtraction shapes, " + title,
        path(options.outputDirectory,
             "normalized_subtraction_" + key + outputSuffix(positiveHalf)),
        "Normalized EEC", true, positiveHalf, true);

    std::vector<std::unique_ptr<TH1D>> comparison;
    comparison.push_back(cloneHistogram(*extracted, "comparison_extracted"));
    std::vector<std::string> comparisonLabels = {
        "Sideband-subtracted data 2024G"};
    for (std::size_t i = 0; i < mc.size(); ++i) {
      comparison.push_back(loadSample(mc[i], options.rootDirectory,
                                      histogramName,
                                      "comparison_mc_" + std::to_string(i)));
      comparisonLabels.push_back(mc[i].label);
    }
    drawMcComparison(
        std::move(comparison), comparisonLabels,
        "Extracted signal vs CMS MC, " + title,
        path(options.outputDirectory,
             "signal_vs_mc_" + key + outputSuffix(positiveHalf)),
        positiveHalf);
  }
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
    gStyle->SetLegendFont(42);

    const std::vector<Sample> mcSamples = {
        {"Soft QCD",
         {path(options.mcDirectory,
               "JPsiMuMu_Fil-JPsiNo-2MuPtEta_TuneCP5_13p6TeV_pythia8-"
               "evtgen-RunIIISummer24.root")}},
        {"Hard QCD, onia off + CR0",
         {path(options.mcDirectory,
               "QCD_Bin-PT-15to7000_Par-PT-flat2022_"
               "Par-JpsiMuMu_pythia8-RunIIISummer24_Private.root"),
          path(options.mcDirectory,
               "QCD_Bin-PT-15to7000_Par-PT-flat2022_"
               "Par-JpsiMuMu_pythia8-RunIIISummer24_ext1_Private.root")}},
        {"Hard QCD, onia on + CR1",
         {path(options.mcDirectory,
               "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-"
               "OniaOn-CR1_pythia8311-RunIIISummer24_Private.root"),
          path(options.mcDirectory,
               "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu-Par-"
               "OniaOn-CR1_pythia8311-RunIIISummer24_ext1_Private.root")}},
        {"Prompt charmonium",
         {path(options.mcDirectory,
               "Jpsito2Mu_Bin-PTJpsi-8_TuneCP5_13p6TeV_pythia8-"
               "RunIIISummer24.root")}}};

    const std::string rootOutputName =
        path(options.outputDirectory, options.rootOutput);
    std::unique_ptr<TFile> rootOutput(
        TFile::Open(rootOutputName.c_str(), "RECREATE"));
    if (!rootOutput || rootOutput->IsZombie())
      throw std::runtime_error("cannot create " + rootOutputName);

    const double backgroundScale =
        options.signalWidth / (options.lowerWidth + options.upperWidth);
    std::cout << "Background estimate = " << backgroundScale
              << " * (lower + upper); extracted = signal window - background"
              << '\n';
    std::cout << "IMPORTANT: data windows and MC must use identical trigger, "
                 "hot-zone, prompt, muon, and jet selections.\n";
    for (const std::string scope : {"alljets", "jpsijet"}) {
      for (const auto &suffix : ptSuffixes()) {
        try {
          processHistogram(options, mcSamples, scope, suffix, *rootOutput);
        } catch (const std::exception &error) {
          std::cerr << "SKIP " << scope << suffix << ": " << error.what()
                    << '\n';
        }
      }
    }
    rootOutput->Close();
    std::cout << "Wrote raw subtraction histograms to " << rootOutputName
              << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    usage(argv[0]);
    return 1;
  }
}
