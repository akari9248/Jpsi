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
#include "plotratio_beta.h"
#include "/storage/shuangyuan/cmsstyle/src/cmsstyle.H"
using namespace cmsstyle;
void normalizeHistograms(std::vector<TH1D *> &hists)
{
  double totalSum = 0.0;
  std::vector<double> histSums(hists.size(), 0.0);
  for (size_t i = 0; i < hists.size(); i++)
  {
    histSums[i] = hists[i]->Integral();
    totalSum += histSums[i];
  }

  if (totalSum > 0)
  {
    for (size_t i = 0; i < hists.size(); i++)
    {
      double proportion = histSums[i] / totalSum;
      hists[i]->Scale(proportion / histSums[i]);
    }
  }
};


class PlotStack
{
public:
  PlotStack(std::vector<TH1D *> input_stack, std::vector<TH1D *> input, bool normalize = true) : hists_stack(input_stack), hists(input)
  {
    setCMSStyle(true);

    std::random_device rd;
    std::uniform_int_distribution<int> dist(1, 100000);
    int random_num = dist(rd);
    // 归一化
    if (normalize)
    {
      normalizeHistograms(hists_stack);
      for (auto &hist : hists)
      {
        double totalSum = hist->Integral();
        hist->Scale(1.0 / totalSum);
      }
    }

    TH1D *nomimal = (TH1D *)hists.at(0)->Clone();
    for (auto &hist : hists)
    {
      TH1D *temp = (TH1D *)hist->Clone();
      temp->Divide(nomimal);
      // std::cout << temp->GetBinContent(1) << " ";
      histsratio.push_back(temp);
    }
    std::vector<double> relErrors;
    for (int i = 1; i <= hists.at(0)->GetNbinsX(); ++i)
    {
      double A = hists.at(0)->GetBinContent(i);
      double deltaA = hists.at(0)->GetBinError(i);
      if (A != 0)
      {
        relErrors.push_back(deltaA / A); // 保存 ΔA/A
      }
      else
      {
        relErrors.push_back(0); // 处理 A=0 的情况
      }
    }

    // 将相对误差设置为新的绝对误差
    for (int i = 1; i <= histsratio.at(0)->GetNbinsX(); ++i)
    {
      double newValue = histsratio.at(0)->GetBinContent(i); // 此时 newValue = 1
      double newError = relErrors[i - 1] * newValue;        // 绝对误差 = 相对误差 * 新值
      histsratio.at(0)->SetBinError(i, newError);
    }
    // std::cout << std::endl;

    std::pair<double, double> minmax = Plots::Histdispose::findGlobalMinMax(hists);
    std::pair<double, double> minmaxratio = Plots::Histdispose::findGlobalMinMax(histsratio);
    std::cout << minmaxratio.first << " " << minmaxratio.second << std::endl;

    // 设置颜色等
    for (int i = 0; i < hists_stack.size(); ++i)
    {
      hists_stack.at(i)->SetLineWidth(1);
      hists_stack.at(i)->SetFillColorAlpha(colors_stack[i], 1.0);
      // hists_stack.at(i)->GetYaxis()->SetRangeUser(0, 0.3);
    }
    for (int i = 0; i < hists.size(); i++)
    {
      hists.at(i)->SetFillStyle(0);
      hists.at(i)->SetLineColor(colors.at(i));
      hists.at(i)->SetLineWidth(2);
      hists.at(i)->SetMarkerStyle(20);
      hists.at(i)->SetMarkerColor(colors.at(i));
      hists.at(i)->SetMarkerStyle(0);
      hists.at(i)->SetMarkerSize(0);
      hists.at(i)->GetYaxis()->SetLabelSize(0.12);
    }
    double range = minmaxratio.second - minmax.first;
    for (int i = 0; i < histsratio.size(); i++)
    {
      histsratio.at(i)->SetFillStyle(0);
      histsratio.at(i)->SetTitle("");
      histsratio.at(i)->SetLineColor(colors.at(i));
      histsratio.at(i)->SetLineWidth(2);
      histsratio.at(i)->SetMarkerStyle(20);
      histsratio.at(i)->SetMarkerColor(colors.at(i));
      histsratio.at(i)->GetXaxis()->SetLabelSize(0.14);
      histsratio.at(i)->GetXaxis()->SetTitleSize(0.13);
      histsratio.at(i)->GetXaxis()->SetTitleOffset(0.3);
      // histsratio.at(i)->GetXaxis()->SetTitle("#varphi");
      histsratio.at(i)->GetYaxis()->SetTitle("#frac{MC}{Data}");
      histsratio.at(i)->GetYaxis()->SetLabelSize(0.14);
      histsratio.at(i)->GetYaxis()->SetTitleSize(0.15);
      histsratio.at(i)->GetYaxis()->SetTitleOffset(0.45);
      histsratio.at(i)->GetYaxis()->CenterTitle();
      histsratio.at(i)->GetYaxis()->SetNdivisions(505);
      histsratio.at(i)->SetMarkerStyle(0);
      histsratio.at(i)->SetMarkerSize(0);
      // histsratio.at(i)->GetYaxis()->SetRangeUser(0.9, 1.1);
      histsratio.at(i)->GetYaxis()->SetRangeUser(minmaxratio.first - range * 0.01, minmaxratio.second + range * 0.01);
    }

    // stack
    stack = new THStack();
    for (auto &hist : hists_stack)
    {
      stack->Add(hist);
    }
    stack->SetMaximum(minmax.second * 1.8);

    cc = new TCanvas(TString::Format("cc%d", random_num), "canvas", 650, 630);
    // cc = cmsstyle::cmsCanvas("", 0, 1, 0, 1, "", "", false, 0.01, 11);
    // SetCmsText("CMS");
    // SetExtraText("");
    gStyle->SetOptStat(0);
    topPad = new TPad("topPad", "Top Pad", 0, 0.25, 1, 1);          // 上半部分
    bottomPad = new TPad("bottomPad", "Bottom Pad", 0, 0, 1, 0.25); // 下半部分
    topPad->SetTopMargin(0.08);
    topPad->SetRightMargin(0.02);
    topPad->SetBottomMargin(0.01);
    topPad->SetLeftMargin(0.15);
    bottomPad->SetTopMargin(0.05);
    bottomPad->SetRightMargin(0.02);
    bottomPad->SetBottomMargin(0.32);
    bottomPad->SetLeftMargin(0.15);
    topPad->Draw();
    bottomPad->Draw();
  }

