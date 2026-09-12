//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Dec 26 13:08:35 2025 by ROOT version 6.26/14
// from TTree JetsAndDaughters/Informations of Jets and Daughters
// found on file: /data/shuangyuan/2024datasets/AK8_testdimuon/JetMET0_Run2024C-MINIv6NANOv15/Chunk0.root
//////////////////////////////////////////////////////////

#ifndef JetsAndDaughters_h
#define JetsAndDaughters_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"
#include "vector"
using namespace std;
class JetsAndDaughters {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   vector<double>  *RecoJetPt;
   vector<double>  *RecoJetEta;
   vector<double>  *RecoJetPhi;
   vector<double>  *RecoJetEnergy;
   vector<double>  *RecoJetEnergySumDaughters;
   vector<int>     *RecoJetPartonFlavourHard;
   vector<int>     *RecoJetPartonFlavourSoft;
   vector<int>     *RecoJetHadronFlavour;
   vector<int>     *RecoJetChargedMultiplicity;
   vector<int>     *RecoJetNeutralMultiplicity;
   vector<int>     *RecoDaughterPdgId;
   vector<int>     *RecoDaughterJetId;
   vector<int>     *RecoDaughterCharge;
   vector<double>  *RecoDaughterPt;
   vector<double>  *RecoDaughterEta;
   vector<double>  *RecoDaughterPhi;
   vector<double>  *RecoDaughterEnergy;
   vector<double>  *RecoJetPtRaw;
   vector<double>  *RecoJetPtJES;
   vector<double>  *RecoJetPtJESUp;
   vector<double>  *RecoJetPtJESDn;
   vector<double>  *RecoJetPtJERUp;
   vector<double>  *RecoJetPtJERDn;
   vector<double>  *RecoJetPtJERTight;
   vector<bool>    *RecoJetPassHotZone;
   vector<double>  *RecoDaughterRandomDrop;
   vector<double>  *RecoDaughterTrackerUncertainty;
   vector<double>  *RecoDaughterEcalUncertainty;
   vector<double>  *RecoDaughterHcalUncertainty;
   vector<double>  *RecoDeepFlavourProbg;
   vector<double>  *RecoDeepFlavourProbuds;
   vector<double>  *RecoDeepFlavourQG;
   vector<double>  *RecoParticleTransformerQG;
   vector<double>  *RecoParticleNetForwardQG;
   vector<double>  *RecoParticleNetCentralQG;
   vector<double>  *RecoParticleNetQG;
   vector<bool>    *TriggerBits;
   vector<double>  *TriggerPrescales;
   vector<bool>    *MetFilterBits;
   vector<double>  *RecoMuonPt;
   vector<double>  *RecoMuonEta;
   vector<double>  *RecoMuonPhi;
   vector<double>  *RecoMuonEnergy;
   vector<int>     *RecoMuonCharge;
   vector<int>     *RecoMuonIsoID;
   vector<bool>    *RecoMuonLooseID;
   vector<bool>    *RecoMuonMediumID;
   vector<bool>    *RecoMuonTightID;
   vector<bool>    *RecoMuonSoftID;
   vector<bool>    *RecoMuonPassMediumTrigger;
   vector<bool>    *RecoMuonPassHighTrigger1;
   vector<bool>    *RecoMuonPassHighTrigger2;
   vector<bool>    *RecoMuonPassHighTrigger3;
   vector<bool>    *RecoMuonPassDoubleTrigger;
   vector<int>     *RecoMuonJetIndex;
   vector<double>  *RecoMuonSoftMvaValue;
   vector<double>  *RecoMuonMvaIDValue;
   vector<int>     *RecoMuonMvaIDWP;
   vector<double>  *RecoDiMuonPt;
   vector<double>  *RecoDiMuonEta;
   vector<double>  *RecoDiMuonPhi;
   vector<double>  *RecoDiMuonEnergy;
   vector<double>  *RecoDiMuonMass;
   vector<int>     *RecoDiMuonCharge;
   vector<int>     *RecoDiMuonDaughter1;
   vector<int>     *RecoDiMuonDaughter2;
   vector<double>  *RecoDiMuonvProb;
   vector<double>  *RecoDiMuonvNChi2;
   vector<double>  *RecoDiMuonMassErr;
   vector<double>  *RecoDiMuonppdlPV;
   vector<double>  *RecoDiMuonppdlErrPV;
   vector<double>  *RecoDiMuoncosAlpha;
   vector<double>  *RecoDiMuonppdlBS;
   vector<double>  *RecoDiMuonppdlErrBS;
   vector<double>  *RecoDiMuonDCA;
   vector<int>     *RecoDiMuoncountTksOfPV;
   vector<double>  *RecoDiMuonvertexWeight;
   vector<double>  *RecoDiMuonsumPTPV;
   vector<int>     *RecoDiMuonmomPDGId;
   vector<double>  *RecoDiMuonppdlTrue;
   Int_t           RunID;
   Long64_t        EventID;
   Int_t           LumiID;
   Bool_t          RecoPassDijet;
   Bool_t          RecoPassZJet;
   Int_t           NumberPrimaryVertex;
   Int_t           NumberGoodVertex;
   Long64_t        TotalEventNumber;
   Double_t        NextPassedNumber;

