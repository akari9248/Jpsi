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
  std::vector<TString> suffix_matches = {"all", "matched", "unmatched"};
  
  for (const auto &ptname: ptnames)
  {
    for (const auto &prefix_level: prefix_levels)
    {
      hists.addHist(prefix_level + "_jpsi_pt" + ptname, 40, 0, 200);
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
  hists.addHist("genreco_jpsi_dr", 40, 0, 0.8);
  hists.addHist("genreco_jpsi_dpt", 40, 0, 200);

  std::vector<TH2D *> hists2ds;
  TH2D *migration_matrix_jpsi_pt = new TH2D("migration_matrix_jpsi_pt", "migration_matrix_jpsi_pt", 40, 0, 200, 40, 0, 200);
  hists2ds.push_back(migration_matrix_jpsi_pt);

  for (const auto &suffix_particle: suffix_particles)
  {
    hists.addHist("genreco_" + suffix_particle + "_particles_dr", 1000, 0, 0.8);
    hists.addHist("genreco_" + suffix_particle + "_particles_dpt", 40, 0, 200);
    hists2ds.push_back(new TH2D("migration_matrix_" + suffix_particle + "_particles_pt", "migration_matrix_" + suffix_particle + "_particles_pt", 40, 0, 200, 40, 0, 200));
    hists2ds.push_back(new TH2D("migration_matrix_" + suffix_particle + "_particles_coschi", "migration_matrix_" + suffix_particle + "_particles_coschi", 20, -1, 1, 20, -1, 1));
    hists2ds.push_back(new TH2D("migration_matrix_" + suffix_particle + "_particles_coschi_genjpsi", "migration_matrix_" + suffix_particle + "_particles_coschi_genjpsi", 20, -1, 1, 20, -1, 1));
    hists2ds.push_back(new TH2D("migration_matrix_" + suffix_particle + "_particles_coschi_recojpsi", "migration_matrix_" + suffix_particle + "_particles_coschi_recojpsi", 20, -1, 1, 20, -1, 1));
  }

  auto SafeWritePart = [&](bool is_final = false) -> bool
  {
    if (valid_events_in_current_part == 0)
    {
      std::cout << "Part " << part_index << " has no valid events, skipping file creation." << std::endl;
      return true;
    }

    TString output_filename = output_path + TString::Format("_Chunk%d_Part%d.root", chunkindex, part_index);
    hists.Write(output_filename, hists2ds);
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
      if (tchain.TriggerBits->at(triggersize - 2) == false || tchain.TriggerBits->at(triggersize - 3) == 0)
        continue;        
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
      for (int i = 0; i < tchain.RecoDiMuonMass->size(); i++)
      {
        if (tchain.RecoDiMuonMass->at(i) < 2.9 || tchain.RecoDiMuonMass->at(i) > 3.3)
          continue;

        int daughter1 = tchain.RecoDiMuonDaughter1->at(i);
        int daughter2 = tchain.RecoDiMuonDaughter2->at(i);
        if (tchain.RecoMuonSoftID->at(daughter1) == false || tchain.RecoMuonSoftID->at(daughter2) == false)
          continue;
        
        /* TBD
        if (tchain.RecoDiMuonvProb->at(i) < 0.01)
          continue;
        if (tchain.RecoMuonPt->at(daughter1) < 2 || tchain.RecoMuonPt->at(daughter2) < 2)
          continue;
        if (std::abs(tchain.RecoMuonEta->at(daughter1)) > 2.4 && std::abs(tchain.RecoMuonEta->at(daughter2)) > 2.4)
          continue;
        if (std::abs(tchain.RecoMuondzPV->at(daughter1) - tchain.RecoMuondzPV->at(daughter2)) > 25)
          continue;
        */
        
        TLorentzVector jpsi_temp;
        jpsi_temp.SetPtEtaPhiE(tchain.RecoDiMuonPt->at(i), tchain.RecoDiMuonEta->at(i), tchain.RecoDiMuonPhi->at(i), tchain.RecoDiMuonEnergy->at(i));
        eventsinfo.back().jpsicandidates_tlz.push_back(jpsi_temp);
        if (std::abs(tchain.RecoDiMuonMass->at(i) - 3.1) < dimuonmass_var)
        {
          dimuonmass_var = std::abs(tchain.RecoDiMuonMass->at(i) - 3.1);
          eventsinfo.back().jpsi_tlz = jpsi_temp;
        }
      }
      // Gen
      if (SampleType == "mc")
      {
        for (int i = 0; i < tchain.cHadrons_Pt_slimmedGenJetsFlavourInfos->size(); i++)
        {
          if (std::abs(tchain.cHadrons_PdgId_slimmedGenJetsFlavourInfos->at(i)) != 443)
          {
            TLorentzVector jpsi_temp;
            jpsi_temp.SetPtEtaPhiE(tchain.cHadrons_Pt_slimmedGenJetsFlavourInfos->at(i), tchain.cHadrons_Eta_slimmedGenJetsFlavourInfos->at(i), 
                                   tchain.cHadrons_Phi_slimmedGenJetsFlavourInfos->at(i), tchain.cHadrons_Energy_slimmedGenJetsFlavourInfos->at(i));
            eventsinfo.at(0).jpsicandidates_tlz.push_back(jpsi_temp);
          }
        }
      }

      // Matching
      // Reco J/psi inside a jet
      for (int i = 0; i < eventsinfo.back().jets.size(); i++)
      {
        double dr = eventsinfo.back().jets.at(i).jet_tlz.DeltaR(eventsinfo.back().jpsi_tlz);
        if (dr < 0.8)
        {
          eventsinfo.back().jpsi_injet = true;
        }
      }
      if (!eventsinfo.back().jpsi_injet)
        continue;
      // MC: gen-reco J/psi, jet, daughter matching
      if (SampleType == "mc")
      {
        // gen-reco J/psi
        double drmin = 999.;
        for (int i = 0; i < eventsinfo.at(0).jpsicandidates_tlz.size(); i++)
        {
          double dr = eventsinfo.at(0).jpsicandidates_tlz.at(i).DeltaR(eventsinfo.back().jpsi_tlz);
          if (dr < drmin)
          {
            drmin = dr;
            eventsinfo.at(0).jpsi_tlz = eventsinfo.at(0).jpsicandidates_tlz.at(i);
            eventsinfo.back().jpsi_matched = true;
          }
        }
        // gen-reco jet
        std::vector<int> recoMatched(eventsinfo.back().jets.size(), -1);
        std::vector<int> genMatched(eventsinfo.at(0).jets.size(), -1);
        for (int i = 0; i < eventsinfo.back().jets.size(); i++)
        {
          double bestDr = 0.4;
          double bestDpt = 9999999.;
          int bestMatch = -1;
          for (int j = 0; j < eventsinfo.at(0).jets.size(); j++)
          {
            if (genMatched[j] != -1) continue;
            double dr = eventsinfo.back().jets[i].jet_tlz.DeltaR(eventsinfo.at(0).jets[j].jet_tlz);
            if (dr > bestDr) continue;
            double dpt = std::abs(eventsinfo.back().jets[i].jet_tlz.Pt() - eventsinfo.at(0).jets[j].jet_tlz.Pt());
            if (dpt < bestDpt) 
            {
              bestDpt = dpt;
              bestMatch = j;
            }
          }
          if (bestMatch != -1) 
          {
            recoMatched[i] = bestMatch;
            genMatched[bestMatch] = i;
          }
        }
        for (int i = 0; i < eventsinfo.at(0).jets.size(); i++)
          eventsinfo.at(0).jets[i].jetmatchindex = genMatched[i];
        for (int i = 0; i < eventsinfo.back().jets.size(); i++) 
          eventsinfo.back().jets[i].jetmatchindex = recoMatched[i];
        // gen-reco daughter
        for (int ireco = 0; ireco < eventsinfo.back().jets.size(); ireco++)
        {
          int igen = recoMatched[ireco];
          auto& recoJet = eventsinfo.back().jets[ireco];
          recoJet.daughtermatchindexs.resize(recoJet.daughters.size(), -1);
          if (igen == -1) continue;
          auto& genJet = eventsinfo.at(0).jets[igen];
          genJet.daughtermatchindexs.resize(genJet.daughters.size(), -1);

          int nRecoDaughters = recoJet.daughters.size();
          int nGenDaughters = genJet.daughters.size();
          std::vector<int> recoIdx(nRecoDaughters);
          std::vector<int> genIdx(nGenDaughters);
          std::iota(recoIdx.begin(), recoIdx.end(), 0);
          std::iota(genIdx.begin(), genIdx.end(), 0);
          std::sort(recoIdx.begin(), recoIdx.end(),
              [&](int a, int b) {
                  return recoJet.daughters[a].Pt() > recoJet.daughters[b].Pt();
              });
          std::sort(genIdx.begin(), genIdx.end(),
              [&](int a, int b) {
                  return genJet.daughters[a].Pt() > genJet.daughters[b].Pt();
              });

          double drCut = sqrt(0.64 / 10.0 / sqrt(nGenDaughters));
          std::vector<bool> genDaughterMatched(nGenDaughters, false);
          for (int recoOrder = 0; recoOrder < nRecoDaughters; recoOrder++)
          {
            int iReco = recoIdx[recoOrder];
            const auto& recoDaughter = recoJet.daughters[iReco];
            int bestGenIdx = -1;
            double dptmin = 9999999.;
            for (int genOrder = 0; genOrder < nGenDaughters; genOrder++)
            {
              int iGen = genIdx[genOrder];
              if (tchain.GenDaughterCharge->at(genJet.daughterindexs[genOrder]) == 0 && tchain.RecoDaughterCharge->at(recoJet.daughterindexs[recoOrder]) != 0) continue;
              if (tchain.GenDaughterCharge->at(genJet.daughterindexs[genOrder]) != 0 && tchain.RecoDaughterCharge->at(recoJet.daughterindexs[recoOrder]) == 0) continue;
              if (genDaughterMatched[iGen]) continue;
              const auto& genDaughter = genJet.daughters[iGen];
              double dr = recoDaughter.DeltaR(genDaughter);
              if (dr > drCut) continue;
              double dpt = std::abs(genDaughter.Pt() - recoDaughter.Pt());
              if (dpt < dptmin)
              {
                dptmin = dpt;
                bestGenIdx = genOrder;
              }
            }
            if (bestGenIdx != -1)
            {
              recoJet.daughtermatchindexs[iReco] = bestGenIdx;
              genJet.daughtermatchindexs[bestGenIdx] = iReco;
              genDaughterMatched[bestGenIdx] = true;
            }
          }
        }

        // matching info
        // J/psi
        if (eventsinfo.back().jpsi_matched)
        {
          double jpsidr = eventsinfo.back().jpsi_tlz.DeltaR(eventsinfo.at(0).jpsi_tlz);
          double jpsidpt = std::abs(eventsinfo.back().jpsi_tlz.Pt() - eventsinfo.at(0).jpsi_tlz.Pt());
          hists["genreco_jpsi_dr"]->Fill(jpsidr, weight);
          hists["genreco_jpsi_dpt"]->Fill(jpsidpt, weight);
          hists2ds.at(0)->Fill(eventsinfo.at(0).jpsi_tlz.Pt(), eventsinfo.back().jpsi_tlz.Pt(), weight);
        }
        // particles
        for (int i = 0; i < eventsinfo.back().jets.size(); i++)
        {
          JetsandDaughters recojet = eventsinfo.back().jets.at(i);
          if (recojet.jetmatchindex == -1)
            continue;
          for (int j = 0; j < recojet.daughters.size(); j++)
          {
            int matchindex = recojet.daughtermatchindexs.at(j);
            if (matchindex == -1)
              continue;
            JetsandDaughters genjet = eventsinfo.at(0).jets.at(recojet.jetmatchindex);
            TString particletype = "";
            if (tchain.RecoDaughterCharge->at(recojet.daughterindexs.at(j)) != 0)
              particletype = "charge";
            else
              particletype = "neutral";
            double particledr = recojet.daughters.at(j).DeltaR(genjet.daughters.at(matchindex));
            double particledpt = std::abs(recojet.daughters.at(j).Pt() - genjet.daughters.at(matchindex).Pt());
            hists["genreco_all_particles_dr"]->Fill(particledr, weight);
            hists["genreco_all_particles_dpt"]->Fill(particledpt, weight);
            hists["genreco_" + particletype + "_particles_dr"]->Fill(particledr, weight);
            hists["genreco_" + particletype + "_particles_dpt"]->Fill(particledpt, weight);   
            hists2ds.at(1)->Fill(genjet.daughters.at(matchindex).Pt(), recojet.daughters.at(j).Pt(), weight);
            if (particletype == "neutral")
              hists2ds.at(5)->Fill(genjet.daughters.at(matchindex).Pt(), recojet.daughters.at(j).Pt(), weight);
            else
              hists2ds.at(9)->Fill(genjet.daughters.at(matchindex).Pt(), recojet.daughters.at(j).Pt(), weight);
          }
        }
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
          dau.Boost(boostvector);
          double coschi = dau.Vect().Dot(eventsinfo.back().jpsi_tlz.Vect()) * 1.0 / dau.Vect().Mag() / eventsinfo.back().jpsi_tlz.Vect().Mag();
          double ec = dau.E() / eventsinfo.back().jpsi_tlz.M();
          
          hists["Reco_coschi_alljet_all_all"]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_all_all" + ptsuffix]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_" + particletype + "_all"]->Fill(coschi, ec * weight);
          hists["Reco_coschi_alljet_" + particletype + "_all" + ptsuffix]->Fill(coschi, ec * weight);

          if (SampleType == "mc")
          {
            TString matchtype = "";
            if (eventsinfo.back().jets.at(i).daughtermatchindexs.at(j) >= 0)
              matchtype = "matched";
            else
              matchtype = "unmatched";
            hists["Reco_coschi_alljet_all_" + matchtype]->Fill(coschi, ec * weight);
            hists["Reco_coschi_alljet_all_" + matchtype + ptsuffix]->Fill(coschi, ec * weight);
            hists["Reco_coschi_alljet_" + particletype + "_" + matchtype]->Fill(coschi, ec * weight);
            hists["Reco_coschi_alljet_" + particletype + "_" + matchtype + ptsuffix]->Fill(coschi, ec * weight);

            int jetmatchindex = eventsinfo.back().jets.at(i).jetmatchindex;
            int daumatchindex = eventsinfo.back().jets.at(i).daughtermatchindexs.at(j);
            if (daumatchindex == -1) continue;
            TLorentzVector gendau = eventsinfo.at(0).jets.at(jetmatchindex).daughters.at(daumatchindex);
            gendau.Boost(boostvector);
            double gencoschi_recojpsi = gendau.Vect().Dot(eventsinfo.back().jpsi_tlz.Vect()) * 1.0 / gendau.Vect().Mag() / eventsinfo.back().jpsi_tlz.Vect().Mag();
            hists2ds.at(4)->Fill(gencoschi_recojpsi, coschi, weight);
            if (particletype == "neutral")
              hists2ds.at(8)->Fill(gencoschi_recojpsi, coschi, weight);
            else if (particletype == "charge")
              hists2ds.at(12)->Fill(gencoschi_recojpsi, coschi, weight);

            if (eventsinfo.at(0).jpsicandidates_tlz.size() == 0) continue;
            TVector3 genboostvector = - (eventsinfo.at(0).jpsi_tlz.BoostVector());
            TLorentzVector dau2 = eventsinfo.back().jets.at(i).daughters.at(j);
            TLorentzVector gendau2 = eventsinfo.at(0).jets.at(jetmatchindex).daughters.at(daumatchindex);
            dau2.Boost(genboostvector);
            gendau2.Boost(genboostvector);
            double coschi_genjpsi = dau2.Vect().Dot(eventsinfo.at(0).jpsi_tlz.Vect()) * 1.0 / dau2.Vect().Mag() / eventsinfo.at(0).jpsi_tlz.Vect().Mag();
            double gencoschi_genjpsi = gendau2.Vect().Dot(eventsinfo.at(0).jpsi_tlz.Vect()) * 1.0 / gendau2.Vect().Mag() / eventsinfo.at(0).jpsi_tlz.Vect().Mag();
            hists2ds.at(2)->Fill(coschi, gencoschi_genjpsi, weight);
            hists2ds.at(3)->Fill(coschi_genjpsi, gencoschi_genjpsi, weight);
            if (particletype == "neutral")
            {
              hists2ds.at(6)->Fill(coschi, gencoschi_genjpsi, weight);
              hists2ds.at(7)->Fill(coschi_genjpsi, gencoschi_genjpsi, weight);
            }
            else if (particletype == "charge")
            {
              hists2ds.at(10)->Fill(coschi, gencoschi_genjpsi, weight);
              hists2ds.at(11)->Fill(coschi_genjpsi, gencoschi_genjpsi, weight);
            }
          }
        }
      }

      if (SampleType == "mc")
      {
        if (eventsinfo.at(0).jpsicandidates_tlz.size() == 0)
          continue;
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
            gen_dau.Boost(gen_boostvector);
            double gen_coschi = gen_dau.Vect().Dot(eventsinfo.at(0).jpsi_tlz.Vect()) * 1.0 / gen_dau.Vect().Mag() / eventsinfo.at(0).jpsi_tlz.Vect().Mag();
            double gen_ec = gen_dau.E() / eventsinfo.at(0).jpsi_tlz.M();
            TString gen_matchtype = "";
            if (eventsinfo.at(0).jets.at(i).jetmatchindex >= 0 && eventsinfo.at(0).jets.at(i).daughtermatchindexs.at(j) >= 0)
              gen_matchtype = "matched";
            else
              gen_matchtype = "unmatched";

            hists["Gen_coschi_alljet_all_all"]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_all_all" + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_" + gen_particletype + "_all"]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_" + gen_particletype + "_all" + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_all_" + gen_matchtype]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_all_" + gen_matchtype + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_" + gen_particletype + "_" + gen_matchtype]->Fill(gen_coschi, gen_ec * weight);
            hists["Gen_coschi_alljet_" + gen_particletype + "_" + gen_matchtype + gen_ptsuffix]->Fill(gen_coschi, gen_ec * weight);
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
