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
  int jetmatchindex = -1;

  // daughters info
  std::vector<ParticleInfo> daughtersinfo;
  std::vector<TLorentzVector> daughters;
  std::vector<int> daughterindexs;
  std::vector<int> daughtermatchindexs;
};

struct EventInfo
{
  std::vector<JetsandDaughters> jets;
  TLorentzVector jpsi_tlz;
  bool jpsi_injet = false;
  bool jpsi_matched = false;
  std::vector<TLorentzVector> jpsicandidates_tlz;
};

TLorentzVector createTLVFromPseudoJet(const fastjet::PseudoJet &subjet)
{
  TLorentzVector tlv;
  tlv.SetPtEtaPhiE(subjet.pt(), subjet.eta(), subjet.phi(), subjet.e());
  return tlv;
};

bool isSameParticle(const TLorentzVector& v1, const TLorentzVector& v2, double drCut = 1e-4, double relEut = 1e-4) 
{
  if (v1.DeltaR(v2) > drCut) return false;
  double relDiffE = std::abs(v1.E() - v2.E()) / (v1.E() + 1e-9);
  if (relDiffE > relEut) return false;

  return true;
}

void processJets(const std::vector<double>& jetPt, const std::vector<double>& jetEta,
                 const std::vector<double>& jetPhi, const std::vector<double>& jetEnergy,
                 const std::vector<double>& daughterPt, const std::vector<double>& daughterEta,
                 const std::vector<double>& daughterPhi, const std::vector<double>& daughterEnergy, const std::vector<int>& daughterJetId,
                 std::vector<JetsandDaughters>& jetsContainer, const std::vector<bool> &passhotzone = {}, std::string leveltype = "Gen", double ptCut = 30.0, double etaCut = 5.0)
{
  for (int i = 0; i < jetPt.size(); ++i)
  {
    if (jetPt[i] < ptCut || std::abs(jetEta[i]) > etaCut)
      continue;
    if (leveltype == "Reco" && !passhotzone[i])
      continue;
    JetsandDaughters jet;
    jet.jet_tlz.SetPtEtaPhiE(jetPt[i], jetEta[i], jetPhi[i], jetEnergy[i]);
    std::vector<TLorentzVector> daughters;
    TLorentzVector alldaughter;
    for (int j = 0; j < daughterPt.size(); ++j)
    {
      if (daughterJetId[j] == i)
      {
        TLorentzVector p;
        p.SetPtEtaPhiE(daughterPt[j], daughterEta[j], daughterPhi[j], daughterEnergy[j]);
        daughters.push_back(p);
        jet.daughterindexs.push_back(j);
        alldaughter += p;
      }
    }

    if (alldaughter.Pt() > 0) 
    {
      double scalefactor = jetPt[i] / alldaughter.Pt();
      for (auto& daughter : daughters)
      {
        TLorentzVector temp;
        temp.SetPtEtaPhiE(daughter.Pt() * scalefactor,
                          daughter.Eta(),
                          daughter.Phi(),
                          daughter.E() * scalefactor);
        jet.daughters.push_back(temp);
      }
    }
    jetsContainer.push_back(jet);
  }
}

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
  t->Add(input_file + "/Chunk*.root/JetsAndDaughters");
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
  std::vector<double> ptbins = {0, 8, 12, 16, 20, 30, 50, 100, 200};
  Int_t nBins = ptbins.size() - 1;
  TAxis *ptaxis = new TAxis(nBins, &ptbins[0]);
  std::vector<TString> ptnames = {""};
  for (int i = 0; i < ptbins.size(); i++)
  {
    TString tempname;
    if (i != ptbins.size() - 1)
      tempname = TString::Format("_jpsipt_%0.f_%0.f", ptbins.at(i), ptbins.at(i + 1));
    else
      tempname = "_jpsipt_200_Inf";
    ptnames.push_back(tempname);
  }
  std::vector<TString> prefix_levels = {"Gen", "Reco"};
  std::vector<TString> suffix_particles = {"all", "neutral", "charge"};
  std::vector<TString> suffix_matches = {"all"};
  
  for (const auto &ptname: ptnames)
  {
    for (const auto &prefix_level: prefix_levels)
    {
      hists.addHist(prefix_level + "_jpsi_pt" + ptname, 50, 0, 200);
      // hists.addHist(prefix_level + "_jpsi_lxy" + ptname, 40, 0, 400);
      hists.addHist(prefix_level + "_jpsi_mass" + ptname, 40, 2.9, 3.3);
      for (const auto &suffix_particle: suffix_particles)
      {
        for (const auto &suffix_match: suffix_matches)
        {
          hists.addHist(prefix_level + "_coschi_alljet_" + suffix_particle + "_" + suffix_match + ptname, 20, -1, 1);
        }
      }
    }
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

  TString SampleType;
  if (input_file.Contains("Run2024"))
    SampleType = "data";
  else
    SampleType = "mc";

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
      // trigger selection
      int triggersize = tchain.TriggerBits->size();
      // if (!(tchain.TriggerBits->at(triggersize - 2) == false && tchain.TriggerBits->at(triggersize - 3) == false))
      // if (tchain.TriggerBits->at(triggersize - 2) == false && tchain.TriggerBits->at(triggersize - 3) == false)
      // if (tchain.TriggerBits->at(triggersize - 4) == false)
      //    continue;       
      // noise filter
      bool passnoisefilter = true;
      for (int i = 0; i < tchain.MetFilterBits->size(); i++)
      {   
        if (tchain.MetFilterBits->at(i) == false)
        {
          passnoisefilter = false;
          break;  
        }                 
      }                                                                                          
      if (passnoisefilter == false)
        continue; 
      // generatorweight (MC only)
      double weight = 1.0;
      if (SampleType == "mc")
        weight = tchain.GeneratorWeight;

      std::vector<EventInfo> eventsinfo;
      if (SampleType == "data")
        eventsinfo.resize(1);
      else
        eventsinfo.resize(2);

      // Jets info
      processJets(*tchain.RecoJetPt, *tchain.RecoJetEta, *tchain.RecoJetPhi, *tchain.RecoJetEnergy, 
                  *tchain.RecoDaughterPt, *tchain.RecoDaughterEta, *tchain.RecoDaughterPhi, *tchain.RecoDaughterEnergy, *tchain.RecoDaughterJetId, 
                  eventsinfo.back().jets, *tchain.RecoJetPassHotZone, "Reco");
      if (SampleType == "mc")
      {
        processJets(*tchain.GenJetPt, *tchain.GenJetEta, *tchain.GenJetPhi, *tchain.GenJetEnergy, 
                    *tchain.GenDaughterPt, *tchain.GenDaughterEta, *tchain.GenDaughterPhi, *tchain.GenDaughterEnergy, *tchain.GenDaughterJetId, 
                    eventsinfo.at(0).jets);
      }

      // jpsi info
      // Reco
      double dimuonmass_var = 999.;
      std::vector<TLorentzVector> recomuons;
      for (int i = 0; i < tchain.RecoDiMuonMass->size(); i++)
      {
        if (tchain.RecoDiMuonMass->at(i) < 2.9 || tchain.RecoDiMuonMass->at(i) > 3.3)
          continue;

        int daughter1 = tchain.RecoDiMuonDaughter1->at(i);
        int daughter2 = tchain.RecoDiMuonDaughter2->at(i);
        if (tchain.RecoMuonSoftID->at(daughter1) == false || tchain.RecoMuonSoftID->at(daughter2) == false)
          continue;
        
        TLorentzVector muon1, muon2;
        muon1.SetPtEtaPhiE(tchain.RecoMuonPt->at(daughter1), tchain.RecoMuonPhi->at(daughter1), tchain.RecoMuonEta->at(daughter1), tchain.RecoMuonEnergy->at(daughter1));
        muon2.SetPtEtaPhiE(tchain.RecoMuonPt->at(daughter2), tchain.RecoMuonPhi->at(daughter2), tchain.RecoMuonEta->at(daughter2), tchain.RecoMuonEnergy->at(daughter2));
        /*
        if (tchain.RecoDiMuonvProb->at(i) < 0.01)
          continue;
        */
        if (tchain.RecoMuonPt->at(daughter1) < 5 || tchain.RecoMuonPt->at(daughter2) < 5)
          continue;
        if (muon1.DeltaR(muon2) > 2)
          continue;
        if (std::abs(tchain.RecoMuonEta->at(daughter1)) > 2.4 || std::abs(tchain.RecoMuonEta->at(daughter2)) > 2.4)
          continue;
        // if (std::abs(tchain.RecoMuondzPV->at(daughter1) - tchain.RecoMuondzPV->at(daughter2)) > 25)
        //   continue;

        TLorentzVector jpsi_temp;
        jpsi_temp.SetPtEtaPhiE(tchain.RecoDiMuonPt->at(i), tchain.RecoDiMuonEta->at(i), tchain.RecoDiMuonPhi->at(i), tchain.RecoDiMuonEnergy->at(i));
        eventsinfo.back().jpsicandidates_tlz.push_back(jpsi_temp);
        if (std::abs(tchain.RecoDiMuonMass->at(i) - 3.1) < dimuonmass_var)
        {
          dimuonmass_var = std::abs(tchain.RecoDiMuonMass->at(i) - 3.1);
          eventsinfo.back().jpsi_tlz = jpsi_temp;
          recomuons.clear();
          if (tchain.RecoMuonCharge->at(daughter1) > 0)
          {
            recomuons.push_back(muon1);
            recomuons.push_back(muon2);
          }
          else
          {
            recomuons.push_back(muon2);
            recomuons.push_back(muon1);
          }
        }
      }
      // Gen
      std::vector<std::vector<TLorentzVector>> genmuons_candidate;
      std::vector<TLorentzVector> genmuons;
      if (SampleType == "mc")
      {
        for (int i = 0; i < tchain.cHadrons_Pt_slimmedGenJetsFlavourInfos->size(); i++)
        {
          if (std::abs(tchain.cHadrons_PdgId_slimmedGenJetsFlavourInfos->at(i)) == 443)
          {
            TLorentzVector jpsi_temp;
            jpsi_temp.SetPtEtaPhiE(tchain.cHadrons_Pt_slimmedGenJetsFlavourInfos->at(i), tchain.cHadrons_Eta_slimmedGenJetsFlavourInfos->at(i), 
                                   tchain.cHadrons_Phi_slimmedGenJetsFlavourInfos->at(i), tchain.cHadrons_Energy_slimmedGenJetsFlavourInfos->at(i));
            eventsinfo.at(0).jpsicandidates_tlz.push_back(jpsi_temp);
            eventsinfo.at(0).jpsi_tlz = jpsi_temp;
	  }
        }

        /*
        for (int i = 0; i < tchain.GenMuonPt->size(); i++)
        {
          for (int j = i + 1; j < tchain.GenMuonPt->size(); j++)
          {
            if ((tchain.GenMuonCharge->at(i) + tchain.GenMuonCharge->at(j)) != 0)
              continue;
            TLorentzVector muon1, muon2;
            muon1.SetPtEtaPhiE(tchain.GenMuonPt->at(i), tchain.GenMuonEta->at(i), tchain.GenMuonPhi->at(i), tchain.GenMuonEnergy->at(i));
            muon2.SetPtEtaPhiE(tchain.GenMuonPt->at(j), tchain.GenMuonEta->at(j), tchain.GenMuonPhi->at(j), tchain.GenMuonEnergy->at(j));
            TLorentzVector dimuon = muon1 + muon2;
            if (dimuon.M() < 2.9 || dimuon.M() > 3.3)
              continue;
            eventsinfo.at(0).jpsicandidates_tlz.push_back(dimuon);
            std::vector<TLorentzVector> genmuons_temp;
            if (tchain.GenMuonCharge->at(i) > 0)
            {
              genmuons_temp.push_back(muon1);
              genmuons_temp.push_back(muon2);
            }
            else
            {
              genmuons_temp.push_back(muon2);
              genmuons_temp.push_back(muon1);
            }
            genmuons_candidate.push_back(genmuons_temp);
          }
        }
        */
      }

      if (eventsinfo.back().jpsicandidates_tlz.size() == 0)
        continue;

      if (SampleType == "mc")
      {
        if (eventsinfo.at(0).jpsicandidates_tlz.size() == 0)
	  continue;
      }

      // jpsi pt bin
      int jpsiptbin = ptaxis->FindBin(eventsinfo.back().jpsi_tlz.Pt());
      TString ptsuffix = ptnames.at(jpsiptbin);
      hists["Reco_jpsi_mass"]->Fill(eventsinfo.back().jpsi_tlz.M(), weight);
      hists["Reco_jpsi_mass" + ptsuffix]->Fill(eventsinfo.back().jpsi_tlz.M(), weight);
      hists["Reco_jpsi_pt"]->Fill(eventsinfo.back().jpsi_tlz.Pt(), weight);
      hists["Reco_jpsi_pt" + ptsuffix]->Fill(eventsinfo.back().jpsi_tlz.Pt(), weight);

      // jpsi coschi
      TVector3 boostvector = - (eventsinfo.back().jpsi_tlz.BoostVector());
      for (int i = 0; i < eventsinfo.back().jets.size(); i++)
      {
        for (int j = 0; j < eventsinfo.back().jets.at(i).daughters.size(); j++)
        {
          TString particletype = "";
          if (tchain.RecoDaughterCharge->at(eventsinfo.back().jets.at(i).daughterindexs.at(j)) == 0)
            particletype = "neutral";
          else
            particletype = "charge";

          TLorentzVector dau = eventsinfo.back().jets.at(i).daughters.at(j);
          // remove muon from j/psi
          if (isSameParticle(recomuons.at(0), dau) || isSameParticle(recomuons.at(1), dau))
            continue;
          dau.Boost(boostvector);
          double coschi = dau.Vect().Dot(eventsinfo.back().jpsi_tlz.Vect()) * 1.0 / dau.Vect().Mag() / eventsinfo.back().jpsi_tlz.Vect().Mag();
          double ec = dau.E() / eventsinfo.back().jpsi_tlz.M();
          
          hists["Reco_coschi_alljet_all_all"]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_all_all" + ptsuffix]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_" + particletype + "_all"]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_" + particletype + "_all" + ptsuffix]->Fill(coschi, ec * weight);
        }
      }

      if (SampleType == "mc")
      {
        // if (eventsinfo.back().jpsi_matched)
        if (eventsinfo.at(0).jpsicandidates_tlz.size() != 0)
        {
          int gen_jpsiptbin = ptaxis->FindBin(eventsinfo.at(0).jpsi_tlz.Pt());
          TString gen_ptsuffix = ptnames.at(gen_jpsiptbin);
          hists["Gen_jpsi_mass"]->Fill(eventsinfo.at(0).jpsi_tlz.M(), weight);
          hists["Gen_jpsi_mass" + gen_ptsuffix]->Fill(eventsinfo.at(0).jpsi_tlz.M(), weight);
          hists["Gen_jpsi_pt"]->Fill(eventsinfo.at(0).jpsi_tlz.Pt(), weight);
          hists["Gen_jpsi_pt" + gen_ptsuffix]->Fill(eventsinfo.at(0).jpsi_tlz.Pt(), weight);
          TVector3 gen_boostvector = -(eventsinfo.at(0).jpsi_tlz.BoostVector());
          for (int i = 0; i < eventsinfo.at(0).jets.size(); i++)
          {
            for (int j = 0; j < eventsinfo.at(0).jets.at(i).daughters.size(); j++)
            {
              TString gen_particletype = "";
              if (tchain.GenDaughterCharge->at(eventsinfo.at(0).jets.at(i).daughterindexs.at(j)) == 0)
                gen_particletype = "neutral";
              else
                gen_particletype = "charge";
              TLorentzVector gen_dau = eventsinfo.at(0).jets.at(i).daughters.at(j);
              // if (isSameParticle(genmuons.at(0), gen_dau) || isSameParticle(genmuons.at(1), gen_dau))
              //   continue;
              gen_dau.Boost(gen_boostvector);
              double gen_coschi = gen_dau.Vect().Dot(eventsinfo.at(0).jpsi_tlz.Vect()) * 1.0 / gen_dau.Vect().Mag() / eventsinfo.at(0).jpsi_tlz.Vect().Mag();
              double gen_ec = gen_dau.E() / eventsinfo.at(0).jpsi_tlz.M();
              
	      hists["Gen_coschi_alljet_all_all"]->Fill(gen_coschi, gen_ec * weight);
              hists["Gen_coschi_alljet_all_all" + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
              hists["Gen_coschi_alljet_" + gen_particletype + "_all"]->Fill(gen_coschi, gen_ec * weight);
              hists["Gen_coschi_alljet_" + gen_particletype + "_all" + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
            }
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