   // List of branches
   TBranch        *b_RecoJetPt;   //!
   TBranch        *b_RecoJetEta;   //!
   TBranch        *b_RecoJetPhi;   //!
   TBranch        *b_RecoJetEnergy;   //!
   TBranch        *b_RecoJetEnergySumDaughters;   //!
   TBranch        *b_RecoJetPartonFlavourHard;   //!
   TBranch        *b_RecoJetPartonFlavourSoft;   //!
   TBranch        *b_RecoJetHadronFlavour;   //!
   TBranch        *b_RecoJetChargedMultiplicity;   //!
   TBranch        *b_RecoJetNeutralMultiplicity;   //!
   TBranch        *b_RecoDaughterPdgId;   //!
   TBranch        *b_RecoDaughterJetId;   //!
   TBranch        *b_RecoDaughterCharge;   //!
   TBranch        *b_RecoDaughterPt;   //!
   TBranch        *b_RecoDaughterEta;   //!
   TBranch        *b_RecoDaughterPhi;   //!
   TBranch        *b_RecoDaughterEnergy;   //!
   TBranch        *b_RecoJetPtRaw;   //!
   TBranch        *b_RecoJetPtJES;   //!
   TBranch        *b_RecoJetPtJESUp;   //!
   TBranch        *b_RecoJetPtJESDn;   //!
   TBranch        *b_RecoJetPtJERUp;   //!
   TBranch        *b_RecoJetPtJERDn;   //!
   TBranch        *b_RecoJetPtJERTight;   //!
   TBranch        *b_RecoJetPassHotZone;   //!
   TBranch        *b_RecoDaughterRandomDrop;   //!
   TBranch        *b_RecoDaughterTrackerUncertainty;   //!
   TBranch        *b_RecoDaughterEcalUncertainty;   //!
   TBranch        *b_RecoDaughterHcalUncertainty;   //!
   TBranch        *b_RecoDeepFlavourProbg;   //!
   TBranch        *b_RecoDeepFlavourProbuds;   //!
   TBranch        *b_RecoDeepFlavourQG;   //!
   TBranch        *b_RecoParticleTransformerQG;   //!
   TBranch        *b_RecoParticleNetForwardQG;   //!
   TBranch        *b_RecoParticleNetCentralQG;   //!
   TBranch        *b_RecoParticleNetQG;   //!
   TBranch        *b_TriggerBits;   //!
   TBranch        *b_TriggerPrescales;   //!
   TBranch        *b_MetFilterBits;   //!
   TBranch        *b_RecoMuonPt;   //!
   TBranch        *b_RecoMuonEta;   //!
   TBranch        *b_RecoMuonPhi;   //!
   TBranch        *b_RecoMuonEnergy;   //!
   TBranch        *b_RecoMuonCharge;   //!
   TBranch        *b_RecoMuonIsoID;   //!
   TBranch        *b_RecoMuonLooseID;   //!
   TBranch        *b_RecoMuonMediumID;   //!
   TBranch        *b_RecoMuonTightID;   //!
   TBranch        *b_RecoMuonSoftID;   //!
   TBranch        *b_RecoMuonPassMediumTrigger;   //!
   TBranch        *b_RecoMuonPassHighTrigger1;   //!
   TBranch        *b_RecoMuonPassHighTrigger2;   //!
   TBranch        *b_RecoMuonPassHighTrigger3;   //!
   TBranch        *b_RecoMuonPassDoubleTrigger;   //!
   TBranch        *b_RecoMuonJetIndex;   //!
   TBranch        *b_RecoMuonSoftMvaValue;   //!
   TBranch        *b_RecoMuonMvaIDValue;   //!
   TBranch        *b_RecoMuonMvaIDWP;   //!
   TBranch        *b_RecoDiMuonPt;   //!
   TBranch        *b_RecoDiMuonEta;   //!
   TBranch        *b_RecoDiMuonPhi;   //!
   TBranch        *b_RecoDiMuonEnergy;   //!
   TBranch        *b_RecoDiMuonMass;   //!
   TBranch        *b_RecoDiMuonCharge;   //!
   TBranch        *b_RecoDiMuonDaughter1;   //!
   TBranch        *b_RecoDiMuonDaughter2;   //!
   TBranch        *b_RecoDiMuonvProb;   //!
   TBranch        *b_RecoDiMuonvNChi2;   //!
   TBranch        *b_RecoDiMuonMassErr;   //!
   TBranch        *b_RecoDiMuonppdlPV;   //!
   TBranch        *b_RecoDiMuonppdlErrPV;   //!
   TBranch        *b_RecoDiMuoncosAlpha;   //!
   TBranch        *b_RecoDiMuonppdlBS;   //!
   TBranch        *b_RecoDiMuonppdlErrBS;   //!
   TBranch        *b_RecoDiMuonDCA;   //!
   TBranch        *b_RecoDiMuoncountTksOfPV;   //!
   TBranch        *b_RecoDiMuonvertexWeight;   //!
   TBranch        *b_RecoDiMuonsumPTPV;   //!
   TBranch        *b_RecoDiMuonmomPDGId;   //!
   TBranch        *b_RecoDiMuonppdlTrue;   //!
   TBranch        *b_RunID;   //!
   TBranch        *b_EventID;   //!
   TBranch        *b_LumiID;   //!
   TBranch        *b_RecoPassDijet;   //!
   TBranch        *b_RecoPassZJet;   //!
   TBranch        *b_NumberPrimaryVertex;   //!
   TBranch        *b_NumberGoodVertex;   //!
   TBranch        *b_TotalEventNumber;   //!
   TBranch        *b_NextPassedNumber;   //!

