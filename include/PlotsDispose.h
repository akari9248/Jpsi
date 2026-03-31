#pragma once
#include "TCanvas.h"
#include "TColor.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH3.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TLine.h"
#include "TPad.h"
#include "TStyle.h"
#include <iostream>
#include <vector>
#include <TPaveText.h>
#include <TPad.h>
#include <algorithm>
#include <random>
#include "TGaxis.h"
#include <THStack.h>
#include <type_traits>
#include "/storage/shuangyuan/cmsstyle/src/cmsstyle.H"
using namespace cmsstyle;

std::pair<TH1D *, TH1D *> GetEnvelopes(const std::vector<TH1D *> &hists)
{

  // 检查所有直方图结构是否一致
  TH1D *h0 = hists[0];
  const Int_t nBins = h0->GetNbinsX();
  const Double_t xMin = h0->GetXaxis()->GetXmin();
  const Double_t xMax = h0->GetXaxis()->GetXmax();

  // 创建包络线直方图
  TH1D *hUpper = new TH1D("hUpper", "Envelope", nBins, xMin, xMax);
  TH1D *hLower = new TH1D("hLower", "Envelope", nBins, xMin, xMax);
  hUpper->SetDirectory(0); // 脱离ROOT目录手动管理内存
  hLower->SetDirectory(0);

  // 计算每个bin的极值
  for (Int_t iBin = 1; iBin <= nBins; ++iBin)
  {
    Double_t maxVal = -std::numeric_limits<Double_t>::infinity();
    Double_t minVal = std::numeric_limits<Double_t>::infinity();

    for (const auto &h : hists)
    {
      const Double_t content = h->GetBinContent(iBin);
      if (content > maxVal)
        maxVal = content;
      if (content < minVal)
        minVal = content;
    }

    hUpper->SetBinContent(iBin, maxVal);
    hLower->SetBinContent(iBin, minVal);
  }

  return {hUpper, hLower};
}

// 将 vector 中所有 TH1D 的 bin 误差设为 0
void ZeroHistErrors(std::vector<TH1D *> &hists)
{
  for (auto &h : hists)
  {
    const Int_t totalBins = h->GetNbinsX() + 2;
    for (Int_t bin = 0; bin < totalBins; ++bin)
    {
      h->SetBinError(bin, 0.0);
    }
  }
}

void normalizeHistograms(std::vector<TH1D *> &hists, double value = 1)
{
  double totalSum = 0.0;
  std::vector<double> histSums(hists.size(), 0.0);
  for (size_t i = 0; i < hists.size(); i++)
  {
    histSums[i] = hists[i]->Integral(0, hists[i]->GetNbinsX() + 1);
    totalSum += histSums[i];
  }

  if (totalSum > 0)
  {
    for (size_t i = 0; i < hists.size(); i++)
    {
      double proportion = histSums[i] / totalSum * value;
      hists[i]->Scale(proportion / histSums[i]);
    }
  }
};

void normalize(std::vector<TH1D *> graphs, bool isbinwidth = false, std::vector<double> normal = {})
{
  for (int i = 0; i < graphs.size(); i++)
  {
    double sum = 0;
    int bins = graphs.at(i)->GetXaxis()->GetNbins();
    double binrange = graphs.at(i)->GetXaxis()->GetBinUpEdge(bins);
    for (int j = 1; j <= graphs.at(i)->GetNbinsX(); j++)
    {
      sum = sum + graphs.at(i)->GetBinContent(j);
    }

    if (normal.size() != 0)
      sum = sum * 1.0 / normal.at(i);
    for (int k = 0; k <= graphs.at(i)->GetNbinsX() + 1; k++)
    {
      if (sum != 0)
      {
        double oldbincontent = graphs.at(i)->GetBinContent(k);
        double oldbinerror = graphs.at(i)->GetBinError(k);
        if (isbinwidth)
        {
          graphs.at(i)->SetBinContent(k, oldbincontent * 1.0 / sum / graphs.at(i)->GetXaxis()->GetBinWidth(k));
          graphs.at(i)->SetBinError(k, oldbinerror * 1.0 / sum / graphs.at(i)->GetXaxis()->GetBinWidth(k));
        }
        else
        {
          graphs.at(i)->SetBinContent(k, oldbincontent * 1.0 / sum);
          graphs.at(i)->SetBinError(k, oldbinerror * 1.0 / sum);
        } // graphs.at(i)->SetBinError(k, 0.0);
      }
    }
  }
}

void scale(std::vector<TH1D *> graphs, int eventsnum = 20000)
{
  for (int i = 0; i < graphs.size(); i++)
  {
    double integral = graphs.at(i)->Integral();
    double scale_factor = eventsnum * 1.0 / integral;
    graphs.at(i)->Scale(scale_factor);
  }
}

