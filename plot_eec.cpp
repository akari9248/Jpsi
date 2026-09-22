#include "EECCommon.h"
#include "include/MotherCategory.h"

#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2D.h>
#include <THStack.h>
#include <TLegend.h>
#include <TLine.h>
#include <TLatex.h>
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
  std::string cmsDirectory = "/eos/user/s/shuangyu/public/Jpsi/eec_unified_cms";
  std::string privateDirectory =
      "/eos/user/s/shuangyu/public/Jpsi/eec_private_feeddown";
  std::string outputDirectory = "plots_eec_cms_private";
  bool drawPositiveHalf = true;
  bool drawCms = true;
  bool drawPrivate = true;
};

struct Source {
  std::string file;
  std::string directory;
};

struct Sample {
  std::string label;
  std::vector<Source> sources;
};

struct ComparisonStyle {
  std::string xAxisTitle = "cos#chi";
  std::string yAxisTitle = "Normalized EEC";
  bool positiveHalf = false;
  bool logY = true;
  bool logRatio = false;
  int rebin = 1;
  bool fixedRatioRange = false;
  double ratioMinimum = 0.5;
  double ratioMaximum = 1.5;
};

struct FractionCategory {
  std::string key;
  std::string label;
  std::vector<std::string> directories;
  int color;
};

void drawNdcTitle(const std::string &title, double x, double y) {
  TLatex label;
  label.SetNDC();
  label.SetTextFont(42);
  label.SetTextAlign(13);
  label.SetTextSize(title.size() > 85 ? 0.030
                                     : (title.size() > 65 ? 0.034 : 0.040));
  label.DrawLatex(x, y, title.c_str());
}

