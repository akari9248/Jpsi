#ifndef CHISCAN_H
#define CHISCAN_H
#include "TLorentzVector.h"
#include "TVector3.h"
#include <vector>
#include <cstdlib>
#include "TH1D.h"
#include "TH2D.h"
#include <cmath>
#include <iostream>
#include "TMath.h"
#include "TString.h"
#include "TH1D.h"
#include "TMatrixD.h"
#include <algorithm>
#include <numeric>
using namespace std;
namespace chiscan
{
  class chiscan
  {
  public:
    TH1D *data;
    TH2D *datacov;
    TMatrixD *datacovinv;
    TH1D *theory;
    TH1D *modified_theory;
    vector<TH1D *> nuisance;
    vector<TH1D *> nuisanceup;
    vector<TH1D *> nuisancedn;
    vector<double> nuisance_theta;

    vector<int> bin_range;
    vector<double> height_pool;
    vector<vector<double>> theta_pool;
    bool float_theory_height = false;
    double float_height_distance = 0;
    int nuisance_num;
    double float_height_distance_best = 0;
    vector<double> nuisance_theta_best;
    double chi2_best = 1e100;

    int nslice = 10;
    int nbestpars = 1;
    vector<double> bestchis;
    vector<double> bestchis_order;
    vector<vector<double>> bestpars;
    bool isnuisanceupdn = false;
   
    double ktorderabs = 1;
    int ktorder_dir =1;
    double ktorder = ktorderabs*ktorder_dir;
    int k = 0;
    int max_scan_num = -1;
    double height_range = 0.08;
    double nuisance_range = 4;
    bool TransformToAntiKT = false;
    double prechi2sum = 0;
    double chi2sum = 0;
    double TransformToAntiKT_boudary = 0;
    double ext_range = 0.1;
    bool isprint_iteration_result = true;
    bool isinvlocal = false;
    double expect_resolution = 0.0001;
    bool isprint_result = true;
    chiscan(TH1D *_data, TH2D *_datacov, TH1D *_theory, vector<TH1D *> _nuisance, vector<int> _bin_range = {-1, -1}, bool _float_theory_height = false)
    {
      nuisance = _nuisance;
      nuisance_num = nuisance.size();
      data = _data;
      datacov = _datacov;
      theory = _theory;
      float_theory_height = _float_theory_height;
      isnuisanceupdn = false;
      if (_bin_range.at(1) == -1)
      {
        bin_range = {1, data->GetNbinsX()};
      }
      else
      {
        bin_range = _bin_range;
      }
      init();
    }
    /*nuisance up >0 and nuisance dn <0*/
    chiscan(TH1D *_data, TH2D *_datacov, TH1D *_theory, vector<TH1D *> _nuisanceup, vector<TH1D *> _nuisancedn, vector<int> _bin_range = {-1, -1}, bool _float_theory_height = false)
    {
      nuisanceup = _nuisanceup;
      nuisancedn = _nuisancedn;
      nuisance_num = _nuisanceup.size();
      data = _data;
      datacov = _datacov;
      theory = _theory;
      float_theory_height = _float_theory_height;
      isnuisanceupdn = true;
      if (_bin_range.at(1) == -1)
      {
        bin_range = {1, data->GetNbinsX()};
      }
      else
      {
        bin_range = _bin_range;
      }
      init();
    }