   JetsAndDaughters(TTree *tree=0);
   virtual ~JetsAndDaughters();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

JetsAndDaughters::JetsAndDaughters(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/data/shuangyuan/2024datasets/AK8_testdimuon/JetMET0_Run2024C-MINIv6NANOv15/Chunk0.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/data/shuangyuan/2024datasets/AK8_testdimuon/JetMET0_Run2024C-MINIv6NANOv15/Chunk0.root");
      }
      f->GetObject("JetsAndDaughters",tree);

   }
   Init(tree);
}

JetsAndDaughters::~JetsAndDaughters()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t JetsAndDaughters::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t JetsAndDaughters::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void JetsAndDaughters::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   RecoJetPt = 0;
   RecoJetEta = 0;
   RecoJetPhi = 0;
   RecoJetEnergy = 0;
   RecoJetEnergySumDaughters = 0;
   RecoJetPartonFlavourHard = 0;
   RecoJetPartonFlavourSoft = 0;
   RecoJetHadronFlavour = 0;
   RecoJetChargedMultiplicity = 0;
   RecoJetNeutralMultiplicity = 0;
   RecoDaughterPdgId = 0;
   RecoDaughterJetId = 0;
   RecoDaughterCharge = 0;
   RecoDaughterPt = 0;
   RecoDaughterEta = 0;
   RecoDaughterPhi = 0;
   RecoDaughterEnergy = 0;
   RecoJetPtRaw = 0;
   RecoJetPtJES = 0;
   RecoJetPtJESUp = 0;
   RecoJetPtJESDn = 0;
   RecoJetPtJERUp = 0;
   RecoJetPtJERDn = 0;
   RecoJetPtJERTight = 0;
   RecoJetPassHotZone = 0;
   RecoDaughterRandomDrop = 0;
   RecoDaughterTrackerUncertainty = 0;
   RecoDaughterEcalUncertainty = 0;
   RecoDaughterHcalUncertainty = 0;
   RecoDeepFlavourProbg = 0;
   RecoDeepFlavourProbuds = 0;
   RecoDeepFlavourQG = 0;
   RecoParticleTransformerQG = 0;
   RecoParticleNetForwardQG = 0;
   RecoParticleNetCentralQG = 0;
   RecoParticleNetQG = 0;
   TriggerBits = 0;
   TriggerPrescales = 0;
   MetFilterBits = 0;
   RecoMuonPt = 0;
   RecoMuonEta = 0;
   RecoMuonPhi = 0;
   RecoMuonEnergy = 0;
   RecoMuonCharge = 0;
   RecoMuonIsoID = 0;
   RecoMuonLooseID = 0;
   RecoMuonMediumID = 0;
   RecoMuonTightID = 0;
   RecoMuonSoftID = 0;
   RecoMuonPassMediumTrigger = 0;
   RecoMuonPassHighTrigger1 = 0;
   RecoMuonPassHighTrigger2 = 0;
   RecoMuonPassHighTrigger3 = 0;
   RecoMuonPassDoubleTrigger = 0;
   RecoMuonJetIndex = 0;
   RecoMuonSoftMvaValue = 0;
   RecoMuonMvaIDValue = 0;
   RecoMuonMvaIDWP = 0;
   RecoDiMuonPt = 0;
   RecoDiMuonEta = 0;
   RecoDiMuonPhi = 0;
   RecoDiMuonEnergy = 0;
   RecoDiMuonMass = 0;
   RecoDiMuonCharge = 0;
   RecoDiMuonDaughter1 = 0;
   RecoDiMuonDaughter2 = 0;
   RecoDiMuonvProb = 0;
   RecoDiMuonvNChi2 = 0;
   RecoDiMuonMassErr = 0;
   RecoDiMuonppdlPV = 0;
   RecoDiMuonppdlErrPV = 0;
   RecoDiMuoncosAlpha = 0;
   RecoDiMuonppdlBS = 0;
   RecoDiMuonppdlErrBS = 0;
   RecoDiMuonDCA = 0;
   RecoDiMuoncountTksOfPV = 0;
   RecoDiMuonvertexWeight = 0;
   RecoDiMuonsumPTPV = 0;
   RecoDiMuonmomPDGId = 0;
   RecoDiMuonppdlTrue = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("RecoJetPt", &RecoJetPt, &b_RecoJetPt);
   fChain->SetBranchAddress("RecoJetEta", &RecoJetEta, &b_RecoJetEta);
   fChain->SetBranchAddress("RecoJetPhi", &RecoJetPhi, &b_RecoJetPhi);
   fChain->SetBranchAddress("RecoJetEnergy", &RecoJetEnergy, &b_RecoJetEnergy);
   fChain->SetBranchAddress("RecoJetEnergySumDaughters", &RecoJetEnergySumDaughters, &b_RecoJetEnergySumDaughters);
   fChain->SetBranchAddress("RecoJetPartonFlavourHard", &RecoJetPartonFlavourHard, &b_RecoJetPartonFlavourHard);
   fChain->SetBranchAddress("RecoJetPartonFlavourSoft", &RecoJetPartonFlavourSoft, &b_RecoJetPartonFlavourSoft);
   fChain->SetBranchAddress("RecoJetHadronFlavour", &RecoJetHadronFlavour, &b_RecoJetHadronFlavour);
   fChain->SetBranchAddress("RecoJetChargedMultiplicity", &RecoJetChargedMultiplicity, &b_RecoJetChargedMultiplicity);
   fChain->SetBranchAddress("RecoJetNeutralMultiplicity", &RecoJetNeutralMultiplicity, &b_RecoJetNeutralMultiplicity);
   fChain->SetBranchAddress("RecoDaughterPdgId", &RecoDaughterPdgId, &b_RecoDaughterPdgId);
   fChain->SetBranchAddress("RecoDaughterJetId", &RecoDaughterJetId, &b_RecoDaughterJetId);
   fChain->SetBranchAddress("RecoDaughterCharge", &RecoDaughterCharge, &b_RecoDaughterCharge);
   fChain->SetBranchAddress("RecoDaughterPt", &RecoDaughterPt, &b_RecoDaughterPt);
   fChain->SetBranchAddress("RecoDaughterEta", &RecoDaughterEta, &b_RecoDaughterEta);
   fChain->SetBranchAddress("RecoDaughterPhi", &RecoDaughterPhi, &b_RecoDaughterPhi);
   fChain->SetBranchAddress("RecoDaughterEnergy", &RecoDaughterEnergy, &b_RecoDaughterEnergy);
   fChain->SetBranchAddress("RecoJetPtRaw", &RecoJetPtRaw, &b_RecoJetPtRaw);
   fChain->SetBranchAddress("RecoJetPtJES", &RecoJetPtJES, &b_RecoJetPtJES);
   fChain->SetBranchAddress("RecoJetPtJESUp", &RecoJetPtJESUp, &b_RecoJetPtJESUp);
   fChain->SetBranchAddress("RecoJetPtJESDn", &RecoJetPtJESDn, &b_RecoJetPtJESDn);
   fChain->SetBranchAddress("RecoJetPtJERUp", &RecoJetPtJERUp, &b_RecoJetPtJERUp);
   fChain->SetBranchAddress("RecoJetPtJERDn", &RecoJetPtJERDn, &b_RecoJetPtJERDn);
   fChain->SetBranchAddress("RecoJetPtJERTight", &RecoJetPtJERTight, &b_RecoJetPtJERTight);
   fChain->SetBranchAddress("RecoJetPassHotZone", &RecoJetPassHotZone, &b_RecoJetPassHotZone);
   fChain->SetBranchAddress("RecoDaughterRandomDrop", &RecoDaughterRandomDrop, &b_RecoDaughterRandomDrop);
   fChain->SetBranchAddress("RecoDaughterTrackerUncertainty", &RecoDaughterTrackerUncertainty, &b_RecoDaughterTrackerUncertainty);
   fChain->SetBranchAddress("RecoDaughterEcalUncertainty", &RecoDaughterEcalUncertainty, &b_RecoDaughterEcalUncertainty);
   fChain->SetBranchAddress("RecoDaughterHcalUncertainty", &RecoDaughterHcalUncertainty, &b_RecoDaughterHcalUncertainty);
   fChain->SetBranchAddress("RecoDeepFlavourProbg", &RecoDeepFlavourProbg, &b_RecoDeepFlavourProbg);
   fChain->SetBranchAddress("RecoDeepFlavourProbuds", &RecoDeepFlavourProbuds, &b_RecoDeepFlavourProbuds);
   fChain->SetBranchAddress("RecoDeepFlavourQG", &RecoDeepFlavourQG, &b_RecoDeepFlavourQG);
   fChain->SetBranchAddress("RecoParticleTransformerQG", &RecoParticleTransformerQG, &b_RecoParticleTransformerQG);
   fChain->SetBranchAddress("RecoParticleNetForwardQG", &RecoParticleNetForwardQG, &b_RecoParticleNetForwardQG);
   fChain->SetBranchAddress("RecoParticleNetCentralQG", &RecoParticleNetCentralQG, &b_RecoParticleNetCentralQG);
   fChain->SetBranchAddress("RecoParticleNetQG", &RecoParticleNetQG, &b_RecoParticleNetQG);
   fChain->SetBranchAddress("TriggerBits", &TriggerBits, &b_TriggerBits);
   fChain->SetBranchAddress("TriggerPrescales", &TriggerPrescales, &b_TriggerPrescales);
   fChain->SetBranchAddress("MetFilterBits", &MetFilterBits, &b_MetFilterBits);
   fChain->SetBranchAddress("RecoMuonPt", &RecoMuonPt, &b_RecoMuonPt);
   fChain->SetBranchAddress("RecoMuonEta", &RecoMuonEta, &b_RecoMuonEta);
   fChain->SetBranchAddress("RecoMuonPhi", &RecoMuonPhi, &b_RecoMuonPhi);
   fChain->SetBranchAddress("RecoMuonEnergy", &RecoMuonEnergy, &b_RecoMuonEnergy);
   fChain->SetBranchAddress("RecoMuonCharge", &RecoMuonCharge, &b_RecoMuonCharge);
   fChain->SetBranchAddress("RecoMuonIsoID", &RecoMuonIsoID, &b_RecoMuonIsoID);
   fChain->SetBranchAddress("RecoMuonLooseID", &RecoMuonLooseID, &b_RecoMuonLooseID);
   fChain->SetBranchAddress("RecoMuonMediumID", &RecoMuonMediumID, &b_RecoMuonMediumID);
   fChain->SetBranchAddress("RecoMuonTightID", &RecoMuonTightID, &b_RecoMuonTightID);
   fChain->SetBranchAddress("RecoMuonSoftID", &RecoMuonSoftID, &b_RecoMuonSoftID);
   fChain->SetBranchAddress("RecoMuonPassMediumTrigger", &RecoMuonPassMediumTrigger, &b_RecoMuonPassMediumTrigger);
   fChain->SetBranchAddress("RecoMuonPassHighTrigger1", &RecoMuonPassHighTrigger1, &b_RecoMuonPassHighTrigger1);
   fChain->SetBranchAddress("RecoMuonPassHighTrigger2", &RecoMuonPassHighTrigger2, &b_RecoMuonPassHighTrigger2);
   fChain->SetBranchAddress("RecoMuonPassHighTrigger3", &RecoMuonPassHighTrigger3, &b_RecoMuonPassHighTrigger3);
   fChain->SetBranchAddress("RecoMuonPassDoubleTrigger", &RecoMuonPassDoubleTrigger, &b_RecoMuonPassDoubleTrigger);
   fChain->SetBranchAddress("RecoMuonJetIndex", &RecoMuonJetIndex, &b_RecoMuonJetIndex);
   fChain->SetBranchAddress("RecoMuonSoftMvaValue", &RecoMuonSoftMvaValue, &b_RecoMuonSoftMvaValue);
   fChain->SetBranchAddress("RecoMuonMvaIDValue", &RecoMuonMvaIDValue, &b_RecoMuonMvaIDValue);
   fChain->SetBranchAddress("RecoMuonMvaIDWP", &RecoMuonMvaIDWP, &b_RecoMuonMvaIDWP);
   fChain->SetBranchAddress("RecoDiMuonPt", &RecoDiMuonPt, &b_RecoDiMuonPt);
   fChain->SetBranchAddress("RecoDiMuonEta", &RecoDiMuonEta, &b_RecoDiMuonEta);
   fChain->SetBranchAddress("RecoDiMuonPhi", &RecoDiMuonPhi, &b_RecoDiMuonPhi);
   fChain->SetBranchAddress("RecoDiMuonEnergy", &RecoDiMuonEnergy, &b_RecoDiMuonEnergy);
   fChain->SetBranchAddress("RecoDiMuonMass", &RecoDiMuonMass, &b_RecoDiMuonMass);
   fChain->SetBranchAddress("RecoDiMuonCharge", &RecoDiMuonCharge, &b_RecoDiMuonCharge);
   fChain->SetBranchAddress("RecoDiMuonDaughter1", &RecoDiMuonDaughter1, &b_RecoDiMuonDaughter1);
   fChain->SetBranchAddress("RecoDiMuonDaughter2", &RecoDiMuonDaughter2, &b_RecoDiMuonDaughter2);
   fChain->SetBranchAddress("RecoDiMuonvProb", &RecoDiMuonvProb, &b_RecoDiMuonvProb);
   fChain->SetBranchAddress("RecoDiMuonvNChi2", &RecoDiMuonvNChi2, &b_RecoDiMuonvNChi2);
   fChain->SetBranchAddress("RecoDiMuonMassErr", &RecoDiMuonMassErr, &b_RecoDiMuonMassErr);
   fChain->SetBranchAddress("RecoDiMuonppdlPV", &RecoDiMuonppdlPV, &b_RecoDiMuonppdlPV);
   fChain->SetBranchAddress("RecoDiMuonppdlErrPV", &RecoDiMuonppdlErrPV, &b_RecoDiMuonppdlErrPV);
   fChain->SetBranchAddress("RecoDiMuoncosAlpha", &RecoDiMuoncosAlpha, &b_RecoDiMuoncosAlpha);
   fChain->SetBranchAddress("RecoDiMuonppdlBS", &RecoDiMuonppdlBS, &b_RecoDiMuonppdlBS);
   fChain->SetBranchAddress("RecoDiMuonppdlErrBS", &RecoDiMuonppdlErrBS, &b_RecoDiMuonppdlErrBS);
   fChain->SetBranchAddress("RecoDiMuonDCA", &RecoDiMuonDCA, &b_RecoDiMuonDCA);
   fChain->SetBranchAddress("RecoDiMuoncountTksOfPV", &RecoDiMuoncountTksOfPV, &b_RecoDiMuoncountTksOfPV);
   fChain->SetBranchAddress("RecoDiMuonvertexWeight", &RecoDiMuonvertexWeight, &b_RecoDiMuonvertexWeight);
   fChain->SetBranchAddress("RecoDiMuonsumPTPV", &RecoDiMuonsumPTPV, &b_RecoDiMuonsumPTPV);
   fChain->SetBranchAddress("RecoDiMuonmomPDGId", &RecoDiMuonmomPDGId, &b_RecoDiMuonmomPDGId);
   fChain->SetBranchAddress("RecoDiMuonppdlTrue", &RecoDiMuonppdlTrue, &b_RecoDiMuonppdlTrue);
   fChain->SetBranchAddress("RunID", &RunID, &b_RunID);
   fChain->SetBranchAddress("EventID", &EventID, &b_EventID);
   fChain->SetBranchAddress("LumiID", &LumiID, &b_LumiID);
   fChain->SetBranchAddress("RecoPassDijet", &RecoPassDijet, &b_RecoPassDijet);
   fChain->SetBranchAddress("RecoPassZJet", &RecoPassZJet, &b_RecoPassZJet);
   fChain->SetBranchAddress("NumberPrimaryVertex", &NumberPrimaryVertex, &b_NumberPrimaryVertex);
   fChain->SetBranchAddress("NumberGoodVertex", &NumberGoodVertex, &b_NumberGoodVertex);
   fChain->SetBranchAddress("TotalEventNumber", &TotalEventNumber, &b_TotalEventNumber);
   fChain->SetBranchAddress("NextPassedNumber", &NextPassedNumber, &b_NextPassedNumber);
   Notify();
}

Bool_t JetsAndDaughters::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void JetsAndDaughters::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t JetsAndDaughters::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

