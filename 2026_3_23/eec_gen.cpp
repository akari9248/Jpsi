#include "TAxis.h"
#include "TFile.h"
#include "TChain.h"
#include "TH3D.h"
#include "TString.h"
#include <Math/GenVector/VectorUtil.h>
#include <Math/PtEtaPhiE4D.h>
#include <Math/PxPyPzE4D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TLorentzVector.h>
#include "Math/Vector4D.h"
#include <TSystem.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <time.h>
#include <unistd.h>
#include <vector>
#include "TVector3.h"
#include "random"
#include <fastjet/ClusterSequence.hh>
#include <fastjet/PseudoJet.hh>
#include "include/MCJetsAndDaughters.h"
#include "include/ProgressBar.h"
#include "include/Hists.h"
#include "include/ParticleInfo.h"

struct JetsandDaughters
{
  // jet info
  TLorentzVector jet_tlz;
  void setJet(const fastjet::PseudoJet &j)
  {
    jet_tlz.SetPtEtaPhiE(j.pt(), j.eta(), j.phi(), j.e());
  }

  // jpsi info
  bool has_jpsi = false;
  TLorentzVector jpsi_tlz;

  // daughters info
  std::vector<ParticleInfo> duaughtersinfo;
  std::vector<TLorentzVector> duaughters;
  std::vector<int> daughterindexs;
};

struct JpsiInfo
{
  TLorentzVector jpsi_tlz;
  double vProb = 0.0;
  double lxy = 0.0;
  bool further_cut = false;
};

struct EventInfo
{
  std::vector<JetsandDaughters> jets;
  std::vector<JpsiInfo> jpsis;
  bool has_jpsi = false;
  bool jpsi_furthercut = false;
  double jpsi_lxy = 0.0;
  double jpsi_mass_var = 999.;
  TLorentzVector jpsi_tlz;
};

TLorentzVector createTLVFromPseudoJet(const fastjet::PseudoJet &subjet)
{
  TLorentzVector tlv;
  tlv.SetPtEtaPhiE(subjet.pt(), subjet.eta(), subjet.phi(), subjet.e());
  return tlv;
};

