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
#include <time.h>
#include <unistd.h>
#include <vector>
#include "TVector3.h"
#include "random"
#include <fastjet/ClusterSequence.hh>
#include <fastjet/PseudoJet.hh>
#include "include/JetsAndDaughters.h"
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
  std::vector<fastjet::PseudoJet> duaughters;
};

struct EventInfo
{
  std::vector<JetsandDaughters> jets;
  bool isdijet = true;
  std::vector<ParticleInfo> particlesinfo;
  std::vector<fastjet::PseudoJet> particles;
  bool has_jpsi = false;
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
  auto tchain = JetsAndDaughters(t);

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
  hists.addHist("jpsi_mc_pt", 40, 0, 200);
  hists.addHist("jpsi_lxy", 40, 0, 400);
  hists.addHist("recojet_pt", 1000, 30, 1030);
  hists.addHist("genjet_pt", 1000, 30, 1030);
  hists.addHist("gendaughter_pt", 500, 0, 100);
  hists.addHist("jpsi_nparticles", 50, 0, 50);
  for (const auto &ptname : ptnames)
  {
    hists.addHist(TString::Format("jpsi_mass%s", ptname.Data()), 40, 2.9, 3.3);
    hists.addHist(TString::Format("jpsi_mc_mass%s", ptname.Data()), 40, 2.9, 3.3);
    hists.addHist(TString::Format("jpsi_coschi%s", ptname.Data()), 40, -1, 1);
    hists.addHist(TString::Format("jpsi_coschi%s_charge", ptname.Data()), 40, -1, 1); 
    hists.addHist(TString::Format("jpsi_mc_coschi%s", ptname.Data()), 40, -1, 1);
    hists.addHist(TString::Format("jpsi_mc_coschi%s_charge", ptname.Data()), 40, -1, 1);
  }
  
  // load mc branches
  std::vector<double> *chadronspt = nullptr;
  std::vector<double> *chadronseta = nullptr;
  std::vector<double> *chadronsphi = nullptr;
  std::vector<double> *chadronse = nullptr;
  std::vector<int> *chadronspdgid = nullptr;
  std::vector<double> *genjetspt = nullptr;
  std::vector<double> *genjetseta = nullptr;
  std::vector<double> *genjetsphi = nullptr;
  std::vector<double> *genjetse = nullptr;
  std::vector<double> *gendaughterspt = nullptr;
  std::vector<double> *gendaughterseta = nullptr;
  std::vector<double> *gendaughtersphi = nullptr;
  std::vector<double> *gendaughterse = nullptr;
  std::vector<int> *gendaughterscharge = nullptr;
  std::vector<int> *gendaughtersid = nullptr;
  double generatorweight;
  if (!input_file.Contains("Run2024"))
  {
    t->SetBranchAddress("GeneratorWeight", &generatorweight);
    // t->SetBranchAddress("cHadrons_Pt_prunedGenParticles", &chadronspt);
    // t->SetBranchAddress("cHadrons_Eta_prunedGenParticles", &chadronseta);
    // t->SetBranchAddress("cHadrons_Phi_prunedGenParticles", &chadronsphi);
    // t->SetBranchAddress("cHadrons_Energy_prunedGenParticles", &chadronse);
    // t->SetBranchAddress("cHadrons_PdgId_prunedGenParticles", &chadronspdgid);
    t->SetBranchAddress("cHadrons_Pt_slimmedGenJetsFlavourInfos", &chadronspt);
    t->SetBranchAddress("cHadrons_Eta_slimmedGenJetsFlavourInfos", &chadronseta);
    t->SetBranchAddress("cHadrons_Phi_slimmedGenJetsFlavourInfos", &chadronsphi);
    t->SetBranchAddress("cHadrons_Energy_slimmedGenJetsFlavourInfos", &chadronse);
    t->SetBranchAddress("cHadrons_PdgId_slimmedGenJetsFlavourInfos", &chadronspdgid);
    t->SetBranchAddress("GenDaughterPt", &gendaughterspt);
    t->SetBranchAddress("GenDaughterEta", &gendaughterseta);
    t->SetBranchAddress("GenDaughterPhi", &gendaughtersphi);
    t->SetBranchAddress("GenDaughterEnergy", &gendaughterse);
    t->SetBranchAddress("GenDaughterCharge", &gendaughterscharge);
    t->SetBranchAddress("GenDaughterJetId", &gendaughtersid);
    t->SetBranchAddress("GenJetPt", &genjetspt);
    t->SetBranchAddress("GenJetEta", &genjetseta);
    t->SetBranchAddress("GenJetPhi", &genjetsphi);
    t->SetBranchAddress("GenJetEnergy", &genjetse);
  }

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
      int triggersize = tchain.TriggerBits->size();
      if (tchain.TriggerBits->at(triggersize - 2) == false || tchain.TriggerBits->at(triggersize - 3) == 0)
         continue;
      // if (tchain.TriggerBits->back() == false)
      //   continue;
      double weight = 1.0;
      if (!input_file.Contains("Run2024"))
        weight = generatorweight;
      for (int i = 0; i < tchain.RecoJetPt->size(); i++)
      // if (tchain.RecoJetPt->size() != 0)
      {
        hists["recojet_pt"]->Fill(tchain.RecoJetPt->at(i), weight);
      }
      
