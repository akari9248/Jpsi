//---------------------------------------------------------------------------------------------------------------------
// Save event information into a TTree.
// Only stores:
//   - Parton shower final state particles (final parton-level)
//   - Final stable hadrons
//   - Event weight
//   - J/psi, its mother and grandmother info
// input parameters: --seed, --chunknum, --events, --outputdir, --oniashower,
// --crmode
//---------------------------------------------------------------------------------------------------------------------
#include "Pythia8/Pythia.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TNamed.h"
#include "TTree.h"
#include "include/MotherCategory.h"
#include "include/ProgressBar.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace Pythia8;

// Find effective grandmother: trace mother's ancestors until finding a particle
// with a different category than mother.
int findEffectiveGrandmother(int mother_idx, const Pythia8::Event &event) {
  if (mother_idx <= 0)
    return 0;
  int mom_cat = jpsi_origin::category(event[mother_idx].id());
  int ancestor = event[mother_idx].mother1();
  while (ancestor > 0) {
    int anc_cat = jpsi_origin::category(event[ancestor].id());
    if (anc_cat != mom_cat)
      return event[ancestor].id();
    ancestor = event[ancestor].mother1();
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // Read command line arguments
  int index = 0;
  int seed = 0;
  int nEvent = 1000;
  std::string outputdir = "output";
  std::string oniashower_mode = "on";
  int cr_mode = 0;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--seed") {
      if (i + 1 < argc)
        seed = std::atoi(argv[++i]);
    } else if (arg == "--chunknum") {
      if (i + 1 < argc)
        index = std::atoi(argv[++i]);
    } else if (arg == "--events") {
      if (i + 1 < argc)
        nEvent = std::atoi(argv[++i]);
    } else if (arg == "--outputdir") {
      if (i + 1 < argc)
        outputdir = argv[++i];
    } else if (arg == "--oniashower") {
      if (i + 1 < argc)
        oniashower_mode = argv[++i];
    } else if (arg == "--crmode") {
      if (i + 1 < argc)
        cr_mode = std::atoi(argv[++i]);
    }
  }

  Pythia pythia;

  // Random seed
  std::string seed_str = "Random:seed = " + std::to_string(seed);
  pythia.readString("Random:setSeed = on");
  pythia.readString(seed_str);

  // Basic settings
  pythia.readString("Beams:idA = 2212");
  pythia.readString("Beams:idB = 2212");
  pythia.readString("Beams:eCM = 13600");
  pythia.readString("HardQCD:all = on");
  pythia.readString("PhaseSpace:pTHatMin = 15.");
  pythia.readString("PhaseSpace:pTHatMax = 7000.");
  pythia.readString("PhaseSpace:bias2Selection = on");
  pythia.readString("PhaseSpace:bias2SelectionPow = 4.5");
  pythia.readString("PhaseSpace:bias2SelectionRef = 15.");
  pythia.readString("443:onMode = off");
  pythia.readString("443:onIfMatch = 13 -13");
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 0");
  pythia.readString("Init:showChangedSettings = off");
  pythia.readString("Init:showChangedParticleData = off");

  // CP5 tuning
  pythia.readString("Tune:pp 14");
  pythia.readString("Tune:ee 7");
  pythia.readString("MultipartonInteractions:ecmPow=0.03344");
  pythia.readString("MultipartonInteractions:bProfile=2");
  pythia.readString("MultipartonInteractions:pT0Ref=1.41");
  pythia.readString("MultipartonInteractions:coreRadius=0.7634");
  pythia.readString("MultipartonInteractions:coreFraction=0.63");
  pythia.readString("ColourReconnection:range=5.176");
  pythia.readString("SigmaTotal:zeroAXB=off");
  pythia.readString("SpaceShower:alphaSorder=2");
  pythia.readString("SpaceShower:alphaSvalue=0.118");
  pythia.readString("SigmaProcess:alphaSvalue=0.118");
  pythia.readString("SigmaProcess:alphaSorder=2");
  pythia.readString("MultipartonInteractions:alphaSvalue=0.118");
  pythia.readString("MultipartonInteractions:alphaSorder=2");
  pythia.readString("TimeShower:alphaSorder=2");
  pythia.readString("TimeShower:alphaSvalue=0.118");
  pythia.readString("SigmaTotal:mode = 0");
  pythia.readString("SigmaTotal:sigmaEl = 22.08");
  pythia.readString("SigmaTotal:sigmaTot = 101.037");
  pythia.readString("PDF:pSet=LHAPDF6:NNPDF31_nnlo_as_0118");
  pythia.readString("PDF:lepton = off");
  pythia.readString("Tune:preferLHAPDF = 2");
  pythia.readString("Main:timesAllowErrors = 10000");
  pythia.readString("Check:epTolErr = 0.01");
  pythia.readString("Beams:setProductionScalesFromLHEF = off");
  pythia.readString("SLHA:minMassSM = 1000.");
  pythia.readString("ParticleDecays:limitTau0 = on");
  pythia.readString("ParticleDecays:tau0Max = 10");

  // OniaShower & ColourReconnection:mode only available in Pythia 8.310+
  {
    double pythia_ver = PYTHIA_VERSION;
    int minor = (int)(pythia_ver * 1000 + 0.5) % 1000;
    bool has_features = (minor >= 310);

    if (has_features) {
      pythia.readString("OniaShower:all = " + oniashower_mode);
      if (cr_mode == 1) {
        pythia.readString("ColourReconnection:mode = 1");
        pythia.readString("BeamRemnants:remnantMode = 1");
      }
    } else {
      std::cout << "Info: Pythia " << pythia_ver
                << " does not support OniaShower/CR mode. Skipping."
                << std::endl;
    }
  }

  if (!pythia.init()) {
    std::cerr << "ERROR: Pythia initialization failed" << std::endl;
    return 1;
  }

  // Output file
  std::string outfile_name =
      outputdir + "/pythia8_jpsi_events_" + std::to_string(index) + ".root";
  TFile *outfile = TFile::Open(outfile_name.c_str(), "RECREATE");
  if (!outfile || outfile->IsZombie())
    return 1;

  // ==================== TTree variables ====================
  double generatorweight = 0.0;

  // Parton shower final state particles
  std::vector<double> ps_pt, ps_eta, ps_phi, ps_e;
  std::vector<int> ps_pdgid;
  std::vector<double> ps_charge;

  // Final stable hadrons
  std::vector<double> hadron_pt, hadron_eta, hadron_phi, hadron_e;
  std::vector<int> hadron_pdgid;
  std::vector<double> hadron_charge;
  std::vector<int> hadron_from_jpsi; // 1 if hadron originates from J/psi decay

  // J/psi info
  double jpsi_pt, jpsi_eta, jpsi_phi, jpsi_e, jpsi_mass;
  int jpsi_pdgid;

  // Mother info
  double mother_pt, mother_eta, mother_phi, mother_e, mother_mass;
  int mother_pdgid;
  bool has_mother;

  // Grandmother pdgid
  int grandmother_pdgid = 0;

  // ==================== Create TTree ====================
  TTree *tree = new TTree("CharmoniumInfo", "CharmoniumInfo");
  tree->Branch("generatorweight", &generatorweight);

  // Parton shower final state
  tree->Branch("ps_pt", &ps_pt);
  tree->Branch("ps_eta", &ps_eta);
  tree->Branch("ps_phi", &ps_phi);
  tree->Branch("ps_e", &ps_e);
  tree->Branch("ps_pdgid", &ps_pdgid);
  tree->Branch("ps_charge", &ps_charge);

  // Hadrons
  tree->Branch("hadron_pt", &hadron_pt);
  tree->Branch("hadron_eta", &hadron_eta);
  tree->Branch("hadron_phi", &hadron_phi);
  tree->Branch("hadron_e", &hadron_e);
  tree->Branch("hadron_pdgid", &hadron_pdgid);
  tree->Branch("hadron_charge", &hadron_charge);
  tree->Branch("hadron_from_jpsi", &hadron_from_jpsi);

  // J/psi
  tree->Branch("jpsi_pt", &jpsi_pt);
  tree->Branch("jpsi_eta", &jpsi_eta);
  tree->Branch("jpsi_phi", &jpsi_phi);
  tree->Branch("jpsi_e", &jpsi_e);
  tree->Branch("jpsi_mass", &jpsi_mass);
  tree->Branch("jpsi_pdgid", &jpsi_pdgid);

  // Mother
  tree->Branch("has_mother", &has_mother);
  tree->Branch("mother_pt", &mother_pt);
  tree->Branch("mother_eta", &mother_eta);
  tree->Branch("mother_phi", &mother_phi);
  tree->Branch("mother_e", &mother_e);
  tree->Branch("mother_mass", &mother_mass);
  tree->Branch("mother_pdgid", &mother_pdgid);

  // Grandmother
  tree->Branch("grandmother_pdgid", &grandmother_pdgid);

  // ==================== Event loop ====================
  int iEvent = 0;
  ProgressBar progress(nEvent);
  while (iEvent < nEvent) {
    if (!pythia.next())
      continue;
    progress.update2(iEvent + 1);
    generatorweight = pythia.info.weight();

    // Find primary J/psi (not from another J/psi)
    int true_jpsi_idx = -1;
    TLorentzVector true_jpsi_p4;
    int mother_idx = -1;
    bool found = false;

    for (int i = 0; i < pythia.event.size(); ++i) {
      if (std::abs(pythia.event[i].id()) != 443)
        continue;
      if (pythia.event[i].mother1() > 0 &&
          std::abs(pythia.event[pythia.event[i].mother1()].id()) == 443)
        continue;
      if (found) {
        true_jpsi_idx = -1;
        break;
      }
      true_jpsi_idx = i;
      true_jpsi_p4.SetPtEtaPhiE(pythia.event[i].pT(), pythia.event[i].eta(),
                                pythia.event[i].phi(), pythia.event[i].e());
      mother_idx = pythia.event[i].mother1();
      found = true;
    }

    if (true_jpsi_idx == -1)
      continue;

    // Mother info
    if (mother_idx > 0 && mother_idx < pythia.event.size()) {
      const Particle &moth = pythia.event[mother_idx];
      mother_pdgid = moth.id();
      mother_pt = moth.pT();
      mother_eta = moth.eta();
      mother_phi = moth.phi();
      mother_e = moth.e();
      mother_mass = moth.m();
      has_mother = true;
    } else {
      mother_pdgid = 0;
      mother_pt = mother_eta = mother_phi = mother_e = mother_mass = 0;
      has_mother = false;
    }

    grandmother_pdgid = findEffectiveGrandmother(mother_idx, pythia.event);

    // Clear containers
    ps_pt.clear();
    ps_eta.clear();
    ps_phi.clear();
    ps_e.clear();
    ps_pdgid.clear();
    ps_charge.clear();

    hadron_pt.clear();
    hadron_eta.clear();
    hadron_phi.clear();
    hadron_e.clear();
    hadron_pdgid.clear();
    hadron_charge.clear();
    hadron_from_jpsi.clear();

    // Collect parton shower final state particles
    for (int i = 0; i < pythia.event.size(); ++i) {
      if (!pythia.event[i].isFinalPartonLevel())
        continue;
      ps_pt.push_back(pythia.event[i].pT());
      ps_eta.push_back(pythia.event[i].eta());
      ps_phi.push_back(pythia.event[i].phi());
      ps_e.push_back(pythia.event[i].e());
      ps_pdgid.push_back(pythia.event[i].id());
      ps_charge.push_back(pythia.event[i].charge());
    }

    // Collect final stable particles (skip neutrinos)
    for (int i = 0; i < pythia.event.size(); ++i) {
      if (!pythia.event[i].isFinal())
        continue;
      int id = pythia.event[i].id();
      if (std::abs(id) == 12 || std::abs(id) == 14 || std::abs(id) == 16)
        continue;

      hadron_pt.push_back(pythia.event[i].pT());
      hadron_eta.push_back(pythia.event[i].eta());
      hadron_phi.push_back(pythia.event[i].phi());
      hadron_e.push_back(pythia.event[i].e());
      hadron_pdgid.push_back(id);
      hadron_charge.push_back(pythia.event[i].charge());

      // Trace ancestry to check if this hadron comes from J/psi decay
      bool from_jpsi = false;
      int curr = i;
      while (curr > 0) {
        if (curr == true_jpsi_idx) {
          from_jpsi = true;
          break;
        }
        curr = pythia.event[curr].mother1();
      }
      hadron_from_jpsi.push_back(from_jpsi ? 1 : 0);
    }

    // J/psi info
    jpsi_pt = true_jpsi_p4.Pt();
    jpsi_eta = true_jpsi_p4.Eta();
    jpsi_phi = true_jpsi_p4.Phi();
    jpsi_e = true_jpsi_p4.E();
    jpsi_mass = true_jpsi_p4.M();
    jpsi_pdgid = pythia.event[true_jpsi_idx].id();

    tree->Fill();
    ++iEvent;
  }

  tree->Write();
  std::ostringstream config;
  config << "generator=pythia8;version=" << PYTHIA_VERSION
         << ";ecm=13600;process=HardQCD:all;pthat_min=15;pthat_max=7000"
         << ";bias_power=4.5;onia_shower=" << oniashower_mode
         << ";cr_mode=" << cr_mode << ";seed=" << seed
         << ";requested_accepted_events=" << nEvent;
  TNamed generationConfig("generation_config", config.str().c_str());
  generationConfig.Write();
  outfile->Close();
  delete outfile;

  std::cout << "Events saved to " << outfile_name << std::endl;
  return 0;
}
