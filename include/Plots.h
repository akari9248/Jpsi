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
#include "TGraph.h"
#include <TGraphAsymmErrors.h>
#include <THStack.h>
#include <type_traits>
// #include "plotratio_beta.h"
#include "PlotsDispose.h"
#include "/storage/shuangyuan/cmsstyle/src/cmsstyle.H"
#include "/storage/shuangyuan/cmsstyle/src/cmsstyle.C"
using namespace cmsstyle;

class Plots
{
private:
  std::vector<TH1D *> hists, hists_stack, hists_ratio;
  TH1D *empty_hist1, *empty_hist2;
  TCanvas *cc;
  TPad *topPad, *bottomPad;
  THStack *stack;
  // "#3f90da", "#ffa90e", "#bd1f01", "#94a4a2", "#832db6", "#a96b59", "#e76300", "#b9ac70", "#717581", "#92dadd"
  std::vector<int> colors_stack = {TColor::GetColor("#3f90da"), TColor::GetColor("#ffa90e"),
                                   TColor::GetColor("#bd1f01"), TColor::GetColor("#94a4a2"),
                                   TColor::GetColor("#832db6"), TColor::GetColor("#a96b59"),
                                   TColor::GetColor("#e76300"), TColor::GetColor("#b9ac70"),
                                   TColor::GetColor("#717581"), TColor::GetColor("#92dadd")};
  std::vector<int> colors = {kBlack, kBlue, kRed, kOrange, 226, kViolet, kGreen, TColor::GetColor("#b9ac70"), kBlue, kRed, kOrange, 226, kViolet, kGreen};
  // std::vector<int> colors = {kOrange, kBlack, kBlue, kRed, 226};
  TLegend *legend_stack, *legend;
  std::vector<std::vector<double>> positions = {{0.15, 0.65, 0.3, 0.85}, {0.75, 0.65, 0.9, 0.85}};
  std::vector<double> pave_position = {0.15, 0.75, 0.26, 0.9};
  std::vector<double> leg_position = {0.66, 0.56, 0.9, 0.87};
  std::vector<double> leg_stack_position = {0.18, 0.56, 0.42, 0.87};
  std::vector<int> linestyle = {2, 2, 2, 2, 2};
  std::vector<int> marker = {22, 22, 22, 22, 22};
  bool havestack = false;
  bool haveratio = false;
  bool havedata = false;
  bool drawratio = false;
  int leg_column = 1;
  int leg_stack_column = 1;

  void CreateCanvas()
  {
    setCMSStyle(true);
    std::random_device rd;
    std::uniform_int_distribution<int> dist(1, 100000);
    cc = new TCanvas(TString::Format("cc%d", dist(rd)), "canvas", 680, 650);
    cc->SetRightMargin(0.05);
    gStyle->SetOptStat(0);
  }

  void SetupPads()
  {
    topPad = new TPad("topPad", "Top Pad", 0, 0.25, 1, 1);
    bottomPad = new TPad("bottomPad", "Bottom Pad", 0, 0, 1, 0.25);
    topPad->SetTopMargin(0.08);
    topPad->SetRightMargin(0.03);
    topPad->SetBottomMargin(0.02);
    topPad->SetLeftMargin(0.2);
    bottomPad->SetTopMargin(0.05);
    bottomPad->SetRightMargin(0.03);
    bottomPad->SetBottomMargin(0.4);
    bottomPad->SetLeftMargin(0.2);
  }