    void init()
    {

      modified_theory = (TH1D *)theory->Clone();

      inverse_cov();
      for (int i = 0; i < nuisance_num; i++)
      {
        nuisance_theta.push_back(0);
      }
      for (int i = 0; i < nuisance_num; i++)
      {
        nbestpars *= 6;
        theta_pool.push_back({});
        for (int ii = 0; ii < nslice + 1; ii++)
        {

          theta_pool.at(i).push_back(-0.5 * nuisance_range + nuisance_range / nslice * ii);
        }
      }
      if (float_theory_height)
      {
        nbestpars *= 6;
        for (int i = 0; i < nslice + 1; i++)
        {
          height_pool.push_back(-0.5 * height_range + height_range / nslice * i);
        }
      }
      else
      {
        height_pool.push_back(0);
      }
      for (int i = 0; i < nbestpars; i++)
      {
        bestchis.push_back(1e100);
        bestpars.push_back({});
      }
    }
    void inverse_cov_local(){
      cout<<"-----------Inversing Local Covariance ---------------------"<<endl;
      int nbinsx = bin_range.at(1) - bin_range.at(0)+1;
      int nbinsy = bin_range.at(1) - bin_range.at(0)+1;
      TString randstr = TString::Format("%d",rand());
      TH2D *datacov_local = new TH2D("datacov_local"+randstr,"datacov_local"+randstr,nbinsx,0,nbinsx,nbinsy,0,nbinsy);
      for (int i = 1; i <= nbinsx; i++) {
        for (int j = 1; j <= nbinsy; j++) {
          datacov_local->SetBinContent(i,j,datacov->GetBinContent(i+bin_range.at(0)-1,j+bin_range.at(0)-1));
        }
      }
      TMatrixD matrix = TMatrixD(nbinsx + 2, nbinsy + 2, datacov_local->GetArray(), "D");
      TMatrixD datacovinv0 = matrix.GetSub(1, nbinsx, 1, nbinsy);
      TMatrixD *datacovinv_local = (TMatrixD *)datacovinv0.Clone();
      datacovinv_local->Invert();
      Double_t* data = datacovinv->GetMatrixArray();
      Double_t* data1 = datacovinv_local->GetMatrixArray();
      Int_t nrows = datacovinv->GetNrows();
      Int_t ncols = datacovinv->GetNcols();
      TFile *f = new TFile("test.root","RECREATE");
      datacov->Write("datacov");
      TH2D *datacovinv_hist = Matrix2TH2D(datacovinv);
      datacovinv_hist->Write("datacovinv_hist");
      TH2D *datacovinv_local_hist = Matrix2TH2D(datacovinv_local);
      for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
          //cout<<i<<" "<<j<<endl;
          if(i<bin_range.at(0)-1||i>bin_range.at(1)-1){
            data[i * ncols + j] = 0;
          }
          if(j<bin_range.at(0)-1||j>bin_range.at(1)-1){
            data[i * ncols + j] = 0;
          }
          if(i>=bin_range.at(0)-1&&i<=bin_range.at(1)-1&&j>=bin_range.at(0)-1&&j<=bin_range.at(1)-1){
            
            data[i * ncols + j] = datacovinv_local_hist->GetBinContent(i-(bin_range.at(0)-1)+1,j-(bin_range.at(0)-1)+1);
          }
          // if(i>=bin_range.at(0)-1&&i<=bin_range.at(1)-1&&i==j){
            
          //   data[i * ncols + j] = datacovinv_local_hist->GetBinContent(i-(bin_range.at(0)-1)+1,j-(bin_range.at(0)-1)+1);
          // }
          // if(i!=j){
          //   data[i * ncols + j] = 0;
          // }
        }
      }
      
