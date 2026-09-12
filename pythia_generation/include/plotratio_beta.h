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
#include <type_traits>
// #include "/storage/shuangyuan/cmsstyle/src/cmsstyle.H"
// using namespace cmsstyle;
namespace Plots
{
  class Histdispose
  {
  public:
    static void normalize(std::vector<TH1D *> graphs, bool isbinwidth = true, std::vector<double> normal = {})
    {
      for (int i = 0; i < graphs.size(); i++)
      {
        double sum = 0;
        int bins = graphs.at(i)->GetXaxis()->GetNbins();
        double binrange = graphs.at(i)->GetXaxis()->GetBinUpEdge(bins);
        for (int j = 0; j <= graphs.at(i)->GetNbinsX() + 1; j++)
        {
          sum = sum + graphs.at(i)->GetBinContent(j);
        }
        // std::cout << sum << " " << graphs.at(i)->Integral() << std::endl;
        // sum = graphs.at(i)->Integral(1, graphs.at(i)->GetNbinsX());
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

    static void scale(std::vector<TH1D *> graphs, int eventsnum = 20000)
    {
      for (int i = 0; i < graphs.size(); i++)
      {
        double integral = graphs.at(i)->Integral();
        // std::cout << integral << " " << graphs.at(i)->GetEntries() << std::endl;
        double scale_factor = eventsnum * 1.0 / integral;
        graphs.at(i)->Scale(scale_factor);
        // int xnum = graphs.at(i)->GetXaxis()->GetNbins();
        // for (int j = 1; j <= xnum; j++)
        // {
        //   double content = graphs.at(i)->GetBinContent(j);
        //   graphs.at(i)->SetBinError(j, std::sqrt(content));
        // }
      }
    }

    static std::pair<double, double> findGlobalMinMax(std::vector<TH1D *> hists)
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

    static std::pair<double, double> findhistrange(std::vector<TH1D *> hists, double scale = 0.02)
    {
      std::pair<double, double> minmax = Plots::Histdispose::findGlobalMinMax(hists);
      double range = minmax.second - minmax.first;
      double max = minmax.second + range * scale;
      double min = minmax.first - range * scale;
      return std::make_pair(min, max);
    }

    static void rebin(std::vector<TH1D *> hists, int rebinnum = 2)
    {
      for (auto &hist : hists)
      {
        hist->Rebin(rebinnum);
      }
    }

    static void symmetrize(std::vector<TH1D *> &graph_1d)
    {
      for (int i = 0; i < graph_1d.size(); i++)
      {
        int binnum = graph_1d.at(i)->GetNbinsX();
        double binlowedge = graph_1d.at(i)->GetXaxis()->GetBinLowEdge(1);
        double binupedge = graph_1d.at(i)->GetXaxis()->GetBinUpEdge(binnum);
        TH1D *graph = new TH1D(TString::Format("graph%d", rand()), "graph", binnum, binlowedge, binupedge);
        for (int j = 1; j <= graph->GetNbinsX() / 2; j++)
        {
          graph->SetBinContent(j, (graph_1d.at(i)->GetBinContent(j) + graph_1d.at(i)->GetBinContent(binnum + 1 - j)) / 2);
          graph->SetBinError(j, sqrt((pow(graph_1d.at(i)->GetBinError(j), 2) + pow(graph_1d.at(i)->GetBinError(binnum + 1 - j), 2)) / 2));

          graph->SetBinContent(binnum + 1 - j, (graph_1d.at(i)->GetBinContent(j) + graph_1d.at(i)->GetBinContent(binnum + 1 - j)) / 2);
          graph->SetBinError(binnum + 1 - j, sqrt((pow(graph_1d.at(i)->GetBinError(j), 2) + pow(graph_1d.at(i)->GetBinError(binnum + 1 - j), 2)) / 2));
        }
        graph_1d.at(i) = graph;
      }
    }

    static void symmetrize(std::vector<TH1D *> &&graph_1d)
    {
      symmetrize(graph_1d); // 调用左值版本，此时graph_1d是左值
    }

  protected:
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

    static std::pair<double, double> findMinMaxBinContent(TH1D *hist)
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
  };

  class BasePlot // 基础库，请不要调用
  {
  public:
    BasePlot(std::vector<TH1D *> input, bool islarge) : hists(input)
    {
      // setCMSStyle(true);
      gStyle->SetOptStat(0);
      std::random_device rd;
      std::uniform_int_distribution<int> dist(1, 100000);
      int random_num = dist(rd);
      if (!islarge)
        cc = new TCanvas(TString::Format("cc%d", random_num), "canvas", 1900, 1600);
      else
      {
        cc = new TCanvas(TString::Format("cc%d", random_num), "canvas", 1100, 800);
        leg_position = {0.72, 0.3, 0.9, 0.9};
        pave_position = {0.72, 0.1, 0.9, 0.3};
        cc->SetRightMargin(0.3);
      }

      for (int i = 0; i < hists.size(); i++)
      {
        hists.at(i)->SetLineColor(color.at(i));
        hists.at(i)->SetLineWidth(3);
        hists.at(i)->SetMarkerColor(color.at(i));
        hists.at(i)->SetMarkerSize(0.5);
        hists.at(i)->SetTitle("");
        // hists.at(i)->GetXaxis()->SetRangeUser(0, 0.5);
        std::random_device rd;
        std::uniform_int_distribution<int> dist(1, 100000);
        int random_num = dist(rd);
        TH1D *h = (TH1D *)hists.at(i)->Clone(TString::Format("h_clone%d", random_num));
        h->Divide(hists.at(0));
        hists_clone.push_back(h);
      }
      Histdispose histdispose;
      std::pair<double, double> minmax = histdispose.findGlobalMinMax(hists);
      double low1 = minmax.first;
      double up1 = minmax.second;
      // std::cout << up1 << " " << low1 << std::endl;
      double minus1 = up1 - low1;
      empty_hist1 = (TH1D *)hists.at(0)->Clone();
      empty_hist1->Reset();
      empty_hist1->GetYaxis()->SetRangeUser(low1 - minus1 * 0.1, up1 + minus1 * 0.1);
      // empty_hist1->GetYaxis()->SetLabelSize(0.045);
      empty_hist1->SetLabelColor(kWhite);
      empty_hist1->SetLabelSize(0.0000001);
      empty_hist1->SetLineColor(kWhite);
      empty_hist1->SetLineWidth(0);

      std::pair<double, double> minmax_ratio = histdispose.findGlobalMinMax(hists_clone);
      double low2 = minmax_ratio.first;
      double up2 = minmax_ratio.second;
      double minus2 = up2 - low2;
      empty_hist2 = (TH1D *)hists_clone.at(0)->Clone();
      empty_hist2->Reset();
      empty_hist2->GetYaxis()->SetRangeUser(low2 - minus2 * 0.1, up2 + minus2 * 0.1);
      empty_hist2->SetLineColor(kWhite);
      empty_hist2->SetLineWidth(0);
      empty_hist2->GetXaxis()->SetLabelSize(0.08);
      empty_hist2->GetYaxis()->SetTitle("Ratio");
      empty_hist2->GetYaxis()->SetLabelSize(0.09);
      empty_hist2->GetYaxis()->SetTitleSize(0.15);
      empty_hist2->GetYaxis()->SetTitleOffset(0.3);
      empty_hist2->GetYaxis()->CenterTitle();
    }

    ~BasePlot()
    {
      delete cc;
      delete empty_hist1;
      delete empty_hist2;
      for (auto &hist : hists_clone)
      {
        delete hist;
      }
    }

  protected:
    TCanvas *cc;
    std::vector<TH1D *> hists;
    TH1D *empty_hist1, *empty_hist2;
    std::vector<TH1D *> hists_clone;
    std::vector<double> up_vec, low_vec;
    std::vector<double> up_vec_ratio, low_vec_ratio;
    std::vector<double> pave_position = {0.15, 0.75, 0.26, 0.9};
    std::vector<double> leg_position = {0.66, 0.75, 0.85, 0.85};
    std::vector<int> linestyle = {2, 2, 2, 2, 2};
    std::vector<double> linewidth = {2, 1, 1, 1, 1};
    // std::vector<int> color = {kBlack, kBlue, kRed, kOrange, kGreen, kViolet, kOrange, kYellow, kGray, kPink, kPearl, kDeepSea, kOcean, kBeach};
    std::vector<int> color = {kBlack, kBlue, kRed, kOrange, kViolet, kGreen, kYellow, kGray, kBeach, kPearl, kDeepSea, kOcean, kGreenPink};
    // std::vector<int> color = {kBlack, kBlue, kRed, 226, kViolet, kBlack, kBlack, kBlack, kBlack, kBlack, kBlack, kBlack, kBlack, kBlack};
    std::vector<int> marker = {20, 3, 22, 23};
    std::vector<int> histslinestyle = {};

    void SetColor(std::vector<int> color_)
    {
      color = color_;
      for (int i = 0; i < hists.size(); i++)
      {
        hists.at(i)->SetLineColor(color.at(i));
        hists_clone.at(i)->SetLineColor(color.at(i));
        hists.at(i)->SetMarkerColor(color.at(i));
        hists_clone.at(i)->SetMarkerColor(color.at(i));
      }
    }
    void SetLineStyle(std::vector<int> linestyle_)
    {
      histslinestyle = linestyle_;
      for (int i = 0; i < hists.size(); i++)
      {
        hists.at(i)->SetLineStyle(histslinestyle.at(i));
        hists_clone.at(i)->SetLineStyle(histslinestyle.at(i));
      }
    }

    void SetXRange(double xlow, double xup)
    {
      empty_hist1->GetXaxis()->SetRangeUser(xlow, xup);
      empty_hist2->GetXaxis()->SetRangeUser(xlow, xup);
    }

    void SetYRange(double ylow, double yup)
    {
      empty_hist1->GetYaxis()->SetRangeUser(ylow, yup);
    }
    void SetYRatioRange(double ylow, double yup)
    {
      empty_hist2->GetYaxis()->SetRangeUser(ylow, yup);
    }

    void SetYTitile(TString title, double size = 0.05, double offset = 1.05)
    {
      empty_hist1->GetYaxis()->SetTitle(title);
      empty_hist1->GetYaxis()->SetTitleSize(size);
      empty_hist1->GetYaxis()->SetTitleOffset(offset);
    }

    void SetLegPosition(std::vector<double> pos)
    {
      leg_position = pos;
    }
    void SetLegPosition(double pos1, double pos2, double pos3, double pos4)
    {
      leg_position = {pos1, pos2, pos3, pos4};
    }
    void AddLegend(std::vector<TString> legend, double size = 0.04, int column = 1)
    {
      TLegend *leg = new TLegend(leg_position.at(0), leg_position.at(1), leg_position.at(2), leg_position.at(3));
      for (int i = 0; i < legend.size(); i++)
      {
        leg->AddEntry(hists.at(i), legend.at(i), "lq");
      }
      leg->SetNColumns(column);
      leg->SetBorderSize(0);
      leg->SetFillColorAlpha(0, 0);
      leg->SetTextSize(size);
      leg->Draw("same");
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

    void AddLatex(const char *tex, double x = 0.91, double y = 0.02)
    {
      TLatex latex;
      latex.DrawLatexNDC(x, y, tex);
    }

    void AddLines(std::vector<std::vector<double>> lines)
    {
      for (int i = 0; i < lines.size(); i++)
      {
        TLine *line1 = new TLine(lines.at(i).at(0), lines.at(i).at(1), lines.at(i).at(2), lines.at(i).at(3));
        line1->SetLineStyle(linestyle.at(i));
        line1->SetLineWidth(linewidth.at(i));
        line1->SetLineColor(kBlack);
        line1->Draw("same");
      }
    }

    void AddChisquare(std::vector<double> pos = {0.42, 0.35, 0.55, 0.45}, double size = 0.035)
    {
      Histdispose histdispose_instance;
      std::vector<double> chi_squares = histdispose_instance.chi_square(hists);
      int nums = hists.size() - 1;
      TPaveText *paveText = new TPaveText(pos.at(0), pos.at(1), pos.at(2), pos.at(3), "NDC");
      for (int i = 0; i < nums; i++)
      {
        TText *text = paveText->AddText(TString::Format("#chi^{2}_{%d} = %.2f", i + 1, chi_squares.at(i)));
        text->SetTextColor(color.at(i + 1));
      }
      paveText->SetBorderSize(0);
      paveText->SetFillColor(0);
      paveText->SetFillColorAlpha(0, 0);
      paveText->SetTextFont(42);
      paveText->SetTextSize(size);
      paveText->SetTextAlign(12);
      paveText->Draw("same");
    }

    void AddText(std::vector<TString> texts, double ypos = 1, double size = 0.04)
    {
      empty_hist1->GetXaxis()->SetLabelSize(0); // 隐藏标签
      // empty_hist1->GetXaxis()->SetTickSize(0);  // 隐藏刻度线
      // empty_hist1->GetXaxis()->SetTitle("");    // 移除标题
      int nBins = empty_hist1->GetNbinsX();
      double ymin = empty_hist1->GetMinimum();
      double ymax = empty_hist1->GetMaximum();
      for (int i = 1; i <= nBins; i++)
      {
        double binCenter = empty_hist1->GetBinCenter(i);
        double binContent = empty_hist1->GetBinContent(i);
        TText *text = new TText(binCenter, ypos, texts.at(i - 1));
        text->SetTextAlign(22); // Center the text
        text->SetTextSize(size);
        text->SetTextFont(42);
        text->Draw("same");
      }
    }
  };

  class Plotratio : public BasePlot
  {
  public:
    Plotratio(std::vector<TH1D *> h, bool islarge = false) : BasePlot(h, islarge), pad1(nullptr), pad2(nullptr)
    {
      if (pad1 != nullptr)
      {
        delete pad1;
        pad1 = nullptr;
      }
      pad1 = new TPad("pad1", "Pad 1", 0, 0.3, 1, 1.0);
      if (pad2 != nullptr)
      {
        delete pad2;
        pad2 = nullptr;
      }
      pad2 = new TPad("pad2", "Pad 2", 0, 0, 1, 0.3);
      pad1->SetBottomMargin(0.001);
      pad1->SetLeftMargin(0.12);
      pad2->SetTopMargin(0.01);
      pad2->SetBottomMargin(0.2);
      pad2->SetLeftMargin(0.12);
      if (islarge)
      {
        pad1->SetRightMargin(0.32);
        pad2->SetRightMargin(0.32);
        pad1->SetLeftMargin(0.1);
        pad2->SetLeftMargin(0.1);
      }
      pad1->Draw();
      pad2->Draw();
    }

    ~Plotratio()
    {
      if (pad1)
      {
        delete pad1;
        pad1 = nullptr;
      }
      if (pad2)
      {
        delete pad2;
        pad2 = nullptr;
      }
    }

    using BasePlot::AddChisquare;
    using BasePlot::AddLatex;
    using BasePlot::AddLegend;
    using BasePlot::AddLines;
    using BasePlot::AddPave;
    using BasePlot::AddText;
    using BasePlot::SetColor;
    using BasePlot::SetLegPosition;
    using BasePlot::SetLineStyle;
    using BasePlot::SetPavePosition;
    using BasePlot::SetXRange;
    using BasePlot::SetYRange;
    using BasePlot::SetYRatioRange;
    using BasePlot::SetYTitile;

    void SetXRatioTitle(TString xtitle, double size = 0.13, double offset = 0.6)
    {
      BasePlot::empty_hist2->GetXaxis()->SetTitle(xtitle);
      BasePlot::empty_hist2->GetXaxis()->SetTitleSize(size);
      BasePlot::empty_hist2->GetXaxis()->SetTitleOffset(offset);
    }
    void SetLogX()
    {
      pad1->SetLogx();
      pad2->SetLogx();
    }
    void SetLogY()
    {
      pad1->SetLogy();
      pad2->SetLogy();
    }
    void Draw()
    {
      pad1->cd();
      empty_hist1->Draw();
      for (int i = 0; i < BasePlot::hists.size(); i++)
        BasePlot::hists.at(i)->Draw("same");
      pad2->cd();
      empty_hist2->Draw();
      TLine *lineraito = new TLine(400, 0.99, 1500, 0.99);
      lineraito->SetLineStyle(2);
      lineraito->Draw("same");
      for (int i = 0; i < BasePlot::hists_clone.size(); i++)
        BasePlot::hists_clone.at(i)->Draw("same");
      BasePlot::cc->cd();
    }
    void Write(TString filename)
    {
      BasePlot::cc->SaveAs(filename);
    }

  private:
    TPad *pad1, *pad2;
  };

  class Plot : public BasePlot
  {
  public:
    Plot(std::vector<TH1D *> h, bool islarge = false) : BasePlot(h, islarge)
    {
      BasePlot::empty_hist1->GetXaxis()->SetLabelSize(0.04);
      BasePlot::empty_hist1->GetXaxis()->SetLabelColor(kBlack);
    }

    using BasePlot::AddChisquare;
    using BasePlot::AddLatex;
    using BasePlot::AddLegend;
    using BasePlot::AddLines;
    using BasePlot::AddPave;
    using BasePlot::AddText;
    using BasePlot::SetColor;
    using BasePlot::SetLegPosition;
    using BasePlot::SetLineStyle;
    using BasePlot::SetPavePosition;
    using BasePlot::SetXRange;
    using BasePlot::SetYRange;
    using BasePlot::SetYTitile;

    void SetXTitle(TString xtitle, double size = 0.06, double offset = 0.7)
    {
      BasePlot::empty_hist1->GetXaxis()->SetTitle(xtitle);
      BasePlot::empty_hist1->GetXaxis()->SetTitleSize(size);
      BasePlot::empty_hist1->GetXaxis()->SetTitleOffset(offset);
    }
    void SetLogX()
    {
      BasePlot::cc->SetLogx();
    }
    void SetLogY()
    {
      BasePlot::cc->SetLogy();
    }
    void Draw()
    {
      BasePlot::cc->cd();
      empty_hist1->Draw("HIST");
      empty_hist1->SetFillStyle(0);
      for (int i = 0; i < BasePlot::hists.size(); i++)
      {
        BasePlot::hists.at(i)->SetFillStyle(0);
        BasePlot::hists.at(i)->Draw("same");
      }
      BasePlot::cc->cd();
    }
    void Write(TString filename)
    {
      BasePlot::cc->SaveAs(filename);
    }
  };

  class Projection
  {
  public:
    Projection(TH2D *input) : hist(input) {}

    void ProjectX(int binwith)
    {
      pairs = CreateProjectionsX(binwith);
    }

    void ProjectY(int binwith)
    {
      pairs = CreateProjectionsY(binwith);
    }

    void Initialize(int begin_, int end_)
    {
      binbegin = begin_;
      binend = end_;
    }

    std::vector<TH1D *> Gethists()
    {
      std::vector<TH1D *> vecs;
      for (int i = binbegin; i <= binend; i++)
      {
        TH1D *h = pairs.at(i).first;
        vecs.push_back(h);
      }
      return vecs;
    }

    std::vector<TString> Getlegends(TString variable, int decimal = 2)
    {
      std::vector<TString> legends;
      for (int i = binbegin; i <= binend; i++)
      {
        std::vector<double> ranges = pairs.at(i).second;
        TString legend = TString::Format(("%." + std::to_string(decimal) + "f < ").c_str(), ranges.at(0)) + variable +
                         TString::Format((" < %." + std::to_string(decimal) + "f").c_str(), ranges.at(1));
        legends.push_back(legend);
      }
      return legends;
    }

  protected:
    std::vector<std::pair<TH1D *, std::vector<double>>> CreateProjectionsX(int binWidth)
    {
      std::vector<std::pair<TH1D *, std::vector<double>>> hist1D_X_and_Ranges;
      int nBinsY = hist->GetNbinsY();
      for (int i = 1; i <= nBinsY; i += binWidth)
      {
        int binMin = i;
        int binMax = std::min(i + binWidth - 1, nBinsY); // 确保不超过最大bin数
        double binMin_value = hist->GetYaxis()->GetBinLowEdge(binMin);
        double binMax_value = hist->GetYaxis()->GetBinUpEdge(binMax);
        TString name = Form("projX_ybin%f-%f", binMin_value, binMax_value);
        TH1D *projX = hist->ProjectionX(name, binMin, binMax);
        std::vector<double> yRange = {binMin_value, binMax_value};
        hist1D_X_and_Ranges.push_back(make_pair(projX, yRange));
      }
      return hist1D_X_and_Ranges;
    }

    std::vector<std::pair<TH1D *, std::vector<double>>> CreateProjectionsY(int binWidth)
    {
      std::vector<std::pair<TH1D *, std::vector<double>>> hist1D_Y_and_Ranges;
      int nBinsX = hist->GetNbinsX();
      for (int i = 1; i <= nBinsX; i += binWidth)
      {
        int binMin = i;
        int binMax = std::min(i + binWidth - 1, nBinsX); // 确保不超过最大bin数
        double binMin_value = hist->GetXaxis()->GetBinLowEdge(binMin);
        double binMax_value = hist->GetXaxis()->GetBinUpEdge(binMax);
        TString name = Form("projY_xbin%f-%f", binMin_value, binMax_value);
        TH1D *projY = hist->ProjectionY(name, binMin, binMax);
        std::vector<double> xRange = {binMin_value, binMax_value};
        hist1D_Y_and_Ranges.push_back(make_pair(projY, xRange));
      }
      return hist1D_Y_and_Ranges;
    }

  private:
    TH2D *hist;
    std::vector<TH1D *> vecs_1dx, vecs_1dy;
    std::vector<std::pair<TH1D *, std::vector<double>>> pairs;
    int binbegin = 0;
    int binend = 2;
  };

  class Plot_TH2D
  {
  public:
    Plot_TH2D(TH2D *input) : hist(input)
    {
      if (!hist)
      {
        std::cerr << "Invalid histogram pointer" << std::endl;
        return;
      }
      cc = new TCanvas("cc", "cc", 900, 800);
      gStyle->SetOptStat(0);
      gStyle->SetOptTitle(1);
      gStyle->SetTitleAlign(23);
      gStyle->SetTitleX(0.6);
      gStyle->SetTitleY(0.97);  // 设置标题靠近顶部
      cc->SetRightMargin(0.15); // 确保右边的间距足够显示 z 轴标签
    }
    // ~Plot_TH2D()
    // {
    //   if (cc)
    //   {
    //     delete cc;
    //   }
    // }

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
      // cc->Update();
    }

    static void Normalize(TH3D *hist, char mode)
    {
      if (!hist)
      {
        std::cerr << "Invalid histogram pointer" << std::endl;
        return;
      }
      if (mode == 't' || mode == 'T')
      {
        double total = 0;
        for (int i = 0; i <= hist->GetNbinsX() + 1; ++i)
        {
          for (int j = 0; j <= hist->GetNbinsY() + 1; ++j)
          {
            for (int k = 0; k <= hist->GetNbinsZ() + 1; ++k)
            {
              total += hist->GetBinContent(i, j, k);
              // std::cout << hist->GetBinContent(i, j) << std::endl;
            }
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
            for (int k = 0; k <= hist->GetNbinsZ() + 1; ++k)
            {
              double oldValue = hist->GetBinContent(i, j, k);
              hist->SetBinContent(i, j, k, oldValue / total);
              hist->SetBinError(i, j, k, hist->GetBinError(i, j, k) / total);
            }
          }
        }
      }
      else
      {
        std::cerr << "Invalid mode. Use 'x', 'y' or 't'." << std::endl;
      }
      // cc->Update();
    }

    //   void Rebin(int rebin_x = 2, int rebin_y = 2)
    //   {
    //     if (!hist)
    //     {
    //       std::cerr << "Invalid histogram pointer" << std::endl;
    //       return;
    //     }
    //     hist->Rebin2D(rebin_x, rebin_y);
    //   }

    void SetTitle(TString title, TString xtitle, TString ytitle,
                  std::vector<double> xsize_offset = {0.05, 0.8},
                  std::vector<double> ysize_offset = {0.05, 0.8})
    {
      hist->SetTitle(title);
      hist->GetXaxis()->SetTitle(xtitle);
      hist->GetXaxis()->SetTitleSize(xsize_offset.at(0));
      hist->GetXaxis()->SetTitleOffset(xsize_offset.at(1));
      hist->GetYaxis()->SetTitle(ytitle);
      hist->GetYaxis()->SetTitleSize(ysize_offset.at(0));
      hist->GetYaxis()->SetTitleOffset(ysize_offset.at(1));
    }

    void Draw()
    {
      if (cc)
      {
        cc->cd();
        hist->Draw("COLZ");
        cc->Update();
      }
    }
    void Write(TString name)
    {
      if (cc)
      {
        cc->SaveAs(name);
      }
    }

  private:
    TH2D *hist;
    TCanvas *cc;
  };
}