  void InitRatioHists(int ratioindex = 0)
  {
    TH1D *nominal = (TH1D *)hists.at(ratioindex)->Clone();
    for (auto &hist : hists)
    {
      TH1D *temp = (TH1D *)hist->Clone();
      std::vector<double> relErrors;
      for (int i = 1; i <= hist->GetNbinsX(); ++i)
      {
        double content = hist->GetBinContent(i);
        double error = hist->GetBinError(i);
        relErrors.push_back((content != 0) ? error / content : 0);
      }

      temp->Divide(nominal);

      for (int i = 1; i <= temp->GetNbinsX(); ++i)
      {
        double ratioContent = temp->GetBinContent(i);
        double newError = ratioContent * relErrors[i - 1];
        temp->SetBinError(i, newError);
      }

      hists_ratio.push_back(temp);
    }
    std::pair<double, double> minmax_ratio = findhistrange(hists_ratio, 0.1);
    empty_hist2 = (TH1D *)hists_ratio.at(0)->Clone();
    empty_hist2->Reset();
    // empty_hist2->SetLabelColor(kWhite);
    // empty_hist2->SetLabelSize(0);
    empty_hist2->SetLineColor(kWhite);
    empty_hist2->SetLineWidth(0);
    empty_hist2->GetXaxis()->SetLabelSize(0.14);
    empty_hist2->GetXaxis()->SetTitleSize(0.13);
    empty_hist2->GetXaxis()->SetTitleOffset(0.3);
    empty_hist2->GetYaxis()->SetTitle("#frac{MC}{Data}");
    empty_hist2->GetYaxis()->SetLabelSize(0.14);
    empty_hist2->GetYaxis()->SetTitleSize(0.15);
    empty_hist2->GetYaxis()->SetTitleOffset(0.45);
    empty_hist2->GetYaxis()->CenterTitle();
    empty_hist2->GetYaxis()->SetNdivisions(505);
    empty_hist2->GetXaxis()->SetNdivisions(909);
    empty_hist2->GetYaxis()->SetRangeUser(minmax_ratio.first, minmax_ratio.second);
  }

  void SetHistsStyle(std::vector<TH1D *> temps)
  {
    if (temps.size() > colors.size())
      std::cerr << "not enough color for hist" << std::endl;
    for (int i = 0; i < temps.size(); i++)
    {
      temps.at(i)->SetLineColor(colors.at(i));
      temps.at(i)->SetLineWidth(2);
      temps.at(i)->SetFillStyle(0);
      temps.at(i)->SetMarkerColor(colors.at(i));
      temps.at(i)->SetMarkerSize(0.4);
      temps.at(i)->SetTitle("");
    }
  }

public:
  Plots(std::vector<TH1D *> inputs) : Plots(inputs, {}) {}

  Plots(std::vector<TH1D *> inputs, std::vector<TH1D *> inputs_stacks)
      : hists(inputs), hists_stack(inputs_stacks)
  {
    TH1::SetDefaultSumw2();
    CreateCanvas();
    SetupPads();
    InitRatioHists();
    for (auto &hist : inputs)
    {
      hist->SetMarkerSize(0.4);
      hist->SetLineWidth(2);
    }
    if (inputs.size() > colors.size())
      std::cout << "not enough color for hist, do not initial" << std::endl;
    else
    {
      SetHistsStyle(hists);
      SetHistsStyle(hists_ratio);
    }
    for (auto &hist : hists)
      hist->SetStats(0);
    for (auto &hist : hists_stack)
      hist->SetStats(0);

    std::pair<double, double> minmax = findhistrange(hists, 1, 0.1);
    std::pair<double, double> minmax_stack = findGlobalMinMax(hists);
    if (!hists_stack.empty())
    {
      havestack = true;
      stack = new THStack();
      for (int i = 0; i < hists_stack.size(); ++i)
      {
        hists_stack.at(i)->SetLineWidth(0);
        hists_stack.at(i)->SetFillColorAlpha(colors_stack[i], 1.0);
      }
      for (auto &hist : hists_stack)
      {
        stack->Add(hist);
      }
      stack->SetMaximum(minmax_stack.second * 1.8);
      // stack->GetXaxis()->SetLabelSize(0);
    }

    empty_hist1 = (TH1D *)hists.at(0)->Clone();
    empty_hist1->Reset();
    empty_hist1->GetYaxis()->SetMaxDigits(3);
    empty_hist1->GetYaxis()->SetRangeUser(minmax.first, minmax.second);
    empty_hist1->GetYaxis()->SetLabelSize(0.05);
    empty_hist1->GetXaxis()->SetLabelSize(0.05);
    empty_hist1->GetXaxis()->SetNdivisions(505);
    empty_hist1->SetLineColor(kWhite);
    empty_hist1->SetLineWidth(0);
  }

  void SetRatio(int index = 0)
  {
    haveratio = true;
    hists_ratio.clear();
    InitRatioHists(index);
    empty_hist1->SetLabelColor(kWhite);
    empty_hist1->SetLabelSize(0);
  }

  void DrawRatio(int index = 0)
  {
    drawratio = true;
    hists_ratio.clear();
    InitRatioHists(index);
  }