void usage(const char *program) {
  std::cerr << "Usage: " << program
            << " [--cms-dir DIR] [--private-dir DIR] [--output-dir DIR]"
               " [--positive-half on|off] [--draw-cms on|off]"
               " [--draw-private on|off]\n";
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
    else if (argument == "--draw-cms")
      options.drawCms = parseOnOff(value());
    else if (argument == "--draw-private")
      options.drawPrivate = parseOnOff(value());
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

std::string jetPtDescription(const std::string &suffix) {
  if (suffix.empty())
    return "inclusive jet p_{T}";
  const auto &suffixes = unified_eec::momentumFractionJetPtBinSuffixes();
  const auto &edges = unified_eec::momentumFractionJetPtBinEdges();
  const auto found = std::find(suffixes.begin(), suffixes.end(), suffix);
  if (found == suffixes.end())
    return suffix;
  const std::size_t index = found - suffixes.begin();
  if (index + 1 == suffixes.size())
    return "jet p_{T} #geq 600 GeV";
  return "jet p_{T} " + std::to_string(static_cast<int>(edges[index])) +
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
                    const ComparisonStyle &style) {
  if (input.size() != labels.size() || input.empty())
    throw std::runtime_error("invalid plotting inputs");

  for (auto &histogram : input) {
    if (style.rebin > 1) {
      if (histogram->GetNbinsX() % style.rebin != 0)
        throw std::runtime_error("rebin factor does not divide histogram bins");
      histogram->Rebin(style.rebin);
    }
    prepareShape(*histogram, style.positiveHalf);
  }

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
    histogram.SetMarkerSize(1.0);
    histogram.SetLineWidth(2);
    maximum = std::max(maximum, histogram.GetMaximum());
    for (int bin = 1; bin <= histogram.GetNbinsX(); ++bin) {
      const double content = histogram.GetBinContent(bin);
      if (content > 0.0)
        minimumPositive = std::min(minimumPositive, content);
    }
  }

  TCanvas canvas("eec_canvas", "eec_canvas", 950, 900);
  TPad upper("upper", "upper", 0.0, 0.30, 1.0, 1.0);
  TPad lower("lower", "lower", 0.0, 0.0, 1.0, 0.30);
  upper.SetBottomMargin(0.025);
  upper.SetLeftMargin(0.14);
  upper.SetRightMargin(0.04);
  upper.SetTopMargin(0.13);
  upper.SetLogy(style.logY);
  lower.SetTopMargin(0.035);
  lower.SetBottomMargin(0.32);
  lower.SetLeftMargin(0.14);
  lower.SetRightMargin(0.04);
  lower.SetLogy(style.logRatio);
  upper.Draw();
  lower.Draw();

  upper.cd();
  input.front()->SetTitle("");
  input.front()->GetYaxis()->SetTitle(style.yAxisTitle.c_str());
  input.front()->GetYaxis()->SetTitleSize(0.055);
  input.front()->GetYaxis()->SetTitleOffset(1.15);
  input.front()->GetYaxis()->SetLabelSize(0.048);
  input.front()->GetXaxis()->SetLabelSize(0.0);
  if (style.logY) {
    input.front()->SetMinimum(std::max(1e-8, minimumPositive * 0.35));
    input.front()->SetMaximum(maximum * 6.0);
  } else {
    input.front()->SetMinimum(0.0);
    input.front()->SetMaximum(maximum * 1.65);
  }
  if (style.positiveHalf)
    input.front()->GetXaxis()->SetRangeUser(0.0, 1.0);
  input.front()->Draw("E1");
  for (std::size_t index = 1; index < input.size(); ++index)
    input[index]->Draw("E1 SAME");
  drawNdcTitle(title, 0.14, 0.965);

  TLegend legend(0.38, 0.60, 0.94, 0.86);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetTextSize(0.047);
  for (std::size_t index = 0; index < input.size(); ++index)
    legend.AddEntry(input[index].get(), labels[index].c_str(), "lep");
  legend.Draw();

  lower.cd();
  std::vector<std::unique_ptr<TH1D>> ratios;
  double ratioMinimum = std::numeric_limits<double>::max();
  double ratioMaximum = std::numeric_limits<double>::lowest();
  for (std::size_t index = 1; index < input.size(); ++index) {
    auto ratio = std::unique_ptr<TH1D>(static_cast<TH1D *>(
        input[index]->Clone(("ratio_" + std::to_string(index)).c_str())));
    ratio->SetDirectory(nullptr);
    ratio->Divide(input.front().get());
    ratio->SetTitle("");
    ratio->GetYaxis()->SetTitle("Ratio");
    ratio->GetXaxis()->SetTitle(style.xAxisTitle.c_str());
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetTitleSize(0.115);
    ratio->GetYaxis()->SetTitleOffset(0.55);
    ratio->GetYaxis()->SetLabelSize(0.10);
    ratio->GetXaxis()->SetTitleSize(0.135);
    ratio->GetXaxis()->SetLabelSize(0.115);
    ratio->GetXaxis()->SetTitleOffset(0.95);
    ratio->GetXaxis()->CenterTitle();
    if (style.positiveHalf)
      ratio->GetXaxis()->SetRangeUser(0.0, 1.0);
    for (int bin = 1; bin <= ratio->GetNbinsX(); ++bin) {
      const double center = ratio->GetXaxis()->GetBinCenter(bin);
      if (style.positiveHalf && center < 0.0)
        continue;
      if (input.front()->GetBinContent(bin) <= 0.0)
        continue;
      const double value = ratio->GetBinContent(bin);
      if (!std::isfinite(value) || (style.logRatio && value <= 0.0))
        continue;
      ratioMinimum = std::min(ratioMinimum, value);
      ratioMaximum = std::max(ratioMaximum, value);
    }
    ratios.push_back(std::move(ratio));
  }
  if (style.fixedRatioRange) {
    ratioMinimum = style.ratioMinimum;
    ratioMaximum = style.ratioMaximum;
  } else if (ratioMinimum == std::numeric_limits<double>::max()) {
    ratioMinimum = 0.8;
    ratioMaximum = 1.2;
  } else {
    ratioMinimum = std::min(ratioMinimum, 1.0);
    ratioMaximum = std::max(ratioMaximum, 1.0);
    if (style.logRatio) {
      ratioMinimum = std::max(1e-3, ratioMinimum / 1.5);
      ratioMaximum *= 1.5;
    } else {
      const double ratioSpan = ratioMaximum - ratioMinimum;
      const double ratioPadding = std::max(0.05, 0.12 * ratioSpan);
      ratioMinimum = std::max(0.0, ratioMinimum - ratioPadding);
      ratioMaximum += ratioPadding;
      if (ratioMaximum - ratioMinimum < 0.4) {
        const double middle = 0.5 * (ratioMinimum + ratioMaximum);
        ratioMinimum = std::max(0.0, middle - 0.2);
        ratioMaximum = middle + 0.2;
      }
    }
  }
  for (std::size_t index = 0; index < ratios.size(); ++index) {
    ratios[index]->GetYaxis()->SetRangeUser(ratioMinimum, ratioMaximum);
    ratios[index]->Draw(index == 0 ? "E1" : "E1 SAME");
  }
  const double xMinimum = style.positiveHalf
                              ? 0.0
                              : input.front()->GetXaxis()->GetXmin();
  const double xMaximum = input.front()->GetXaxis()->GetXmax();
  TLine unity(xMinimum, 1.0, xMaximum, 1.0);
  unity.SetLineStyle(2);
  unity.Draw();

  canvas.SaveAs(output.c_str());
  std::cout << "Wrote " << output << '\n';
}