void scale(std::vector<TH1D *> graphs, TH1D *nominal)
{
  double eventsnum = nominal->Integral();
  for (int i = 0; i < graphs.size(); i++)
  {
    double integral = graphs.at(i)->Integral();
    double scale_factor = eventsnum * 1.0 / integral;
    graphs.at(i)->Scale(scale_factor);
  }
}

void scaleall(std::vector<TH1D *> graphs, int eventsnum = 20000)
{
  double sum = 0.0;
  for (int i = 0; i < graphs.size(); i++)
  {
    double integral = graphs.at(i)->Integral();
    sum = sum + integral;
  }
  for (int i = 0; i < graphs.size(); i++)
  {
    double scale_factor = eventsnum * 1.0 / sum;
    graphs.at(i)->Scale(scale_factor);
  }
}

void scaleall(std::vector<TH1D *> graphs, TH1D *nominal)
{
  double sum = 0.0;
  for (int i = 0; i < graphs.size(); i++)
  {
    double integral = graphs.at(i)->Integral();
    sum = sum + integral;
  }
  double eventsnum = nominal->Integral();
  for (int i = 0; i < graphs.size(); i++)
  {
    double scale_factor = eventsnum * 1.0 / sum;
    graphs.at(i)->Scale(scale_factor);
  }
}

std::pair<double, double> findMinMaxBinContent(TH1D *hist)
{
  int nBins = hist->GetNbinsX();
  double minVal = hist->GetBinContent(1);
  double maxVal = hist->GetBinContent(1);
  for (int i = 2; i <= nBins; i++)
  {
    double binContent = hist->GetBinContent(i);
    if (binContent == 0)
      continue;
    if (binContent < minVal)
      minVal = binContent;
    if (binContent > maxVal)
      maxVal = binContent;
  }
  return std::make_pair(minVal, maxVal);
}

std::pair<double, double> findGlobalMinMax(std::vector<TH1D *> hists)
{
  double globalMin = std::numeric_limits<double>::max();
  double globalMax = std::numeric_limits<double>::lowest();
  for (TH1D *hist : hists)
  {
    std::pair<double, double> localMinMax = findMinMaxBinContent(hist);
    if (localMinMax.first < globalMin)
      globalMin = localMinMax.first;
    if (localMinMax.second > globalMax)
      globalMax = localMinMax.second;
  }
  return std::make_pair(globalMin, globalMax);
}

std::pair<double, double> findhistrange(std::vector<TH1D *> hists, double scale)
{
  std::pair<double, double> minmax = findGlobalMinMax(hists);
  double range = minmax.second - minmax.first;
  double max = minmax.second + range * scale;
  double min = minmax.first - range * scale;
  return std::make_pair(min, max);
}

std::pair<double, double> findhistrange(std::vector<TH1D *> hists, double scaleup, double scaledn)
{
  std::pair<double, double> minmax = findGlobalMinMax(hists);
  double range = minmax.second - minmax.first;
  double max = minmax.second + range * scaleup;
  double min = minmax.first - range * scaledn;
  return std::make_pair(min, max);
}

void rebin(std::vector<TH1D *> hists, int rebinnum = 2)
{
  for (auto &hist : hists)
  {
    hist->Rebin(rebinnum);
  }
}

// void symmetrize(std::vector<TH1D *> &graph_1d)
// {
//   for (int i = 0; i < graph_1d.size(); i++)
//   {
//     int binnum = graph_1d.at(i)->GetNbinsX();
//     double binlowedge = graph_1d.at(i)->GetXaxis()->GetBinLowEdge(1);
//     double binupedge = graph_1d.at(i)->GetXaxis()->GetBinUpEdge(binnum);
//     TH1D *graph = new TH1D(TString::Format("graph%d", rand()), "graph", binnum, binlowedge, binupedge);
//     for (int j = 1; j <= graph->GetNbinsX() / 2; j++)
//     {
//       graph->SetBinContent(j, (graph_1d.at(i)->GetBinContent(j) + graph_1d.at(i)->GetBinContent(binnum + 1 - j)) / 2);
//       graph->SetBinError(j, sqrt((pow(graph_1d.at(i)->GetBinError(j), 2) + pow(graph_1d.at(i)->GetBinError(binnum + 1 - j), 2)) / 2));

//       graph->SetBinContent(binnum + 1 - j, (graph_1d.at(i)->GetBinContent(j) + graph_1d.at(i)->GetBinContent(binnum + 1 - j)) / 2);
//       graph->SetBinError(binnum + 1 - j, sqrt((pow(graph_1d.at(i)->GetBinError(j), 2) + pow(graph_1d.at(i)->GetBinError(binnum + 1 - j), 2)) / 2));
//     }
//     graph_1d.at(i) = graph;
//   }
// }