  void SetLegPosition(std::vector<std::vector<double>> positions_ = {{0.15, 0.65, 0.3, 0.85}, {0.75, 0.65, 0.9, 0.85}})
  {
    positions = positions_;
  }

  void AddLegend(std::vector<std::vector<TString>> legends, double size = 0.04)
  {
    topPad->cd();
    legend_stack = cmsLeg(positions.at(0).at(0), positions.at(0).at(1), positions.at(0).at(2), positions.at(0).at(3), size);
    for (int i = 0; i < legends.at(0).size(); i++)
    {
      legend_stack->AddEntry(hists_stack.at(i), legends.at(0).at(i), "f");
    }
    // legend_stack->SetBorderSize(0);
    // legend_stack->SetFillColorAlpha(0, 0);
    // legend_stack->SetTextSize(0.04);
    // legend_stack->SetTextFont(42);
    // legend_stack->Draw("same");

    legend = cmsLeg(positions.at(1).at(0), positions.at(1).at(1), positions.at(1).at(2), positions.at(1).at(3), size);
    for (int i = 0; i < legends.at(1).size(); i++)
    {
      legend->AddEntry(hists.at(i), legends.at(1).at(i), "lp");
    }
    // legend->SetBorderSize(0);
    // legend->SetFillColorAlpha(0, 0);
    // legend->SetTextSize(0.04);
    // legend->Draw("same");
  }

  void AddText(std::vector<TString> texts, double ypos = 0.0, double size = 0.04)
  {
    bottomPad->cd();
    for (int i = 0; i < histsratio.size(); i++)
    {
      histsratio.at(i)->GetXaxis()->SetLabelSize(0);
    }
    int nBins = histsratio.at(0)->GetNbinsX();
    double ymin = histsratio.at(0)->GetMinimum();
    double ymax = histsratio.at(0)->GetMaximum();
    // std::vector<TString> texts = {"unmatched", "gg", "qg", "qq"};
    for (int i = 1; i <= nBins; i++)
    {
      double binCenter = histsratio.at(0)->GetBinCenter(i);
      double binContent = histsratio.at(0)->GetBinContent(i);
      TText *text = new TText(binCenter, ypos, texts.at(i - 1));
      text->SetTextAlign(22); // Center the text
      text->SetTextSize(size);
      text->SetTextFont(42);
      text->Draw("same");
    }
  }
  void SetPavePosition(std::vector<double> pos)
  {
    pave_position = pos;
  }
  void SetPavePosition(double pos1, double pos2, double pos3, double pos4)
  {
    pave_position = {pos1, pos2, pos3, pos4};
  }