Sample withDirectories(const Sample &sample,
                       const std::vector<std::string> &directories) {
  Sample result{sample.label, {}};
  for (const auto &source : sample.sources)
    for (const auto &directory : directories)
      result.sources.push_back({source.file, directory});
  return result;
}

void drawCategoryFractions(const std::vector<Sample> &samples,
                           const std::vector<std::string> &axisLabels,
                           const std::vector<FractionCategory> &categories,
                           const std::string &histogramName,
                           const std::string &title,
                           const std::string &output) {
  if (samples.empty() || samples.size() != axisLabels.size() ||
      categories.empty())
    throw std::runtime_error("invalid category-fraction inputs");

  std::vector<std::vector<double>> fractions(
      categories.size(), std::vector<double>(samples.size(), 0.0));
  std::vector<bool> hasSelectedEvents(samples.size(), false);

  std::cout << "Category fractions among selected J/psi-jet events in "
            << histogramName << ":\n";
  for (std::size_t sampleIndex = 0; sampleIndex < samples.size();
       ++sampleIndex) {
    std::vector<double> yields(categories.size(), 0.0);
    double total = 0.0;
    for (std::size_t category = 0; category < categories.size(); ++category) {
      const Sample categorySample =
          withDirectories(samples[sampleIndex],
                          categories[category].directories);
      auto selectedEvents = loadAndAdd(
          categorySample, histogramName,
          "category_yield_" + std::to_string(sampleIndex) + "_" +
              std::to_string(category));
      // Include underflow and overflow so that the open-ended >=200 GeV bin
      // is counted even though the stored J/psi-pT histogram ends at 200 GeV.
      yields[category] = selectedEvents->Integral(
          0, selectedEvents->GetNbinsX() + 1);
      if (!std::isfinite(yields[category]) || yields[category] < 0.0)
        throw std::runtime_error("invalid category yield in " +
                                 samples[sampleIndex].label);
      total += yields[category];
    }
    std::cout << "  " << samples[sampleIndex].label;
    if (!(total > 0.0)) {
      std::cout << "  no selected events\n";
      continue;
    }
    hasSelectedEvents[sampleIndex] = true;
    for (std::size_t category = 0; category < categories.size(); ++category) {
      fractions[category][sampleIndex] = yields[category] / total;
      std::cout << "  " << categories[category].key << "="
                << 100.0 * fractions[category][sampleIndex] << "%";
    }
    std::cout << '\n';
  }

  TCanvas canvas("category_canvas", "category_canvas", 1200, 850);
  canvas.SetLeftMargin(0.12);
  canvas.SetRightMargin(0.33);
  canvas.SetBottomMargin(0.14);
  canvas.SetTopMargin(0.13);

  THStack stack("category_stack", ";Private sample;Fraction");
  std::vector<std::unique_ptr<TH1D>> histograms;
  for (std::size_t category = 0; category < categories.size(); ++category) {
    auto histogram = std::make_unique<TH1D>(
        ("category_fraction_" + std::to_string(category)).c_str(), "",
        static_cast<int>(samples.size()), 0.5,
        static_cast<double>(samples.size()) + 0.5);
    histogram->SetDirectory(nullptr);
    histogram->SetFillColor(categories[category].color);
    histogram->SetLineColor(kBlack);
    histogram->SetLineWidth(1);
    for (std::size_t sampleIndex = 0; sampleIndex < samples.size();
         ++sampleIndex) {
      histogram->SetBinContent(static_cast<int>(sampleIndex) + 1,
                               fractions[category][sampleIndex]);
      histogram->GetXaxis()->SetBinLabel(static_cast<int>(sampleIndex) + 1,
                                         axisLabels[sampleIndex].c_str());
    }
    stack.Add(histogram.get());
    histograms.push_back(std::move(histogram));
  }

  stack.Draw("HIST");
  stack.SetMinimum(0.0);
  stack.SetMaximum(1.0);
  stack.GetYaxis()->SetTitleSize(0.052);
  stack.GetYaxis()->SetTitleOffset(1.05);
  stack.GetYaxis()->SetLabelSize(0.044);
  stack.GetXaxis()->SetTitleSize(0.052);
  stack.GetXaxis()->SetTitleOffset(1.05);
  stack.GetXaxis()->SetLabelSize(0.042);
  stack.GetXaxis()->LabelsOption("h");
  stack.GetXaxis()->CenterTitle();
  drawNdcTitle(title, 0.12, 0.965);

  TLegend legend(0.68, 0.45, 0.99, 0.86);
  legend.SetBorderSize(0);
  legend.SetFillStyle(0);
  legend.SetTextSize(0.033);
  for (std::size_t category = 0; category < categories.size(); ++category)
    legend.AddEntry(histograms[category].get(),
                    categories[category].label.c_str(), "f");
  legend.Draw();

  TLatex percentage;
  percentage.SetTextFont(42);
  percentage.SetTextAlign(22);
  percentage.SetTextSize(0.026);
  for (std::size_t sampleIndex = 0; sampleIndex < samples.size();
       ++sampleIndex) {
    if (!hasSelectedEvents[sampleIndex])
      continue;
    double bottom = 0.0;
    for (std::size_t category = 0; category < categories.size(); ++category) {
      const double fraction = fractions[category][sampleIndex];
      if (fraction >= 0.04)
        percentage.DrawLatex(
            static_cast<double>(sampleIndex) + 1.0, bottom + 0.5 * fraction,
            (std::to_string(
                 static_cast<int>(std::lround(100.0 * fraction))) +
             "%")
                .c_str());
      bottom += fraction;
    }
  }

  TLatex noEvents;
  noEvents.SetTextFont(42);
  noEvents.SetTextAlign(12);
  noEvents.SetTextAngle(90.0);
  noEvents.SetTextSize(0.030);
  for (std::size_t sampleIndex = 0; sampleIndex < samples.size();
       ++sampleIndex)
    if (!hasSelectedEvents[sampleIndex])
      noEvents.DrawLatex(static_cast<double>(sampleIndex) + 1.0, 0.05,
                         "No events");

  canvas.SaveAs(output.c_str());
  std::cout << "Wrote " << output << '\n';
}