  void SetData()
  {
    havedata = true;
    cmsstyle::cmsObjectDraw(hists.at(0), "E1", {{"MarkerStyle", kFullCircle}, {"MarkerSize", 0.8}});
    cmsstyle::cmsObjectDraw(hists_ratio.at(0), "E1", {{"MarkerStyle", kFullCircle}, {"MarkerSize", 0.8}});
  }

  void SetAllData()
  {
    havedata = true;
    for (int i = 0; i < hists.size(); i++)
    {
      cmsstyle::cmsObjectDraw(hists.at(i), "E1", {{"MarkerStyle", kFullCircle}, {"MarkerSize", 0.8}});
      cmsstyle::cmsObjectDraw(hists_ratio.at(i), "E1", {{"MarkerStyle", kFullCircle}, {"MarkerSize", 0.8}});
    }
  }

  void SetScale(double scale = 1.8)
  {
    std::pair<double, double> minmax_stack = findGlobalMinMax(hists);
    stack->SetMaximum(minmax_stack.second * scale);
  }

  // for label size
  void SetYLabelSize(double size = 0.05)
  {
    if (havestack)
      stack->GetYaxis()->SetLabelSize(size);
    else
      empty_hist1->GetYaxis()->SetLabelSize(size);
  }
  void SetYRatioLabelSize(double size = 0.05)
  {
    empty_hist2->GetYaxis()->SetLabelSize(size);
  }
  void SetXLabelSize(double size = 0.05)
  {
    empty_hist1->GetXaxis()->SetLabelSize(size);
    if (haveratio || drawratio)
      empty_hist2->GetXaxis()->SetLabelSize(size);
  }

  // for legend
  void SetLegColumn(int histcolumn = 1, int stackcolumn = 1)
  {
    leg_column = histcolumn;
    leg_stack_column = stackcolumn;
  }
  void SetLegPosition(double xlow = 0.66, double ylow = 0.56, double xup = 0.9, double yup = 0.88)
  {
    leg_position = {xlow, ylow, xup, yup};
  }
  void SetLegPosition(std::vector<double> pos, bool isstack = false)
  {
    if (isstack)
      leg_stack_position = pos;
    else
      leg_position = pos;
  }
  void SetLegPosition(std::vector<double> pos, std::vector<double> pos_stack)
  {
    leg_position = pos;
    leg_stack_position = pos_stack;
  }
  void SetLegPosition(std::vector<std::vector<double>> poss)
  {
    leg_position = poss.at(0);
    leg_stack_position = poss.at(1);
  }
  void AddLegend(std::vector<TString> legends, double size = 0.07, bool istack = false)
  {
    cc->cd();
    if (haveratio)
      topPad->cd();
    legend = cmsLeg(leg_position.at(0), leg_position.at(1), leg_position.at(2), leg_position.at(3), size);
    for (int i = 0; i < legends.size(); i++)
    {
      if (istack)
        legend->AddEntry(hists_stack.at(i), legends.at(i), "f");
      else
      {
        if (havedata && i == 0)
          legend->AddEntry(hists.at(0), legends.at(i), "lpe");
        else
          legend->AddEntry(hists.at(i), legends.at(i), "l");
      }
    }
    legend->SetNColumns(leg_column);
    legend->Draw("same");
  }
  void AddLegend(std::initializer_list<TString> legends, double size = 0.07, bool istack = false)
  {
    AddLegend(std::vector<TString>(legends), size, istack);
  }
  void AddLegend(std::vector<std::vector<TString>> legends, double size = 0.07, bool istack = false)
  {
    if (!havestack)
      std::cerr << "no stack hists for legend" << std::endl;
    cc->cd();
    if (haveratio)
      topPad->cd();
    legend = cmsLeg(leg_position.at(0), leg_position.at(1), leg_position.at(2), leg_position.at(3), size);
    legend_stack = cmsLeg(leg_stack_position.at(0), leg_stack_position.at(1), leg_stack_position.at(2), leg_stack_position.at(3), size);
    if (havedata)
    {
      legend->AddEntry(hists.at(0), legends.at(0).at(0), "elp");
      for (int i = 1; i < legends.at(0).size(); i++)
        legend->AddEntry(hists.at(i), legends.at(0).at(i), "l");
    }
    else
    {
      for (int i = 0; i < legends.at(0).size(); i++)
      {
        legend->AddEntry(hists.at(i), legends.at(0).at(i), "lp");
      }
    }
    for (int i = 0; i < legends.at(1).size(); i++)
      legend_stack->AddEntry(hists_stack.at(i), legends.at(1).at(i), "f");
    // legend_stack->SetNColumns(5);
    legend->SetNColumns(leg_column);
    legend_stack->SetNColumns(leg_stack_column);
    legend->Draw("same");
    legend_stack->Draw("same");
  }
  void AddLegend(std::vector<std::vector<TString>> legends, std::vector<std::vector<double>> pos, std::vector<int> column,
                 double size = 0.07, bool istack = false)
  {
    if (!havestack)
      std::cerr << "no stack hists for legend" << std::endl;
    cc->cd();
    if (haveratio)
      topPad->cd();
    int index = 0;
    for (int k = 0; k < legends.size(); k++)
    {
      std::vector<TString> legends_ = legends.at(k);
      std::vector<double> pos_ = pos.at(k);
      TLegend *leg_ = cmsLeg(pos_.at(0), pos_.at(1), pos_.at(2), pos_.at(3), size);
      for (int i = 0; i < legends_.size(); i++)
      {
        if (i % 2 != 0)
          continue;
        leg_->AddEntry(hists.at(index), legends_.at(i), "lp");
        index++;
      }
      leg_->SetNColumns(column.at(k));
      leg_->Draw("same");
    }
  }
  void AddLegend(std::vector<TString> legends1, std::vector<TString> legends2)
  {
    AddLegend({legends1, legends2});
  }