  void AddPave(std::vector<TString> paves, double size = 0.04, int font = 42)
  {
    cc->cd();

    TPaveText *paveText = new TPaveText(pave_position.at(0), pave_position.at(1), pave_position.at(2), pave_position.at(3), "NDC");
    for (int i = 0; i < paves.size(); i++)
      paveText->AddText(paves.at(i));
    paveText->SetBorderSize(0);
    paveText->SetFillColor(0);
    paveText->SetFillColorAlpha(0, 0);
    paveText->SetTextFont(font);
    paveText->SetTextSize(size);
    paveText->SetTextAlign(12);
    paveText->Draw("same");

    // cc->RedrawAxis();
    // cc->Update();
  }

  void SetYRange(double min, double max)
  {
    stack->SetMinimum(min); // 设置 Y 轴的最小值
    stack->SetMaximum(max);
  }

  void SetYRatioRange(double min, double max)
  {
    for (int i = 0; i < histsratio.size(); i++)
    {
      histsratio.at(i)->GetYaxis()->SetRangeUser(min, max);
    }
  }

  void SetTitle(TString name)
  {
    stack->SetTitle(name);
  }

  // void SetYTitle(TString name)
  // {
  //   stack->GetYaxis()->SetTitle(name);
  // }

  void SetYTitle(TString name, double size = 0.055, double offset = 1.2)
  {
    stack->GetYaxis()->SetTitle(name);
    stack->GetYaxis()->SetTitleSize(size);
    stack->GetYaxis()->SetTitleOffset(offset);
  }

  void SetXRatioTitle(TString name, double size = 0.2, double offset = 0.7)
  {
    for (int i = 0; i < histsratio.size(); i++)
    {
      histsratio.at(i)->GetXaxis()->SetTitleSize(size);
      histsratio.at(i)->GetXaxis()->SetTitleOffset(offset);
      histsratio.at(i)->GetXaxis()->SetTitle(name);
    }
  }

  void SetLineStyle(std::vector<int> linestyle)
  {
    for (int i = 0; i < hists.size(); i++)
    {
      hists.at(i)->SetLineStyle(linestyle.at(i));
      histsratio.at(i)->SetLineStyle(linestyle.at(i));
    }
  }

  void SetColor(std::vector<int> color)
  {
    for (int i = 0; i < hists.size(); i++)
    {
      hists.at(i)->SetLineColor(color.at(i));
      histsratio.at(i)->SetLineColor(color.at(i));
      hists.at(i)->SetMarkerColor(color.at(i));
      histsratio.at(i)->SetMarkerColor(color.at(i));
    }
  }

  void Draw(bool ishist = false)
  {
    topPad->cd();
    setCMSStyle(true);
    stack->Draw("hist");
    // stack->GetYaxis()->SetTitle("#frac{1}{N}#frac{d#sigma}{d#varphi}");
    if (ishist)
    {
      hists.at(0)->SetStats(0);
      hists.at(0)->Draw("same E1");
      for (int i = 1; i < hists.size(); i++)
      {
        hists.at(i)->Draw("same hist E1");
      }
    }
    else
    {
      for (int i = 0; i < hists.size(); i++)
      {
        hists.at(i)->Draw("same hist E1");
      }
    }
    stack->GetXaxis()->SetLabelSize(0);
    // SetExtraText("Private work (CMS data/simulation)");
    // CMS_lumi(topPad, 0, 1.2);
    SetLumi(34.7, "fb", "Run 3");
    SetEnergy(13.6, "TeV");
    bottomPad->cd();
    setCMSStyle(true);
    bottomPad->SetTitle("");
    for (auto &hist : histsratio)
    {
      hist->Draw("same hist E1");
    }
  }

  void Write(TString filename = "output.pdf")
  {
    cc->SaveAs(filename);
  }

private:
  std::vector<TH1D *> hists_stack, hists, histsratio;
  TCanvas *cc;
  TPad *topPad, *bottomPad;
  THStack *stack;
  // "#3f90da", "#ffa90e", "#bd1f01", "#94a4a2", "#832db6", "#a96b59", "#e76300", "#b9ac70", "#717581", "#92dadd"
  std::vector<int> colors_stack = {TColor::GetColor("#3f90da"), TColor::GetColor("#ffa90e"),
                                   TColor::GetColor("#bd1f01"), TColor::GetColor("#94a4a2"),
                                   TColor::GetColor("#832db6"), TColor::GetColor("#a96b59")};
  std::vector<int> colors = {kBlack, kBlue, kRed, kOrange, 226, kViolet, kGreen};
  // std::vector<int> colors = {kOrange, kBlack, kBlue, kRed, 226};
  TLegend *legend_stack, *legend;
  std::vector<std::vector<double>> positions = {{0.15, 0.65, 0.3, 0.85}, {0.75, 0.65, 0.9, 0.85}};
  std::vector<double> pave_position = {0.15, 0.75, 0.26, 0.9};
};