std::vector<std::string> allPtSuffixes() {
  std::vector<std::string> result = {""};
  const auto &binned = unified_eec::jpsiPtBinSuffixes();
  result.insert(result.end(), binned.begin(), binned.end());
  return result;
}

std::vector<std::string> allMomentumFractionJetPtSuffixes() {
  std::vector<std::string> result = {""};
  const auto &binned = unified_eec::momentumFractionJetPtBinSuffixes();
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

using LoadedComparison =
    std::pair<std::vector<std::unique_ptr<TH1D>>, std::vector<std::string>>;

LoadedComparison loadForComparison(const std::vector<Sample> &samples,
                                   const std::string &histogramName,
                                   const std::string &namePrefix,
                                   bool positiveHalf, bool omitEmpty) {
  LoadedComparison loaded;
  for (std::size_t index = 0; index < samples.size(); ++index) {
    auto histogram = loadAndAdd(samples[index], histogramName,
                                namePrefix + std::to_string(index));
    const int firstBin =
        positiveHalf ? histogram->GetXaxis()->FindFixBin(0.0) : 1;
    const double integral = histogram->Integral(firstBin,
                                                histogram->GetNbinsX());
    if (omitEmpty && (!std::isfinite(integral) || integral <= 0.0)) {
      std::cerr << "OMIT " << samples[index].label << " from " << histogramName
                << ": empty histogram\n";
      continue;
    }
    loaded.first.push_back(std::move(histogram));
    loaded.second.push_back(samples[index].label);
  }
  return loaded;
}

void drawCategoryFractionFamily(const std::vector<Sample> &samples,
                                const std::vector<std::string> &axisLabels,
                                const std::string &outputDirectory) {
  const std::vector<int> categoryIds = {1, 2, 3, 4, 5, 6, 7};
  const std::vector<std::string> labels = {
      "b-hadron", "Charmonium feed-down", "Gluon/proton", "Quark",
      "^{3}S_{1}^{[8]}", "^{1}S_{0}^{[8]}", "^{3}P_{J}^{[8]}"};
  const std::vector<int> colors = {
      kAzure + 1, kOrange + 7, kGreen + 2, kViolet + 1,
      kRed + 1,   kMagenta + 1, kCyan + 2};
  std::vector<FractionCategory> categories;
  for (std::size_t index = 0; index < categoryIds.size(); ++index) {
    const int category = categoryIds[index];
    std::vector<std::string> directories;
    if (category == 2) {
      for (int component = 0;
           component < jpsi_origin::numberOfCharmoniumComponents(); ++component)
        directories.push_back(
            jpsi_origin::charmoniumComponentDirectoryName(component));
    } else
      directories.push_back("PrivateGen_cat" + std::to_string(category));
    categories.push_back({"cat" + std::to_string(category), labels[index],
                          std::move(directories), colors[index]});
  }
  for (const auto &suffix : allPtSuffixes()) {
    try {
      drawCategoryFractions(
          samples, axisLabels, categories, "jpsi_pt" + suffix,
          "Private-sample category composition, " + ptDescription(suffix),
          outputDirectory + "/private_category_fractions" + suffix + ".pdf");
    } catch (const std::exception &error) {
      std::cerr << "SKIP private_category_fractions" << suffix << ": "
                << error.what() << '\n';
    }
  }
}

void drawCharmoniumFractionFamily(const std::vector<Sample> &samples,
                                  const std::vector<std::string> &axisLabels,
                                  const std::string &outputDirectory) {
  const std::vector<FractionCategory> categories = {
      {"from_b", "b-hadron feed-down",
       {jpsi_origin::charmoniumComponentDirectoryName(0)},
       TColor::GetColor("#8C564B")},
      {"psi_2S", "#psi(2S)",
       {jpsi_origin::charmoniumComponentDirectoryName(1)}, kAzure + 1},
      {"chi_c1", "#chi_{c1}",
       {jpsi_origin::charmoniumComponentDirectoryName(2)}, kOrange + 7},
      {"chi_c2", "#chi_{c2}",
       {jpsi_origin::charmoniumComponentDirectoryName(3)}, kRed + 1},
      {"others", "Other charmonium",
       {jpsi_origin::charmoniumComponentDirectoryName(4)}, kGray + 1}};

  for (const auto &suffix : allPtSuffixes()) {
    try {
      drawCategoryFractions(
          samples, axisLabels, categories, "jpsi_pt" + suffix,
          "Charmonium feed-down composition, " + ptDescription(suffix),
          outputDirectory + "/private_charmonium_fractions" + suffix +
              ".pdf");
    } catch (const std::exception &error) {
      std::cerr << "SKIP private_charmonium_fractions" << suffix << ": "
                << error.what() << '\n';
    }
  }
}

void drawFamily(const std::vector<Sample> &samples,
                const std::string &outputPrefix,
                const std::string &descriptionPrefix,
                const std::string &outputDirectory, bool positiveHalf,
                bool omitEmpty = false) {
  ComparisonStyle fullStyle;
  fullStyle.rebin = 2;
  for (const auto &suffix : allPtSuffixes()) {
    const std::string histogramName = "eec_alljets_all" + suffix;
    try {
      auto loaded = loadForComparison(samples, histogramName,
                                      outputPrefix + suffix + "_", false,
                                      omitEmpty);
      if (loaded.first.size() < 2)
        throw std::runtime_error("fewer than two non-empty samples");
      const std::string title =
          descriptionPrefix + ", " + ptDescription(suffix);
      drawComparison(std::move(loaded.first), loaded.second, title,
                     outputDirectory + "/" + outputPrefix + suffix + ".pdf",
                     fullStyle);
      if (positiveHalf) {
        auto positiveLoaded = loadForComparison(
            samples, histogramName, outputPrefix + suffix + "_positive_", true,
            omitEmpty);
        if (positiveLoaded.first.size() < 2)
          throw std::runtime_error(
              "fewer than two non-empty positive-half samples");
        ComparisonStyle positiveStyle = fullStyle;
        positiveStyle.positiveHalf = true;
        drawComparison(
            std::move(positiveLoaded.first), positiveLoaded.second,
            title + ", cos#chi > 0",
            outputDirectory + "/" + outputPrefix + suffix + "_positive.pdf",
            positiveStyle);
      }
    } catch (const std::exception &error) {
      std::cerr << "SKIP " << outputPrefix << suffix << ": " << error.what()
                << '\n';
    }
  }
}

void drawObservableFamily(const std::vector<Sample> &samples,
                          const std::string &histogramPrefix,
                          const std::string &outputPrefix,
                          const std::string &descriptionPrefix,
                          const std::string &xAxisTitle,
                          const std::string &outputDirectory) {
  ComparisonStyle style;
  style.xAxisTitle = xAxisTitle;
  style.yAxisTitle = "Normalized J/#psi jets";
  style.logY = false;
  style.rebin = 4;
  style.fixedRatioRange = true;
  for (const auto &suffix : allMomentumFractionJetPtSuffixes()) {
    const std::string histogramName = histogramPrefix + suffix;
    try {
      auto histograms =
          loadSamples(samples, histogramName, outputPrefix + suffix + "_");
      drawComparison(
          std::move(histograms), labels(samples),
          descriptionPrefix + ", " + jetPtDescription(suffix),
          outputDirectory + "/" + outputPrefix + suffix + ".pdf", style);
    } catch (const std::exception &error) {
      std::cerr << "SKIP " << outputPrefix << suffix << ": " << error.what()
                << '\n';
    }
  }
}

std::vector<Sample> privateCategorySamples(const std::string &file) {
  Sample charmonium{"charmonium feed-down", {}};
  for (int component = 0;
       component < jpsi_origin::numberOfCharmoniumComponents(); ++component)
    charmonium.sources.push_back(
        {file, jpsi_origin::charmoniumComponentDirectoryName(component)});
  return {{"b-hadron", {{file, "PrivateGen_cat1"}}},
          std::move(charmonium),
          {"gluon/proton", {{file, "PrivateGen_cat3"}}},
          {"quark", {{file, "PrivateGen_cat4"}}},
          {"Octet (^{3}S_{1}^{[8]}+^{1}S_{0}^{[8]}+^{3}P_{J}^{[8]})",
           {{file, "PrivateGen_cat5"},
            {file, "PrivateGen_cat6"},
            {file, "PrivateGen_cat7"}}}};
}

std::vector<std::string> charmoniumComponentDirectories() {
  std::vector<std::string> result;
  for (int component = 0;
       component < jpsi_origin::numberOfCharmoniumComponents(); ++component)
    result.push_back(
        jpsi_origin::charmoniumComponentDirectoryName(component));
  return result;
}

std::vector<Sample>
privateSamplesForCategory(const std::vector<Sample> &samples,
                          const std::vector<std::string> &directories) {
  std::vector<Sample> result;
  for (const auto &sample : samples) {
    Sample categorized{sample.label, {}};
    for (const auto &source : sample.sources)
      for (const auto &directory : directories)
        categorized.sources.push_back({source.file, directory});
    result.push_back(std::move(categorized));
  }
  return result;
}

void drawPrivateFeeddownTagDiagnostics(
    const std::vector<Sample> &samples,
    const std::vector<std::string> &axisLabels,
    const std::string &outputDirectory) {
  struct Diagnostics {
    std::string label;
    std::string key;
    std::unique_ptr<TH2D> migration;
    std::array<double, 6> metrics{};
  };
  const std::array<const char *, 4> keys = {
      "oniaoff_cr0", "oniaoff_cr1", "oniaon_cr0", "oniaon_cr1"};
  std::vector<Diagnostics> available;
  for (std::size_t index = 0; index < samples.size(); ++index) {
    if (samples[index].sources.empty() ||
        gSystem->AccessPathName(samples[index].sources.front().file.c_str()))
      continue;
    std::unique_ptr<TFile> file(
        TFile::Open(samples[index].sources.front().file.c_str(), "READ"));
    if (!file || file->IsZombie())
      continue;
    TH2D *inputMigration = nullptr;
    TH1D *inputMetrics = nullptr;
    file->GetObject(
        "PrivateFeeddownTagDiagnostics/private_feeddown_tag_migration",
        inputMigration);
    file->GetObject(
        "PrivateFeeddownTagDiagnostics/private_feeddown_tag_metrics",
        inputMetrics);
    if (!inputMigration || !inputMetrics) {
      std::cerr << "SKIP private feed-down diagnostics for "
                << samples[index].label << ": histograms not found\n";
      continue;
    }
    Diagnostics diagnostic;
    diagnostic.label = axisLabels.at(index);
    diagnostic.key = keys.at(index);
    diagnostic.migration.reset(static_cast<TH2D *>(inputMigration->Clone(
        ("feeddown_migration_" + diagnostic.key).c_str())));
    diagnostic.migration->SetDirectory(nullptr);
    for (int metric = 0; metric < 6; ++metric)
      diagnostic.metrics.at(metric) = inputMetrics->GetBinContent(metric + 1);
    available.push_back(std::move(diagnostic));
  }
  if (available.empty())
    return;

  gStyle->SetPaintTextFormat(".2f");
  auto drawMigration = [&](TH2D &migration, const std::string &title,
                           const std::string &zTitle,
                           const std::string &output) {
    migration.SetTitle("");
    migration.GetXaxis()->SetTitle("Reconstructed tag");
    migration.GetYaxis()->SetTitle("Truth origin");
    migration.SetMinimum(0.0);
    migration.SetMaximum(1.0);
    migration.GetYaxis()->SetRange(2, 5);
    migration.GetZaxis()->SetTitle(zTitle.c_str());
    migration.GetXaxis()->SetTitleOffset(1.10);
    migration.GetXaxis()->SetTitleSize(0.048);
    migration.GetXaxis()->SetLabelSize(0.040);
    migration.GetXaxis()->CenterTitle();
    migration.GetYaxis()->SetTitleOffset(2.65);
    migration.GetYaxis()->SetTitleSize(0.048);
    migration.GetYaxis()->SetLabelSize(0.040);
    migration.GetYaxis()->CenterTitle();
    migration.GetZaxis()->SetTitleOffset(1.10);
    migration.SetMarkerSize(1.5);
    TCanvas canvas("feeddown_migration_canvas", "", 1000, 850);
    canvas.SetLeftMargin(0.25);
    canvas.SetRightMargin(0.17);
    canvas.SetBottomMargin(0.15);
    canvas.SetTopMargin(0.13);
    migration.Draw("COLZ TEXT");
    drawNdcTitle(title, 0.25, 0.965);
    canvas.SaveAs(output.c_str());
  };

  for (auto &diagnostic : available) {
    auto horizontal = std::unique_ptr<TH2D>(static_cast<TH2D *>(
        diagnostic.migration->Clone(("feeddown_horizontal_" + diagnostic.key)
                                        .c_str())));
    horizontal->SetDirectory(nullptr);
    for (int truth = 2; truth <= horizontal->GetNbinsY(); ++truth) {
      const double total =
          horizontal->Integral(1, horizontal->GetNbinsX(), truth, truth);
      if (total <= 0.0)
        continue;
      for (int tag = 1; tag <= horizontal->GetNbinsX(); ++tag)
        horizontal->SetBinContent(
            tag, truth, horizontal->GetBinContent(tag, truth) / total);
    }
    drawMigration(
        *horizontal,
        "Horizontal-normalized migration, " + diagnostic.label,
        "P(tag | truth)",
        outputDirectory +
            "/private_feeddown_tag_migration_horizontal_normalized_" +
            diagnostic.key + ".pdf");

    auto vertical = std::unique_ptr<TH2D>(static_cast<TH2D *>(
        diagnostic.migration->Clone(("feeddown_vertical_" + diagnostic.key)
                                        .c_str())));
    vertical->SetDirectory(nullptr);
    for (int tag = 1; tag <= vertical->GetNbinsX(); ++tag) {
      const double total = vertical->Integral(tag, tag, 2, 5);
      if (total <= 0.0)
        continue;
      for (int truth = 2; truth <= 5; ++truth)
        vertical->SetBinContent(
            tag, truth, vertical->GetBinContent(tag, truth) / total);
    }
    drawMigration(
        *vertical,
        "Vertical-normalized migration, " + diagnostic.label,
        "P(truth | tag)",
        outputDirectory +
            "/private_feeddown_tag_migration_vertical_normalized_" +
            diagnostic.key + ".pdf");
  }

  TH2D summary("private_feeddown_tag_metric_summary", "",
               static_cast<int>(available.size()), 0.5,
               static_cast<double>(available.size()) + 0.5, 6, 0.5, 6.5);
  const std::array<const char *, 6> metricLabels = {
      "#chi_{c} efficiency", "#chi_{c} purity", "#psi(2S) efficiency",
      "#psi(2S) purity", "Direct untagged efficiency", "Accuracy"};
  for (int metric = 0; metric < 6; ++metric)
    summary.GetYaxis()->SetBinLabel(metric + 1, metricLabels.at(metric));
  for (std::size_t sample = 0; sample < available.size(); ++sample) {
    summary.GetXaxis()->SetBinLabel(static_cast<int>(sample) + 1,
                                    available[sample].label.c_str());
    for (int metric = 0; metric < 6; ++metric)
      summary.SetBinContent(static_cast<int>(sample) + 1, metric + 1,
                            available[sample].metrics.at(metric));
  }
  summary.SetMinimum(0.0);
  summary.SetMaximum(1.0);
  summary.SetMarkerSize(1.5);
  summary.GetXaxis()->SetTitle("Private sample");
  summary.GetYaxis()->SetTitle("");
  summary.GetXaxis()->SetTitleOffset(1.05);
  summary.GetXaxis()->SetTitleSize(0.046);
  summary.GetXaxis()->SetLabelSize(0.040);
  summary.GetXaxis()->CenterTitle();
  summary.GetYaxis()->SetLabelSize(0.038);
  TCanvas summaryCanvas("feeddown_metric_canvas", "", 1200, 850);
  summaryCanvas.SetLeftMargin(0.25);
  summaryCanvas.SetRightMargin(0.15);
  summaryCanvas.SetBottomMargin(0.15);
  summaryCanvas.SetTopMargin(0.13);
  summary.Draw("COLZ TEXT");
  drawNdcTitle("Private feed-down tag performance", 0.25, 0.965);
  summaryCanvas.SaveAs(
      (outputDirectory + "/private_feeddown_tag_metrics.pdf").c_str());
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
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetLegendFont(42);
    gStyle->SetTitleFontSize(0.045);

    if (!options.drawCms && !options.drawPrivate)
      throw std::invalid_argument(
          "at least one of --draw-cms/--draw-private must be on");

    if (options.drawCms) {
      const std::string cms = options.cmsDirectory;
      const std::vector<Sample> cmsSamples = {
        {"Data 2024G",
         {{path(cms, "ParkingDoubleMuonLowMass0_Run2024G-MINIv6NANOv15.root"),
           "CmsReco"}}},
        {"Soft QCD",
         {{path(cms, "JPsiMuMu_Fil-JPsiNo-2MuPtEta_TuneCP5_13p6TeV_pythia8-"
                     "evtgen-RunIIISummer24.root"),
           "CmsReco"}}},
        {"Hard QCD, onia off + CR0",
         {{path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-"
                     "RunIIISummer24_Private.root"),
           "CmsReco"},
          {path(cms, "QCD_Bin-PT-15to7000_Par-PT-flat2022_Par-JpsiMuMu_pythia8-"
                     "RunIIISummer24_ext1_Private.root"),
           "CmsReco"}}},
        {"Hard QCD, onia on + CR1",
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
      drawObservableFamily(cmsSamples, "z_pt", "cms_reco_z_samples",
                           "CMS reco J/#psi-in-jet z",
                           "z = p_{T}(J/#psi)/p_{T}(jet)",
                           options.outputDirectory);
      drawObservableFamily(cmsSamples, "z_h", "cms_reco_zh_samples",
                           "CMS reco J/#psi-in-jet z_{h}",
                           "z_{h} = p^{+}_{J/#psi}/p^{+}_{jet}",
                           options.outputDirectory);
    }

    if (options.drawPrivate) {
      const std::string privateDir = options.privateDirectory;
    const std::string oniaOff8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaoff_CR0_PYTHIA8311.root");
    const std::string oniaOffCr1_8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaoff_CR1_PYTHIA8311.root");
    const std::string oniaOnCr0_8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaon_CR0_PYTHIA8311.root");
    const std::string oniaOn8311 =
        path(privateDir, "HardQCD_Pt15to7000_Oniaon_CR1_PYTHIA8311.root");
    const std::vector<Sample> privateSamples = {
        {"Onia off CR0", {{oniaOff8311, "PrivateGen"}}},
        {"Onia off CR1", {{oniaOffCr1_8311, "PrivateGen"}}},
        {"Onia on CR0", {{oniaOnCr0_8311, "PrivateGen"}}},
        {"Onia on CR1", {{oniaOn8311, "PrivateGen"}}}};
    drawFamily(privateSamples, "private_samples", "Private generation all jets",
               options.outputDirectory, options.drawPositiveHalf);
    drawObservableFamily(privateSamples, "z_pt", "private_z_samples",
                         "Private generation J/#psi-in-jet z",
                         "z = p_{T}(J/#psi)/p_{T}(jet)",
                         options.outputDirectory);
    drawObservableFamily(privateSamples, "z_h", "private_zh_samples",
                         "Private generation J/#psi-in-jet z_{h}",
                         "z_{h} = p^{+}_{J/#psi}/p^{+}_{jet}",
                         options.outputDirectory);
    drawCategoryFractionFamily(
        privateSamples,
        {"Off CR0", "Off CR1", "On CR0", "On CR1"},
        options.outputDirectory);
    drawCharmoniumFractionFamily(
        privateSamples,
        {"Off CR0", "Off CR1", "On CR0", "On CR1"},
        options.outputDirectory);
    drawPrivateFeeddownTagDiagnostics(
        privateSamples,
        {"Off CR0", "Off CR1", "On CR0", "On CR1"},
        options.outputDirectory);

    drawFamily(privateCategorySamples(oniaOff8311),
               "private_oniaoff_cr0_categories",
               "Private OniaOff CR0 categories, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateCategorySamples(oniaOffCr1_8311),
               "private_oniaoff_cr1_categories",
               "Private OniaOff CR1 categories, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateCategorySamples(oniaOnCr0_8311),
               "private_oniaon_cr0_categories",
               "Private OniaOn CR0 categories, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateCategorySamples(oniaOn8311),
               "private_oniaon_cr1_categories",
               "Private OniaOn CR1 categories, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);

    drawFamily(privateSamplesForCategory(privateSamples, {"PrivateGen_cat1"}),
               "private_b_hadron_samples",
               "Private b-hadron category across samples, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateSamplesForCategory(privateSamples,
                                         charmoniumComponentDirectories()),
               "private_charmonium_feeddown_samples",
               "Private charmonium feed-down category across samples, all "
               "jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateSamplesForCategory(privateSamples, {"PrivateGen_cat3"}),
               "private_gluon_proton_samples",
               "Private gluon/proton category across samples, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateSamplesForCategory(privateSamples, {"PrivateGen_cat4"}),
               "private_quark_samples",
               "Private quark category across samples, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    drawFamily(privateSamplesForCategory(
                   privateSamples,
                   {"PrivateGen_cat5", "PrivateGen_cat6", "PrivateGen_cat7"}),
               "private_octet_samples",
               "Private octet (^{3}S_{1}^{[8]}+^{1}S_{0}^{[8]}+^{3}P_{J}^{[8]}) "
               "across samples, all jets",
               options.outputDirectory, options.drawPositiveHalf, true);
    }

    return 0;
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    usage(argv[0]);
    return 1;
  }
}