      if (!input_file.Contains("Run2024"))
      {
        for (int i = 0; i < genjetspt->size(); i++)
        // if (genjetspt->size() != 0)
        {
          hists["genjet_pt"]->Fill(genjetspt->at(i), weight);
        }
      }
      for (int i = 0; i < tchain.RecoDiMuonPt->size(); i++)
      {
        if (tchain.RecoDiMuonMass->at(i) > 2.9 && tchain.RecoDiMuonMass->at(i) < 3.3)
        {
          if (tchain.RecoDiMuonCharge->at(i) != 0)
            continue;

          int dau1_index = tchain.RecoDiMuonDaughter1->at(i);
          int dau2_index = tchain.RecoDiMuonDaughter2->at(i);
          
          if (tchain.RecoMuonSoftID->at(dau1_index) == false || tchain.RecoMuonSoftID->at(dau2_index) == false)
            continue;

          bool furthercut = false;
          if (tchain.RecoDiMuonvProb->at(i) > 0.01)
          {
            if (tchain.RecoMuonPt->at(dau1_index) > 2 && tchain.RecoMuonPt->at(dau2_index) > 2)
            {
              if (std::abs(tchain.RecoMuonEta->at(dau1_index)) < 2.4 && std::abs(tchain.RecoMuonEta->at(dau2_index)) < 2.4)
                furthercut = true;
            }
          }

          bool dimuoninjet = false;
          for (int j = 0; j < tchain.RecoJetPt->size(); j++)
          {
            if (tchain.RecoJetPt->at(j) < 30 || std::abs(tchain.RecoJetEta->at(j) > 5.0))
              continue;
            TLorentzVector jet_tlv;
            jet_tlv.SetPtEtaPhiE(tchain.RecoJetPt->at(j), tchain.RecoJetEta->at(j), tchain.RecoJetPhi->at(j), tchain.RecoJetEnergy->at(j));
            TLorentzVector dimuon_tlv;
            dimuon_tlv.SetPtEtaPhiE(tchain.RecoDiMuonPt->at(i), tchain.RecoDiMuonEta->at(i), tchain.RecoDiMuonPhi->at(i), tchain.RecoDiMuonEnergy->at(i));
            double deltaR = jet_tlv.DeltaR(dimuon_tlv);
            if (deltaR < radius)
            {
              dimuoninjet = true;
              break;
            }
          }
          if (dimuoninjet == false)
            continue;

          hists["jpsi_pt"]->Fill(tchain.RecoDiMuonPt->at(i), weight);
          hists["jpsi_lxy"]->Fill(tchain.RecoDiMuonppdlPV->at(i) * 1e4, weight);

          TLorentzVector dimuon_tlv;
          dimuon_tlv.SetPtEtaPhiE(tchain.RecoDiMuonPt->at(i), tchain.RecoDiMuonEta->at(i), tchain.RecoDiMuonPhi->at(i), tchain.RecoDiMuonEnergy->at(i));
          TVector3 boostvec = -dimuon_tlv.BoostVector();
          int jpsiptbin = ptaxis->FindBin(dimuon_tlv.Pt());
          TString ptsuffix = ptnames.at(jpsiptbin);
          for (int j = 0; j < tchain.RecoDaughterPt->size(); j++)
          {
            int index = tchain.RecoDaughterJetId->at(j);
            if (tchain.RecoJetPt->at(index) < 30 || std::abs(tchain.RecoJetEta->at(index) > 5.0))
              continue;
            TLorentzVector daughter_tlv;
            daughter_tlv.SetPtEtaPhiE(tchain.RecoDaughterPt->at(j), tchain.RecoDaughterEta->at(j), tchain.RecoDaughterPhi->at(j), tchain.RecoDaughterEnergy->at(j));
            daughter_tlv.Boost(boostvec);
            double coschi = daughter_tlv.Vect().Dot(dimuon_tlv.Vect()) / (daughter_tlv.Vect().Mag() * dimuon_tlv.Vect().Mag());
            double ec = daughter_tlv.E() / dimuon_tlv.M();
            hists["jpsi_coschi"]->Fill(coschi, ec * weight);
            hists["jpsi_coschi" + ptsuffix]->Fill(coschi, ec * weight);
            if (tchain.RecoDaughterCharge->at(j) != 0)
            {
              hists["jpsi_coschi_charge"]->Fill(coschi, ec * weight);
              hists["jpsi_coschi" + ptsuffix + "_charge"]->Fill(coschi, ec * weight);
            }
          }
          hists["jpsi_mass"]->Fill(tchain.RecoDiMuonMass->at(i), weight);
          hists["jpsi_mass" + ptsuffix]->Fill(tchain.RecoDiMuonMass->at(i), weight);

        }
      }