  void AddExtraLegend(double xlow, double ylow, double xup, double yup)
  {
    TH1D *empty_hist_extra1 = (TH1D *)hists.at(0)->Clone();
    TH1D *empty_hist_extra2 = (TH1D *)hists.at(0)->Clone();
    TLegend *leg_ = cmsLeg(xlow, ylow, xup, yup, 0.06);
    empty_hist_extra1->SetLineStyle(1);
    empty_hist_extra2->SetLineStyle(2);
    empty_hist_extra1->SetLineColor(kBlack);
    empty_hist_extra2->SetLineColor(kBlack);

    leg_->AddEntry(empty_hist_extra1, "Correlation on", "l");
    leg_->AddEntry(empty_hist_extra2, "Correlation off", "l");
    leg_->Draw("same");
  }

  // for pave
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
    if (haveratio)
      topPad->cd();
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
  }

  void AddText(std::vector<TString> texts, double ypos = 0.0, double size = 0.04)
  {
    cc->cd();
    if (haveratio)
      bottomPad->cd();
    for (int i = 0; i < hists_ratio.size(); i++)
    {
      hists.at(i)->GetXaxis()->SetLabelSize(0);
      hists_ratio.at(i)->GetXaxis()->SetLabelSize(0);
      empty_hist1->GetXaxis()->SetLabelSize(0);
      empty_hist2->GetXaxis()->SetLabelSize(0);
    }
    if (havestack)
      stack->GetXaxis()->SetLabelSize(0);
    int nBins = hists_ratio.at(0)->GetNbinsX();
    double ymin = hists_ratio.at(0)->GetMinimum();
    double ymax = hists_ratio.at(0)->GetMaximum();
    // std::vector<TString> texts = {"unmatched", "gg", "qg", "qq"};
    for (int i = 1; i <= nBins; i++)
    {
      double binCenter = hists_ratio.at(0)->GetBinCenter(i);
      double binContent = hists_ratio.at(0)->GetBinContent(i);
      TText *text = new TText(binCenter, ypos, texts.at(i - 1));
      text->SetTextAlign(22); // Center the text
      text->SetTextSize(size);
      text->SetTextFont(42);
      text->Draw("same");
    }
  }

  // for range
  void SetYRange(double min, double max)
  {
    if (havestack)
    {
      stack->SetMinimum(min);
      stack->SetMaximum(max);
    }
    else
    {
      empty_hist1->GetYaxis()->SetRangeUser(min, max);
    }
  }
  void SetXRange(double min, double max)
  {
    if (havestack)
      stack->GetHistogram()->GetXaxis()->SetRangeUser(min, max);
    else
      empty_hist1->GetXaxis()->SetRangeUser(min, max);
    empty_hist2->GetXaxis()->SetRangeUser(min, max);
  }
  void SetYRatioRange(double min, double max)
  {
    empty_hist2->GetYaxis()->SetRangeUser(min, max);
  }

  // for log
  void SetLogY(bool withratio = false)
  {
    if (haveratio)
    {
      topPad->SetLogy();
      if (withratio)
        bottomPad->SetLogy();
    }
    else
    {
      cc->SetLogy();
    }
  }
  void SetLogX()
  {
    if (haveratio)
    {
      topPad->SetLogx();
      bottomPad->SetLogx();
    }
    else
    {
      cc->SetLogx();
    }
  }

  // for title
  void SetXTitle(TString name, double size = 0.18, double offset = 0.7)
  {
    empty_hist1->GetXaxis()->SetTitleSize(size);
    empty_hist1->GetXaxis()->SetTitleOffset(offset);
    empty_hist1->GetXaxis()->SetTitle(name);
    if (havestack)
    {
      stack->GetXaxis()->SetTitleOffset(offset);
      stack->GetXaxis()->SetTitleSize(size);
      stack->GetXaxis()->SetTitle(name);
    }
    if (!empty_hist2)
    {
      empty_hist2->GetXaxis()->SetTitleSize(size);
      empty_hist2->GetXaxis()->SetTitleOffset(offset);
      empty_hist2->GetXaxis()->SetTitle(name);
    }
  }
  void SetXRatioTitle(TString name, double size = 0.18, double offset = 0.7)
  {
    empty_hist2->GetXaxis()->SetTitleSize(size);
    empty_hist2->GetXaxis()->SetTitleOffset(offset);
    empty_hist2->GetXaxis()->SetTitle(name);
  }
  void SetYTitle(TString name, double size = 0.055, double offset = 1.2)
  {
    if (havestack)
    {
      stack->GetYaxis()->SetTitle(name);
      stack->GetYaxis()->SetTitleSize(size);
      stack->GetYaxis()->SetTitleOffset(offset);
    }
    else
    {
      empty_hist1->GetYaxis()->SetTitle(name);
      empty_hist1->GetYaxis()->SetTitleSize(size);
      empty_hist1->GetYaxis()->SetTitleOffset(offset);
      // empty_hist1->GetYaxis()->CenterTitle();
    }
  }
  void SetYRatioTitle(TString name, double size = 0.15, double offset = 0.45)
  {
    empty_hist2->GetYaxis()->SetTitle(name);
    empty_hist2->GetYaxis()->SetTitleSize(size);
    empty_hist2->GetYaxis()->SetTitleOffset(offset);
    empty_hist2->GetYaxis()->CenterTitle();
  }

  // for style
  void SetLineStyle(std::vector<int> lines)
  {
    for (int i = 0; i < lines.size(); i++)
    {
      hists.at(i)->SetLineStyle(lines.at(i));
      hists_ratio.at(i)->SetLineStyle(lines.at(i));
    }
  }
  void SetLineColor(std::vector<int> colors_, bool withmarker = true)
  {
    for (int i = 0; i < colors_.size(); i++)
    {
      hists.at(i)->SetLineColor(colors_.at(i));
      hists_ratio.at(i)->SetLineColor(colors_.at(i));
      if (withmarker)
      {
        hists.at(i)->SetMarkerColor(colors_.at(i));
        hists_ratio.at(i)->SetMarkerColor(colors_.at(i));
      }
    }
  }
  void SetMarkerStyle(std::vector<int> styles, double size = 0.1)
  {
    for (int i = 0; i < styles.size(); i++)
    {
      hists.at(i)->SetMarkerStyle(styles.at(i));
      hists_ratio.at(i)->SetMarkerStyle(styles.at(i));
      hists.at(i)->SetMarkerSize(size);
      hists_ratio.at(i)->SetMarkerSize(size);
    }
  }

  // for extra line
  void AddExtraLine(double xlow, double ylow, double xup, double yup, bool isratio = false)
  {
    cc->cd();
    if (haveratio)
    {
      topPad->cd();
      if (isratio)
        bottomPad->cd();
    }
    TLine *line = new TLine(xlow, ylow, xup, yup);
    line->SetLineStyle(2);
    line->SetLineColor(kBlack);
    line->Draw("same");
  }
  void AddExtraLine(std::vector<double> pos, bool isratio = false)
  {
    AddExtraLine(pos.at(0), pos.at(1), pos.at(2), pos.at(3), isratio);
  }

  // for error band
  void AddErrorBand(TGraphAsymmErrors *grSysBand)
  {
    cc->cd();
    if (haveratio)
      bottomPad->cd();
    // int color = TColor::GetColor("#40e34bff");
    int color = kBlack;
    double falpha = 1.0;
    int style = 3013;
    grSysBand->SetFillColorAlpha(color, falpha);
    grSysBand->SetFillStyle(style);
    gStyle->SetHatchesSpacing(1);
    // gStyle->SetHatchesLineWidth(2);
    grSysBand->SetLineColor(0);
    grSysBand->Draw("same E2");

    TH1D *hDummy = new TH1D("", "", 1, 0, 1);
    hDummy->SetFillStyle(style);
    hDummy->SetFillColorAlpha(color, falpha);
    hDummy->SetLineColor(color);
    hDummy->SetLineWidth(0);

    // if (haveratio)
    //   topPad->cd();
    TLegend *leg_error = cmsLeg(0.32, 0.4, 0.47, 0.5, 0.16);
    leg_error->AddEntry(hDummy, "unc.", "f");
    leg_error->Draw("same");
    // delete hDummy;
  }

  // for cms
  void AddLumi(double lumi = 34.7, TString xsec = "fb", TString era = "Run 3",
               TString cmstext = "CMS", TString extratext = "preliminary", int pos = 0)
  {
    SetLumi(lumi, (std::string)xsec, (std::string)era);
    SetEnergy(13.6, "TeV");
    SetCmsText((std::string)cmstext);
    SetExtraText((std::string)extratext);
    CMS_lumi(topPad, pos, 1.2);
  }

  void Draw()
  {
    if (haveratio)
    {
      cc->cd();
      setCMSStyle(true);
      topPad->Draw();
      bottomPad->Draw();
      topPad->cd();
      setCMSStyle(true);
      if (havestack)
        stack->Draw("hist");
      else
        empty_hist1->Draw("hist");
      if (havedata)
        hists.at(0)->Draw("same E0");
      else
        hists.at(0)->Draw("same hist");
      if (hists.size() > 1)
      {
        for (int i = 1; i < hists.size(); i++)
          hists.at(i)->Draw("same hist");
      }

      bottomPad->cd();
      setCMSStyle(true);
      empty_hist2->Draw("same hist");
      if (havedata)
        hists_ratio.at(0)->Draw("same E0");
      else
        hists_ratio.at(0)->Draw("same hist");
      if (hists_ratio.size() > 1)
      {
        for (int i = 1; i < hists_ratio.size(); i++)
          hists_ratio.at(i)->Draw("same hist");
      }
    }
    else
    {
      cc->cd();
      setCMSStyle(true);
      if (havestack)
      {
        stack->Draw("hist");
        std::cout << hists.at(0)->GetBinContent(1) << std::endl;
        if (drawratio)
        {
          for (int i = 0; i < hists_ratio.size(); i++)
            hists_ratio.at(i)->Draw("same hist");
        }
        else
        {
          if (havedata)
            hists.at(0)->Draw("same E0");
          else
            hists.at(0)->Draw("same hist");
          if (hists.size() > 1)
          {
            for (int i = 1; i < hists.size(); i++)
              hists.at(i)->Draw("same hist");
          }
        }
      }
      else
      {
        if (drawratio)
        {
          empty_hist2->Draw();
          for (int i = 0; i < hists_ratio.size(); i++)
            hists_ratio.at(i)->Draw("same hist");
        }
        else
        {
          empty_hist1->Draw();
          if (havedata)
            hists.at(0)->Draw("same E0");
          else
            hists.at(0)->Draw("same hist");
          if (hists.size() > 1)
          {
            for (int i = 1; i < hists.size(); i++)
              hists.at(i)->Draw("same hist");
          }
        }
      }
    }
  }

  void Write(TString name)
  {
    if (!name.Contains(".pdf"))
      name = name + ".pdf";
    cc->SaveAs(name);
  }
};