      datacovinv_local_hist->Write("datacovinv_local_hist");
      datacovinv_hist = Matrix2TH2D(datacovinv);
      datacovinv_hist->Write("datacovinv_hist_modified");
      f->Close();
    }
    static TH2D *Matrix2TH2D(TMatrixD *input){
      Int_t nrows = input->GetNrows();
      Int_t ncols = input->GetNcols();
      Double_t* data = input->GetMatrixArray();
      TH2D *hist = new TH2D(TString::Format("%d",rand()),TString::Format("%d",rand()),nrows,0,nrows,ncols,0,ncols);
       for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
          //hist->SetBinContent(i+1,j+1,data[(nrows-i-1) * ncols + j]);
          hist->SetBinContent(i+1,j+1,data[i * ncols + j]);
        }
       }
       return hist;
    }
    void inverse_cov()
    {
      int nbinsx = datacov->GetNbinsX();
      int nbinsy = datacov->GetNbinsY();
      TMatrixD matrix = TMatrixD(nbinsx + 2, nbinsy + 2, datacov->GetArray(), "D");
      TMatrixD datacovinv0 = matrix.GetSub(1, nbinsx, 1, nbinsy);
      datacovinv = (TMatrixD *)datacovinv0.Clone();
      datacovinv->Invert();
      
    }

    void scanchi2()
    {
      if(isinvlocal) inverse_cov_local();
      vector<double> min, max;
      LoopScan(nuisance_num + 1, 0);
      
      int count_reach = -1;
      while (k <= 40 || count_reach <= 25)
      {
        auto temp = bestchis;
        bestchis_order= bestchis;
        std::sort(bestchis_order.begin(), bestchis_order.end());
        std::sort(temp.begin(), temp.end());
        // if(prechi2sum - chi2sum<0.00001) count_reach++;
        // cout << chi2_best << " " << temp.at(1) << " " << temp.at(2) << " " << count_reach << endl;
        // cout << theta_pool.at(0).at(0) << " " << theta_pool.at(0).at(nslice) << endl;
        // cout << height_pool.at(0) << " " << height_pool.at(height_pool.size() - 1) << endl;
        // cout << k << " " << prechi2sum << " " << chi2sum << " " << float_height_distance_best << " " << nuisance_theta_best.at(0) << endl;
        // // cout<<height_pool.at(0)<<" "<<height_pool.at(nslice-1)<<" "<<temp.at(temp.size()-1) - temp.at(0)<<endl;
        prechi2sum = std::accumulate(bestchis.begin(), bestchis.end(), 0.0);
        
        GetBestPars(min, max);
        re_asign_pool(min, max);
        TransformToAntiKT = true;
        for (int i = 0; i < max.size() - 1; i++)
        {
          TransformToAntiKT = TransformToAntiKT && ((max.at(i) - min.at(i)) < nuisance_range*TransformToAntiKT_boudary);
        }
        TransformToAntiKT = TransformToAntiKT && ((max.at(max.size() - 1) - min.at(max.size() - 1))  < height_range*TransformToAntiKT_boudary);
        if (TransformToAntiKT)
          ktorder_dir = -1;;
        ktorder = ktorderabs*ktorder_dir;
        //ktorder = 5;
        
        
        LoopScan(nuisance_num + 1, 0);
        chi2sum = std::accumulate(bestchis.begin(), bestchis.end(), 0.0);
        if(isprint_iteration_result)
          print_iteration_result();
        if (temp.at(temp.size() - 1) - temp.at(0) < expect_resolution)
          count_reach++;
        k++;
        if (k > max_scan_num && max_scan_num != -1)
          break;
        // std::sort(bestchis.begin(), bestchis.end());
        //  cout<<k<<" ";
        //  for(int i=0;i<bestchis.size();i++){
        //    cout<<bestchis.at(i)<<"  ";
        //  }
        //  cout<<endl;

        // cout<<chi2_best<<" "<<k<<" "<<height_pool.at(0)<<" "<<height_pool.at(nslice)<<endl;
        // cout<<chi2_best<<" "<<k<<" "<<theta_pool.at(0).at(0)<<" "<<theta_pool.at(0).at(nslice)<<endl;
        // //cout<<nuisance_theta_best.at(0)<<" "<<float_height_distance_best<<endl;
      }
      if(isprint_result)
        print_result();
      // cout<<theta_pool.at(0).at(1)<<endl;
    }
    void HiddenOutput(){
      isprint_result=false;
      isprint_iteration_result=false;
    }
    void write_result(TString out_name = "scan_out.root"){
      TFile *f_out = new TFile(out_name,"RECREATE");
      TH1D *nuisance_outs = new TH1D("nuisance_best_fit","nuisance_best_fit",nuisance_num,0,nuisance_num);
      for(int i=0;i<nuisance_num;i++){
        nuisance_outs->SetBinContent(i+1,nuisance_theta_best.at(i));
      }
      TH1D *theory_float_out = new TH1D("theory_float_out","theory_float_out",1,0,1);
      theory_float_out->SetBinContent(1,float_height_distance_best);
      TH1D *chi2_best_out = new TH1D("chi2_best_out","chi2_best_out",1,0,1);
      chi2_best_out->SetBinContent(1,chi2_best);
      TH1D *Number_of_scans_out = new TH1D("Number_of_scans_out","Number_of_scans_out",1,0,1);
      Number_of_scans_out->SetBinContent(1,k);

      
      for(int i=0;i<data->GetNbinsX();i++){
        data->SetBinError(i+1,sqrt(0));
        if((*datacovinv)(i,i) != 0)
          data->SetBinError(i+1,sqrt(1.0/(*datacovinv)(i,i)));
      }
      data->Write("Data");
      theory->Write("Theory");
      datacov->Write("DataCov");
      TH2D *datacovinv_hist =(TH2D *) datacov->Clone();
      for (int i = 1; i <= datacov->GetNbinsX(); i++) {
        for (int j = 1; j <= datacov->GetNbinsY(); j++) {
          auto datacovinvx = *datacovinv;
          datacovinv_hist->SetBinContent(i,j,datacovinvx(i-1,j-1));
        }
      }

      datacovinv_hist->Write("DataCovInv");
      
      for(int i=0;i<nuisanceup.size();i++){
        nuisanceup.at(i)->Write(TString::Format("NuisanceUp%d",i));
        nuisancedn.at(i)->Write(TString::Format("NuisanceDn%d",i));
      }
      nuisance_outs->Write("nuisance_best_fit");
      theory_float_out->Write("float_best_fit");
      chi2_best_out->Write("chi2");
      Number_of_scans_out->Write("Number_of_scans_out");
      TH1D *bestfit_hist = GetModifiedTheoryPrediction(theory,nuisanceup,nuisancedn,nuisance_theta_best,float_height_distance_best);
      bestfit_hist->Write("best_fit_theory");
      bestfit_hist->Divide(theory);
      for(int i=1;i<=bestfit_hist->GetNbinsX();i++){
        bestfit_hist->SetBinContent(i,bestfit_hist->GetBinContent(i)-1);
      }
      bestfit_hist->GetYaxis()->SetRangeUser(-0.5,0.5);
      bestfit_hist->Write("best_fit_theory_ratio");
      bestfit_hist = GetModifiedTheoryPrediction(theory,nuisanceup,nuisancedn,nuisance_theta_best,float_height_distance_best);
      bestfit_hist->Divide(data);
      for(int i=1;i<=bestfit_hist->GetNbinsX();i++){
        bestfit_hist->SetBinContent(i,bestfit_hist->GetBinContent(i)-1);
      }
      bestfit_hist->GetYaxis()->SetRangeUser(-0.5,0.5);
      bestfit_hist->Write("best_fit_data_ratio");
      f_out->Close();
    }
    void print_result()
    {
      cout << "-----------Chi2 Scan Result-------------" << endl;
      cout << "Chi2 = " << chi2_best << endl;
      cout << "Theory Float = " << float_height_distance_best << endl;
      for (int i = 0; i < nuisance_num; i++)
      {
        cout << "Nuisance" << i << "    = " << nuisance_theta_best.at(i) << endl;
      }
      cout << "Number of scans = " << k << endl;
    }
    void print_iteration_result(){

      cout << "------------ iter "<< k<<" -----------------" << endl;
      cout << "Best Chi2s = " << bestchis.at(0)<<" "<<bestchis_order.at(1)<<" "<<bestchis_order.at(2)<<" ..." << endl;
      cout << "Best Chi2s Sum = " << std::setprecision(6) << prechi2sum << " => " << std::setprecision(6) << chi2sum << endl;

      cout << "Best Theory Float = " << float_height_distance_best << endl;
      for (int i = 0; i < nuisance_num; i++)
      {
        cout << "Best Nuisance" << i << " = "<<nuisance_theta_best.at(i) << endl;
      }
      cout << "Theory Float Range =  [ " << height_pool.at(0) << " - " << height_pool.at(height_pool.size() - 1)<<" ]" << endl;
      for (int i = 0; i < nuisance_num; i++)
      {
        cout << "Nuisance" << i << " = [ " << theta_pool.at(i).at(0) << " - " << theta_pool.at(i).at(nslice)<<" ] " << endl;
      }
      cout << "Using algorithm = ";
      if(ktorder < 0 ){
        cout << " Anti-KT , Parameter = "<< ktorderabs<<endl;
      }
      if(ktorder > 0){
        cout << "   KT    , Parameter = "<< ktorderabs<<endl;
      }

    }
    void re_asign_pool(vector<double> &min, vector<double> &max)
    {
      for (int i = 0; i <= nuisance_num; i++)
      {
        if (min.at(i) == max.at(i) && i < nuisance_num)
        {
          int index = findindices(theta_pool.at(i), min.at(i));
          if (index != -1)
          {
            if (index == theta_pool.at(i).size() - 1)
            {
              min.at(i) = theta_pool.at(i).at(index - 1);
              max.at(i) = 2 * theta_pool.at(i).at(index) - theta_pool.at(i).at(index - 1);
            }
            else if (index == 0)
            {
              min.at(i) = 2 * theta_pool.at(i).at(0) - theta_pool.at(i).at(1);
              max.at(i) = theta_pool.at(i).at(index + 1);
            }
            else
            {
              min.at(i) = theta_pool.at(i).at(index - 1);
              max.at(i) = theta_pool.at(i).at(index + 1);
            }
          }
        }
        if (min.at(i) == max.at(i) && i == nuisance_num)
        {
          if (height_pool.size() != 1)
          {

            int index = findindices(height_pool, min.at(i));
            if (index != -1)
            {
              if (index == height_pool.size() - 1)
              {
                min.at(i) = height_pool.at(index - 1);
                max.at(i) = 2 * height_pool.at(index) - height_pool.at(index - 1);
              }
              else if (index == 0)
              {
                min.at(i) = 2 * height_pool.at(0) - height_pool.at(1);
                max.at(i) = height_pool.at(index + 1);
              }
              else
              {
                min.at(i) = height_pool.at(index - 1);
                max.at(i) = height_pool.at(index + 1);
              }
            }
          }
          else
          {
            min.at(i) = height_pool.at(0);
            max.at(i) = height_pool.at(0);
          }
        }
        double width = max.at(i) - min.at(i);
        double r = ((double)rand() / (RAND_MAX));
        
        if(i<min.size()-1&&(1+ext_range)*width<nuisance_range){
           min.at(i) = min.at(i) - width *ext_range/2;
          max.at(i) = max.at(i) + width *ext_range/2;
        }
        if(i==min.size()-1&&2*width<height_range){
          min.at(i) = min.at(i) - width *ext_range/2;
          max.at(i) = max.at(i) + width *ext_range/2;
        }
       
        // min.at(i) = min.at(i) - width *0.01*r;
        // max.at(i) = max.at(i) + width *0.01*r;
      }
      if(height_pool.size()!=1)
        height_pool.clear();
      for (int i = 0; i < min.size(); i++)
      {
        if (i != min.size() - 1)
          theta_pool.at(i).clear();
        vector<double> stepweight;
        double sumweight = 0;
        // for(int k=0;k<nslice;k++){
        //   stepweight.push_back(pow(abs(k - nslice/2)+1,2));
        //   sumweight += stepweight.at(k);
        // }
        // double stepwidth=(max.at(i)-min.at(i))*1.0/sumweight;
        double centralweight = 1;
        // if (i < min.size() - 1)
        // {
        //   centralweight = (max.at(i) - nuisance_theta_best.at(i)) / (nuisance_theta_best.at(i) - min.at(i));
        // }
        // else
        // {
        //   centralweight = (max.at(i) - float_height_distance_best) / (float_height_distance_best - min.at(i));
        // }
        for (int k = 0; k < nslice; k++)
        {
          // double dir= ((double) rand() / (RAND_MAX)) - 0.5*2;

          stepweight.push_back(pow(abs(k - nslice / 2 + 0.5), ktorder));
          if (k - nslice / 2 >= 0)
            stepweight.at(k) *= centralweight;
          sumweight += stepweight.at(k);
        }
        for (int k = 0; k < nslice - 1; k++)
        {
          double r = ((double)rand() / (RAND_MAX));
          double range = stepweight.at(k + 1) + stepweight.at(k);
          stepweight.at(k) = range * r;
          stepweight.at(k + 1) = range - range * r;
        }
        // stepweight.at(nslice-1)=sumweight-tempsumweight;
        double stepwidth = (max.at(i) - min.at(i)) * 1.0 / sumweight;
        double sumweight0 = 0;
        for (int k = 0; k <= nslice; k++)
        {
          if (i < min.size() - 1)
          {
            theta_pool.at(i).push_back(min.at(i) + sumweight0 * stepwidth);
          }
          else if (float_theory_height)
          {
            height_pool.push_back(min.at(i) + sumweight0 * stepwidth);
          }
          if (k != nslice)
            sumweight0 += stepweight.at(k);
          
           
        }

      }
    }
    int findindices(vector<double> values, double num)
    {
      for (int i = 0; i < values.size(); ++i)
      {
        if (values[i] == num)
        {
          return i;
        }
      }
      return -1;
    }
    void GetBestPars(vector<double> &min, vector<double> &max)
    {
      int numRows = bestpars.size();
      int numCols = bestpars[0].size();
      std::vector<double> maxVals(numCols, -std::numeric_limits<double>::max());
      std::vector<double> minVals(numCols, std::numeric_limits<double>::max());

      // 遍历二维向量，更新每一列的最大值和最小值
      for (int j = 0; j < numCols; ++j)
      {
        for (int i = 0; i < numRows; ++i)
        {
          if (bestpars[i][j] > maxVals[j])
          {
            maxVals[j] = bestpars[i][j];
          }
          if (bestpars[i][j] < minVals[j])
          {
            minVals[j] = bestpars[i][j];
          }
        }
      }
      max = maxVals;
      min = minVals;
    }
    void LoopScan(int nLevel, int currentLevel)
    {
      if (currentLevel == nLevel - 1)
      {
        for (int i = 0; i < height_pool.size(); ++i)
        {
          float_height_distance = height_pool.at(i);
          GetTheoryPrediction();
          double chi2_out = calchi2();
          if (chi2_out < chi2_best)
          {
            chi2_best = chi2_out;
            float_height_distance_best = float_height_distance;
            nuisance_theta_best = nuisance_theta;
          }
          int begin = 0;
          for (int ii = 0; ii < bestchis.size(); ii++)
          {
            if (bestchis.at(ii) == 1e100)
            {
              begin = ii;
              break;
            }
          }
          for (int ii = begin; ii < bestchis.size(); ii++)
          {
            if (chi2_out < bestchis.at(ii))
            {
              if (findindices(bestchis, chi2_out) != -1)
              {
                bool iscontinue = true;
                for (int i = 0; i < nuisance_num; i++)
                {
                  if (nuisance_theta.at(i) != bestpars.at(findindices(bestchis, chi2_out)).at(i))
                  {
                    iscontinue = false;
                  }
                }
                if (float_height_distance == bestpars.at(findindices(bestchis, chi2_out)).at(nuisance_num) && iscontinue)
                  continue;
              }
              bestchis.at(ii) = chi2_out;
              bestpars.at(ii) = nuisance_theta;
              bestpars.at(ii).push_back(float_height_distance);

              break;
            }
          }
        }
      }
      else
      {
        for (int i = 0; i < theta_pool.at(currentLevel).size(); ++i)
        {

          nuisance_theta.at(currentLevel) = theta_pool.at(currentLevel).at(i);
          LoopScan(nLevel, currentLevel + 1);
        }
      }
    }

    double calchi2()
    {
      double chi2 = 0;
      TH1D *diff = (TH1D *)data->Clone();
      diff->Add(modified_theory, -1);
      vector<double> temp = {};
      auto datacovinvx = *datacovinv;
      for (int i = bin_range.at(0); i <= bin_range.at(1); i++)
      {
        double num = 0;
        for (int j = bin_range.at(0); j <= bin_range.at(1); j++)
        {
          
          num += diff->GetBinContent(j) * datacovinvx(i - 1, j - 1);
        }
        temp.push_back(num);
      }
      for (int i = bin_range.at(0); i <= bin_range.at(1); i++)
      {
        chi2 += temp.at(i - bin_range.at(0)) * diff->GetBinContent(i);
        //cout<<i<<" "<< temp.at(i - bin_range.at(0)) * diff->GetBinContent(i)<<" "<<diff->GetBinContent(i)<<" "<<datacovinvx(i - 1, i - 1)<<endl;
      }
      for (int i = 0; i < nuisance_theta.size(); i++)
      {
        chi2 += nuisance_theta.at(i) * nuisance_theta.at(i);
      }
      diff->Delete();
      return chi2;
    }
    // double calchi2()
    // {
    //   double chi2 = 0;
    //   TH1D *diff = (TH1D *)data->Clone();
    //   diff->Add(modified_theory, -1);
    //   vector<double> temp = {};
    //   for (int i = bin_range.at(0); i <= bin_range.at(1); i++)
    //   {
    //     double num = 0;
    //     for (int j = bin_range.at(0); j <= bin_range.at(1); j++)
    //     {
    //       auto datacovinvx = *datacovinv;
    //       num += diff->GetBinContent(j) * datacovinvx(i-1, j-1);

    //     }
    //     temp.push_back(num);
    //   }
    //   for (int i = bin_range.at(0); i <= bin_range.at(1); i++)
    //   {
    //     chi2 += temp.at(i-bin_range.at(0)) * diff->GetBinContent(i);
    //   }
    //   return chi2;
    // }

    void GetTheoryPrediction()
    {
      for (int i = 1; i <= modified_theory->GetNbinsX() + 1; i++)
      {
        double cot = theory->GetBinContent(i);
        
        for (int j = 0; j < nuisance_num; j++)
        {
          if (isnuisanceupdn)
          {
            if(abs(nuisance_theta.at(j))<1){
              if (nuisance_theta.at(j) > 0)
              cot = cot * pow((1 + nuisanceup.at(j)->GetBinContent(i)), nuisance_theta.at(j));
            else
              cot = cot * pow((1 + nuisancedn.at(j)->GetBinContent(i)), abs(nuisance_theta.at(j)));
            }else{
              if (nuisance_theta.at(j) > 0)
              cot = cot * (1 + nuisanceup.at(j)->GetBinContent(i)*nuisance_theta.at(j));
            else
              cot = cot * (1 + nuisancedn.at(j)->GetBinContent(i)* abs(nuisance_theta.at(j)));
            }
          }
          else
          {
            cot = cot * pow((1 + nuisance.at(j)->GetBinContent(i)), nuisance_theta.at(j));
          }
        }
        cot = cot + float_height_distance;
        modified_theory->SetBinContent(i, cot);
      }
    }
    TH1D *GetModifiedTheoryPrediction(TH1D *theory, vector<TH1D *> nuisanceup,
                                      vector<TH1D *> nuisancedn,
                                      vector<double> nuisance_theta,
                                      double float_height_distance) {
      TH1D *modified_theory_out = (TH1D *)theory->Clone();
      int num_bins = theory->GetNbinsX();
      for (int i = 1; i <= num_bins + 1; i++) {
        double cot = theory->GetBinContent(i);
        
        for (int j = 0; j < nuisanceup.size(); j++) {
          if (abs(nuisance_theta.at(j)) < 1) {
            if (nuisance_theta.at(j) > 0)
              cot = cot * pow((1 + nuisanceup.at(j)->GetBinContent(i)),
                              nuisance_theta.at(j));
            else
              cot = cot * pow((1 + nuisancedn.at(j)->GetBinContent(i)),
                              abs(nuisance_theta.at(j)));
          } else {
            if (nuisance_theta.at(j) > 0)
              cot = cot * (1 + nuisanceup.at(j)->GetBinContent(i) *
                                   nuisance_theta.at(j));
            else
              cot = cot * (1 + nuisancedn.at(j)->GetBinContent(i) *
                                   abs(nuisance_theta.at(j)));
          }
        }

        cot = cot + float_height_distance;
        modified_theory_out->SetBinContent(i, cot);
      }

      return modified_theory_out;
    }
    static TH2D *GetStandarCov(TH1D *hist,bool isstd=false)
    {
      if (!hist)
      {
        std::cerr << "Input histogram is null!" << std::endl;
        return nullptr;
      }

      // Get the number of bins from the input histogram
      int nBins = hist->GetNbinsX();

      // Create an array to hold the bin edges
      std::vector<double> binEdges(nBins + 1);

      // Fill the bin edges array
      for (int i = 1; i <= nBins; ++i)
      {
        binEdges[i - 1] = hist->GetXaxis()->GetBinLowEdge(i);
      }
      binEdges[nBins] = hist->GetXaxis()->GetBinUpEdge(nBins);

      // Create a new TH2D histogram with the same binning as the input TH1D histogram
      TH2D *covMatrix = new TH2D("covMatrix", "Covariance Matrix", nBins, binEdges.data(), nBins, binEdges.data());

      // Loop over bins and set diagonal elements to the bin error squared
      for (int i = 1; i <= nBins; ++i)
      {
        double binError = hist->GetBinError(i);
        if(isstd) binError = sqrt(hist->GetBinContent(i));
        covMatrix->SetBinContent(i, i, binError * binError);
      }

      return covMatrix;
    }
  };
}

#endif