      if (!input_file.Contains("Run2024"))
      {
        TLorentzVector jpsi_mc_tlv;
        for (int j = 0; j < gendaughterspt->size(); j++)
        {
          TLorentzVector daughter_tlv;
          if (gendaughtersid->at(j) == 0)
            hists["gendaughter_pt"]->Fill(gendaughterspt->at(j), weight);
        }

        bool has_jpsi_mc = false;
        for (int cid = 0; cid < chadronspdgid->size(); cid++)
        {
          if (chadronspdgid->at(cid) == 443)
          {
            has_jpsi_mc = true;
            jpsi_mc_tlv.SetPtEtaPhiE(chadronspt->at(cid), chadronseta->at(cid), chadronsphi->at(cid), chadronse->at(cid));
          }
        }
        if (has_jpsi_mc == false)
          continue;

        bool jpsi_mc_injet = false;
        int jpsi_jetindex = -1;
	for (int genid = 0; genid < genjetspt->size(); genid++)
        {
          if (genjetspt->at(genid) < 30 || std::abs(genjetseta->at(genid)) > 5.0)
            continue;
          TLorentzVector genjet_tlz;
          genjet_tlz.SetPtEtaPhiE(genjetspt->at(genid), genjetseta->at(genid), genjetsphi->at(genid), genjetse->at(genid));
          if (genjet_tlz.DeltaR(jpsi_mc_tlv) < radius)
          {
            jpsi_mc_injet = true;
	    jpsi_jetindex = genid;
            break;
          }
        }
        if (jpsi_mc_injet == false)
          continue;

	int nparticles = 0;
	for (int j = 0; j < gendaughterspt->size(); j++)
	{
	  if (gendaughtersid->at(j) = jpsi_jetindex)
	    nparticles++;
	}
	hists["jpsi_nparticles"]->Fill(nparticles, weight);
        
        hists["jpsi_mc_pt"]->Fill(jpsi_mc_tlv.Pt(), weight);
        int jpsimc_ptbin = ptaxis->FindBin(jpsi_mc_tlv.Pt());
        TString ptmc_suffix = ptnames.at(jpsimc_ptbin);
        TVector3 boostvec_mc = -(jpsi_mc_tlv.BoostVector());
        for (int j = 0; j < gendaughterspt->size(); j++)
        {
          int jetid = gendaughtersid->at(j);
          if (genjetspt->at(jetid) < 30 || std::abs(genjetseta->at(jetid)) > 5.0)
            continue;
          TLorentzVector daughter_tlv;
          daughter_tlv.SetPtEtaPhiE(gendaughterspt->at(j), gendaughterseta->at(j), gendaughtersphi->at(j), gendaughterse->at(j));
          daughter_tlv.Boost(boostvec_mc);
          double coschi = daughter_tlv.Vect().Dot(jpsi_mc_tlv.Vect()) / (daughter_tlv.Vect().Mag() * jpsi_mc_tlv.Vect().Mag()); 
          double ec = daughter_tlv.E() / jpsi_mc_tlv.M();
          hists["jpsi_mc_coschi"]->Fill(coschi, ec * weight);
          hists["jpsi_mc_coschi" + ptmc_suffix]->Fill(coschi, ec * weight);
          if (gendaughterscharge->at(j) != 0)
          {
            hists["jpsi_mc_coschi_charge"]->Fill(coschi, ec * weight);
            hists["jpsi_mc_coschi" + ptmc_suffix + "_charge"]->Fill(coschi, ec * weight);
          }
        }
        hists["jpsi_mc_mass"]->Fill(jpsi_mc_tlv.M(), weight);
        hists["jpsi_mc_mass" + ptmc_suffix]->Fill(jpsi_mc_tlv.M(), weight);
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