int main(int argc, char *argv[])
{
  TString basefolder = "/data/shuangyuan/2024datasets/Ak8";
  TString input_file = "";
  TString output_path = "";
  double alpha = 1.0;
  double omega = 3.0 - alpha;
  int chunknum = 2;
  int chunkindex = 0;
  double ymax = 1.0;
  int opt;

  // Parse command line arguments
  while ((opt = getopt(argc, argv, "i:o:n:e:")) != -1)
  {
    switch (opt)
    {
    case 'i':
      input_file = optarg;
      break;
    case 'o':
      output_path = optarg;
      break;
    case 'n':
      chunknum = std::atoi(optarg);
      break;
    case 'e':
      chunkindex = std::atoi(optarg);
      break;
    default:
      std::cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_path> -n <chunk_num> -e <chunk_index>" << std::endl;
      return 1;
    }
  }

  // Check if input file is provided
  if (input_file == "")
  {
    std::cerr << "Error: Input file not specified!" << std::endl;
    std::cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_path> -n <chunk_num> -e <chunk_index>" << std::endl;
    return 1;
  }

  // Create output directory
  // gSystem->Exec(TString::Format("mkdir -p %s", output_path.Data()));

  // Create and configure TChain
  TChain *t = new TChain("JetsAndDaughters");
  t->Add(input_file + "/*.root/JetsAndDaughters");
  auto tchain = MCJetsAndDaughters(t);

  int entries = t->GetEntries();
  if (entries == 0)
  {
    std::cout << "Warning: No events found!" << std::endl;
    delete t;
    return 1;
  }

  // Calculate chunk ranges
  int chunksize = std::ceil(static_cast<double>(entries) / chunknum);
  int entrybegin = chunkindex * chunksize;
  int entryend = std::min((chunkindex + 1) * chunksize, entries);

  if (entrybegin >= entries)
  {
    std::cout << "Warning: chunkindex " << chunkindex << " is out of range" << std::endl;
    entrybegin = entries;
    entryend = entries;
  }

  std::cout << "Total entries: " << entries << " Processing range: " << entrybegin << " - " << entryend
            << " (" << (entryend - entrybegin) << " events)" << std::endl;

  int total_events = entryend - entrybegin;
  int events_per_part = std::max(total_events, 1);
  int part_index = 0;
  int events_processed = 0;
  int events_in_current_part = 0;
  int valid_events_in_current_part = 0;

  Hists hists;
  std::vector<TString> ptnames = {"", "_pt0_10", "_pt10_20", "_pt20_30", "_pt30_50", "_pt50_Inf"};
  std::vector<double> ptbins = {0, 10, 20, 30, 50};
  Int_t nBins = ptbins.size() - 1;
  TAxis *ptaxis = new TAxis(nBins, &ptbins[0]);

  hists.addHist("jpsi_pt", 40, 0, 200);
  hists.addHist("jpsi_num", 10, 0, 10);
  hists.addHist("jpsi_lxy", 40, 0, 400);
  hists.addHist("jpsiall_pt", 40, 0, 200);
  hists.addHist("jpsiall_num", 10, 0, 10);
  hists.addHist("jpsiall_lxy", 40, 0, 400);
  hists.addHist("genjet_pt", 1000, 30, 1030);
  for (const auto &ptname : ptnames)
  {
    hists.addHist(TString::Format("jpsi_mass%s", ptname.Data()), 40, 2.9, 3.3);
    hists.addHist(TString::Format("jpsi_coschi%s", ptname.Data()), 40, -1, 1);
    hists.addHist(TString::Format("jpsi_coschi%s_charge", ptname.Data()), 40, -1, 1); 
    hists.addHist(TString::Format("jpsiall_mass%s", ptname.Data()), 40, 2.9, 3.3);
    hists.addHist(TString::Format("jpsiall_coschi%s", ptname.Data()), 40, -1, 1);
    hists.addHist(TString::Format("jpsiall_coschi%s_charge", ptname.Data()), 40, -1, 1);
  }
  
  double generatorweight;
  t->SetBranchAddress("GeneratorWeight", &generatorweight);

  auto SafeWritePart = [&](bool is_final = false) -> bool
  {
    if (valid_events_in_current_part == 0)
    {
      std::cout << "Part " << part_index << " has no valid events, skipping file creation." << std::endl;
      return true;
    }

    TString output_filename = output_path + TString::Format("_Chunk%d_Part%d.root", chunkindex, part_index);
    hists.Write(output_filename);
    std::cout << "Saved part " << part_index << " to " << output_filename
              << " (valid events: " << valid_events_in_current_part
              << ", processed events: " << events_in_current_part;
    if (is_final)
    {
      std::cout << ", final part";
    }
    std::cout << ")" << std::endl;

    return true;
  };

  ProgressBar process(total_events);
  int loop = 1;
  double radius = 0.8;

  for (int k = entrybegin; k < entryend; k++)
  {
    process.update2(loop);
    loop++;
    events_processed++;
    events_in_current_part++;

    bool event_was_written = false;

    try
    {
      t->GetEntry(k);

      double weight = generatorweight;

      EventInfo eventinfo;
      // Jets info
      for (int i = 0; i < tchain.GenJetPt->size(); i++)
      {
        JetsandDaughters jet;
        // pt & eta checking
        if (tchain.GenJetPt->at(i) < 30 || std::abs(tchain.GenJetEta->at(i)) > 5.0)
          continue;
        jet.jet_tlz.SetPtEtaPhiE(tchain.GenJetPt->at(i), tchain.GenJetEta->at(i), tchain.GenJetPhi->at(i), tchain.GenJetEnergy->at(i));
        
        // Find daughters in jet
        std::vector<TLorentzVector> daughters;
        TLorentzVector alldaughter;
        for (int j = 0; j < tchain.GenDaughterPt->size(); j++)
        {
          if (tchain.GenDaughterJetId->at(j) == i)
          {
            TLorentzVector p;
            p.SetPtEtaPhiE(tchain.GenDaughterPt->at(j), tchain.GenDaughterEta->at(j), tchain.GenDaughterPhi->at(j), tchain.GenDaughterEnergy->at(j));
            daughters.push_back(p);
            jet.daughterindexs.push_back(j);
            alldaughter = alldaughter + p;
          }
        }
        double scalefactor = tchain.GenJetPt->at(i) * 1.0 / alldaughter.Pt();
        for(auto &daughter: daughters)
        {
          TLorentzVector temp;
          temp.SetPtEtaPhiE(daughter.Pt() * scalefactor, daughter.Eta(), daughter.Phi(), daughter.E() * scalefactor);
          jet.duaughters.push_back(temp);
        }
        eventinfo.jets.push_back(jet);
      }

      // J/psi info
      for (int i = 0; i < tchain.cHadrons_PdgId_prunedGenParticles->size(); i++)
      {
        if (tchain.cHadrons_PdgId_prunedGenParticles->at(i) == 443)
        {
          JpsiInfo jpsiinfo;
          jpsiinfo.jpsi_tlz.SetPtEtaPhiE(tchain.cHadrons_Pt_prunedGenParticles->at(i), tchain.cHadrons_Eta_prunedGenParticles->at(i), tchain.cHadrons_Phi_prunedGenParticles->at(i), tchain.cHadrons_Energy_prunedGenParticles->at(i));
          
          bool jpsiinjet = false;
          for (int j = 0; j < eventinfo.jets.size(); j++)
          {
            double dr = eventinfo.jets.at(j).jet_tlz.DeltaR(jpsiinfo.jpsi_tlz);
            if (dr < radius)
            {
              jpsiinjet = true;
              break;
            }
          }
          if (jpsiinjet == false)
            continue;
          eventinfo.jpsis.push_back(jpsiinfo);
        }
      }
      hists["jpsi_num"]->Fill(eventinfo.jpsis.size(), weight);
      if (eventinfo.jpsis.size() == 0)
        continue;
      // Find best jpsi
      for (int i = 0; i < eventinfo.jpsis.size(); i++)
      {
        JpsiInfo temp = eventinfo.jpsis.at(i);
        int jpsiptbin = ptaxis->FindBin(temp.jpsi_tlz.Pt());
        TString ptsuffix = ptnames.at(jpsiptbin);
        hists["jpsiall_pt"]->Fill(temp.jpsi_tlz.Pt(), weight);
        hists["jpsiall_lxy"]->Fill(temp.lxy, weight);
        hists["jpsiall_mass"]->Fill(temp.jpsi_tlz.M(), weight);
        hists["jpsiall_mass" + ptsuffix]->Fill(temp.jpsi_tlz.M(), weight);

        TVector3 boostvector = - (temp.jpsi_tlz.BoostVector());
        for (int m = 0; m < eventinfo.jets.size(); m++)
        {
          for (int j = 0; j < eventinfo.jets.at(m).duaughters.size(); j++)
          {
            TLorentzVector dau = eventinfo.jets.at(m).duaughters.at(j);
            dau.Boost(boostvector);
            double coschi = dau.Vect().Dot(temp.jpsi_tlz.Vect()) * 1.0 / dau.Vect().Mag() / temp.jpsi_tlz.Vect().Mag();
            double ec = dau.E() / temp.jpsi_tlz.M();
	    hists["jpsiall_coschi"]->Fill(coschi, ec * weight);
            hists["jpsiall_coschi" + ptsuffix]->Fill(coschi, ec * weight);
            int index = eventinfo.jets.at(m).daughterindexs.at(j);
            if (tchain.GenDaughterCharge->at(index) != 0)
            {
              hists["jpsiall_coschi_charge"]->Fill(coschi, ec * weight);
              hists["jpsiall_coschi" + ptsuffix + "_charge"]->Fill(coschi, ec * weight);
            }
          }
        }

        if (std::abs(temp.jpsi_tlz.M() - 3.096) < eventinfo.jpsi_mass_var)
        {
          eventinfo.jpsi_mass_var = std::abs(temp.jpsi_tlz.M() - 3.096);
          eventinfo.jpsi_furthercut = temp.further_cut;
          eventinfo.jpsi_tlz = temp.jpsi_tlz;
          eventinfo.jpsi_lxy = temp.lxy;
        }
      }

      // Best jpsi calculation
      hists["jpsi_pt"]->Fill(eventinfo.jpsi_tlz.Pt(), weight);
      hists["jpsi_lxy"]->Fill(eventinfo.jpsi_lxy, weight);

      // jpsi pt bin
      int jpsiptbin = ptaxis->FindBin(eventinfo.jpsi_tlz.Pt());
      TString ptsuffix = ptnames.at(jpsiptbin);
      hists["jpsi_mass"]->Fill(eventinfo.jpsi_tlz.M(), weight);
      hists["jpsi_mass" + ptsuffix]->Fill(eventinfo.jpsi_tlz.M(), weight);

      // jpsi coschi
      TVector3 boostvector = - (eventinfo.jpsi_tlz.BoostVector());
      for (int i = 0; i < eventinfo.jets.size(); i++)
      {
        hists["genjet_pt"]->Fill(eventinfo.jets.at(i).jet_tlz.Pt(), weight);
        for (int j = 0; j < eventinfo.jets.at(i).duaughters.size(); j++)
        {
          TLorentzVector dau = eventinfo.jets.at(i).duaughters.at(j);
          dau.Boost(boostvector);
          double coschi = dau.Vect().Dot(eventinfo.jpsi_tlz.Vect()) * 1.0 / dau.Vect().Mag() / eventinfo.jpsi_tlz.Vect().Mag();
          double ec = dau.E() / eventinfo.jpsi_tlz.M();
	  hists["jpsi_coschi"]->Fill(coschi, ec * weight);
          hists["jpsi_coschi" + ptsuffix]->Fill(coschi, ec * weight);
          int index = eventinfo.jets.at(i).daughterindexs.at(j);
          {
            hists["jpsi_coschi_charge"]->Fill(coschi, ec * weight);
            hists["jpsi_coschi" + ptsuffix + "_charge"]->Fill(coschi, ec * weight);
          }
        }
      }

      valid_events_in_current_part++;
    }

    catch (const std::exception &e)
    {
      std::cout << "Exception processing event " << k << ": " << e.what() << ", skipping..." << std::endl;
      continue;
    }
    catch (...)
    {
      std::cout << "Unknown exception processing event " << k << ", skipping..." << std::endl;
      continue;
    }

    if (k == entryend - 1)
    {
      std::cout << "Reached the last event: k = " << k << std::endl;
    }

    bool should_write_part = false;
    if (events_in_current_part >= events_per_part)
    {
      should_write_part = true;
    }
    else if (k == entryend - 1)
    {
      should_write_part = true;
    }

    if (should_write_part)
    {
      if (SafeWritePart(k == entryend - 1))
      {
        part_index++;
        events_in_current_part = 0;
        valid_events_in_current_part = 0;
      }
    }
  }
  if (events_in_current_part > 0)
  {
    if (SafeWritePart(true))
    {
      part_index++;
    }
  }

  delete t;

  std::cout << std::endl;
  std::cout << "Processing completed successfully!" << std::endl;
  std::cout << "Total events processed: " << events_processed << std::endl;
  std::cout << "Total parts created: " << part_index << std::endl;

  return 0;
}