void symmetrize(std::vector<TH1D *> &histograms)
{
  for (int i = 0; i < histograms.size(); i++)
  {
    TH1D *hist = histograms.at(i);
    int nbins = hist->GetNbinsX();

    // 创建临时容器保存原始值
    std::vector<double> contents(nbins + 1);
    std::vector<double> errors(nbins + 1);

    // 保存原始内容
    for (int j = 1; j <= nbins; j++)
    {
      contents[j] = hist->GetBinContent(j);
      errors[j] = hist->GetBinError(j);
    }

    // 执行对称化操作（仅修改指定位置）
    for (int j = 1; j <= nbins / 2; j++)
    {
      int k = nbins + 1 - j;

      // 计算对称值和误差
      double sym_content = (contents[j] + contents[k]) / 2.0;
      double sym_error = sqrt((errors[j] * errors[j] + errors[k] * errors[k])) / sqrt(2.0);

      // 直接修改直方图内容
      hist->SetBinContent(j, sym_content);
      hist->SetBinError(j, sym_error);

      // 同步修改对称位置
      hist->SetBinContent(k, sym_content);
      hist->SetBinError(k, sym_error);
    }
  }
}

void symmetrize(std::vector<TH1D *> &&graph_1d)
{
  symmetrize(graph_1d);
}

double chi_square_normalize(TH1D *h0, TH1D *h1, TH1D *error_graph = nullptr)
{
  int xbinnum = h0->GetXaxis()->GetNbins();
  double chi_square = 0.0;
  for (int i = 1; i <= xbinnum; i++)
  {
    double error = h0->GetBinError(i);
    if (error_graph != nullptr)
      error = error_graph->GetBinError(i);
    double variance = error * error;
    if (variance == 0)
      continue;
    double minus = h1->GetBinContent(i) - h0->GetBinContent(i);
    double chi_square_temp = minus * minus / variance;
    chi_square = chi_square + chi_square_temp;
  }
  return chi_square;
}

std::vector<double> chi_square(std::vector<TH1D *> graphs, TH1D *error_graph = nullptr)
{
  TH1D *h0 = graphs.at(0);
  std::vector<double> chi_squares;
  for (int i = 1; i < graphs.size(); i++)
  {
    double chi_square_temp = 0.0;
    TH1D *h1 = graphs.at(i);
    chi_square_temp = chi_square_normalize(h0, h1, error_graph);
    chi_squares.push_back(chi_square_temp);
  }
  return chi_squares;
}

static void Normalize(TH2D *hist, char mode)
{
  if (!hist)
  {
    std::cerr << "Invalid histogram pointer" << std::endl;
    return;
  }
  // 归一化X轴或Y轴
  if (mode == 'x' || mode == 'X' || mode == 'y' || mode == 'Y')
  {
    int nbinsX = hist->GetNbinsX();
    int nbinsY = hist->GetNbinsY();
    bool xMode = (mode == 'x' || mode == 'X');
    for (int i = 1; i <= (xMode ? nbinsY : nbinsX); ++i)
    {
      double total = 0;
      for (int j = 1; j <= (xMode ? nbinsX : nbinsY); ++j)
      {
        total += xMode ? hist->GetBinContent(j, i) : hist->GetBinContent(i, j);
      }
      if (total == 0)
        continue;
      for (int j = 1; j <= (xMode ? nbinsX : nbinsY); ++j)
      {
        double oldValue = xMode ? hist->GetBinContent(j, i) : hist->GetBinContent(i, j);
        hist->SetBinContent(xMode ? j : i, xMode ? i : j, oldValue / total);
        hist->SetBinError(xMode ? j : i, xMode ? i : j, hist->GetBinError(xMode ? j : i, xMode ? i : j) / total);
      }
    }
  }
  // 归一化整体
  else if (mode == 't' || mode == 'T')
  {
    double total = 0;
    for (int i = 0; i <= hist->GetNbinsX() + 1; ++i)
    {
      for (int j = 0; j <= hist->GetNbinsY() + 1; ++j)
      {
        total += hist->GetBinContent(i, j);
        // std::cout << hist->GetBinContent(i, j) << std::endl;
      }
    }
    // if (total == 0.0)
    // {
    //   std::cerr << "The total sum of bins is zero" << std::endl;
    //   return;
    // }
    for (int i = 0; i <= hist->GetNbinsX() + 1; ++i)
    {
      for (int j = 0; j <= hist->GetNbinsY() + 1; ++j)
      {
        double oldValue = hist->GetBinContent(i, j);
        hist->SetBinContent(i, j, oldValue / total);
        hist->SetBinError(i, j, hist->GetBinError(i, j) / total);
      }
    }
  }
  else
  {
    std::cerr << "Invalid mode. Use 'x', 'y' or 't'." << std::endl;
  }
}