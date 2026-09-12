//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jan 19 09:52:11 2026 by ROOT version 6.38.00
// from TTree MCJetsAndDaughters/Informations of Jets and Daughters
// found on file: root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK8/QCD_Bin-PT-600to800_pythia8-RunIIISummer24/Chunk0.root
//////////////////////////////////////////////////////////

#ifndef MCJetsAndDaughters_h
#define MCJetsAndDaughters_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"
#include "vector"

using namespace std;
class MCJetsAndDaughters {
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
   vector<double>  *RecoMuondxyPV;
   vector<double>  *RecoMuondxyErrPV;
   vector<double>  *RecoMuondxyBS;
   vector<double>  *RecoMuondxyErrBS;
   vector<double>  *RecoMuondzPV;
   vector<double>  *RecoMuondzErr;
   vector<double>  *RecoMuonip3d;
   vector<double>  *RecoMuonsip3d;
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
   vector<double>  *GenJetPt;
   vector<double>  *GenJetEta;
   vector<double>  *GenJetPhi;
   vector<double>  *GenJetEnergy;
   vector<double>  *GenJetEnergySumDaughters;
   vector<int>     *GenJetPartonFlavourHard;
   vector<int>     *GenJetPartonFlavourSoft;
   vector<int>     *GenJetHadronFlavour;
   vector<int>     *GenJetChargedMultiplicity;
   vector<int>     *GenJetNeutralMultiplicity;
   vector<int>     *GenDaughterPdgId;
   vector<int>     *GenDaughterJetId;
   vector<int>     *GenDaughterCharge;
   vector<double>  *GenDaughterPt;
   vector<double>  *GenDaughterEta;
   vector<double>  *GenDaughterPhi;
   vector<double>  *GenDaughterEnergy;
   vector<int>     *Partons_PdgId_prunedGenParticles;
   vector<double>  *Partons_Pt_prunedGenParticles;
   vector<double>  *Partons_Eta_prunedGenParticles;
   vector<double>  *Partons_Phi_prunedGenParticles;
   vector<double>  *Partons_Energy_prunedGenParticles;
   vector<int>     *cHadrons_PdgId_prunedGenParticles;
   vector<double>  *cHadrons_Pt_prunedGenParticles;
   vector<double>  *cHadrons_Eta_prunedGenParticles;
   vector<double>  *cHadrons_Phi_prunedGenParticles;
   vector<double>  *cHadrons_Energy_prunedGenParticles;
   vector<int>     *bHadrons_PdgId_prunedGenParticles;
   vector<double>  *bHadrons_Pt_prunedGenParticles;
   vector<double>  *bHadrons_Eta_prunedGenParticles;
   vector<double>  *bHadrons_Phi_prunedGenParticles;
   vector<double>  *bHadrons_Energy_prunedGenParticles;
   vector<int>     *cHadrons_PdgId_slimmedGenJetsFlavourInfos;
   vector<double>  *cHadrons_Pt_slimmedGenJetsFlavourInfos;
   vector<double>  *cHadrons_Eta_slimmedGenJetsFlavourInfos;
   vector<double>  *cHadrons_Phi_slimmedGenJetsFlavourInfos;
   vector<double>  *cHadrons_Energy_slimmedGenJetsFlavourInfos;
   vector<int>     *bHadrons_PdgId_slimmedGenJetsFlavourInfos;
   vector<double>  *bHadrons_Pt_slimmedGenJetsFlavourInfos;
   vector<double>  *bHadrons_Eta_slimmedGenJetsFlavourInfos;
   vector<double>  *bHadrons_Phi_slimmedGenJetsFlavourInfos;
   vector<double>  *bHadrons_Energy_slimmedGenJetsFlavourInfos;
   vector<double>  *GenMuonPt;
   vector<double>  *GenMuonEta;
   vector<double>  *GenMuonPhi;
   vector<double>  *GenMuonEnergy;
   vector<int>     *GenMuonCharge;
   Double_t        GeneratorWeight;
   vector<double>  *PSWeights;
   vector<double>  *QCDScaleWeights;
   vector<double>  *PDFWeights;
   vector<double>  *LHEPartPt;
   vector<double>  *LHEPartEta;
   vector<double>  *LHEPartPhi;
   vector<double>  *LHEPartMass;
   vector<double>  *LHEPartEnergy;
   vector<double>  *LHEPartIncomingPz;
   vector<int>     *LHEPartPdgId;
   vector<int>     *LHEPartStatus;
   vector<int>     *LHEPartSpin;
   vector<double>  *GenPartonPt;
   Double_t        PreFiringWeightNm;
   Double_t        PreFiringWeightUp;
   Double_t        PreFiringWeightDn;
   Int_t           RunID;
   Long64_t        EventID;
   Int_t           LumiID;
   Bool_t          RecoPassDijet;
   Bool_t          RecoPassZJet;
   Int_t           NumberPrimaryVertex;
   Int_t           NumberGoodVertex;
   Double_t        CrossSection;
   Int_t           NumberInteractionPileup;
   Double_t        NumberTruePileup;
   Double_t        PileupMaxPtHat;
   Bool_t          GenPassDijet;
   vector<int>     *GenJetMatching;
   vector<int>     *GenDaughterMatching;
   vector<int>     *RecoJetMatching;
   vector<int>     *RecoDaughterMatching;
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
   TBranch        *b_RecoMuondxyPV;   //!
   TBranch        *b_RecoMuondxyErrPV;   //!
   TBranch        *b_RecoMuondxyBS;   //!
   TBranch        *b_RecoMuondxyErrBS;   //!
   TBranch        *b_RecoMuondzPV;   //!
   TBranch        *b_RecoMuondzErr;   //!
   TBranch        *b_RecoMuonip3d;   //!
   TBranch        *b_RecoMuonsip3d;   //!
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
   TBranch        *b_GenJetPt;   //!
   TBranch        *b_GenJetEta;   //!
   TBranch        *b_GenJetPhi;   //!
   TBranch        *b_GenJetEnergy;   //!
   TBranch        *b_GenJetEnergySumDaughters;   //!
   TBranch        *b_GenJetPartonFlavourHard;   //!
   TBranch        *b_GenJetPartonFlavourSoft;   //!
   TBranch        *b_GenJetHadronFlavour;   //!
   TBranch        *b_GenJetChargedMultiplicity;   //!
   TBranch        *b_GenJetNeutralMultiplicity;   //!
   TBranch        *b_GenDaughterPdgId;   //!
   TBranch        *b_GenDaughterJetId;   //!
   TBranch        *b_GenDaughterCharge;   //!
   TBranch        *b_GenDaughterPt;   //!
   TBranch        *b_GenDaughterEta;   //!
   TBranch        *b_GenDaughterPhi;   //!
   TBranch        *b_GenDaughterEnergy;   //!
   TBranch        *b_Partons_PdgId_prunedGenParticles;   //!
   TBranch        *b_Partons_Pt_prunedGenParticles;   //!
   TBranch        *b_Partons_Eta_prunedGenParticles;   //!
   TBranch        *b_Partons_Phi_prunedGenParticles;   //!
   TBranch        *b_Partons_Energy_prunedGenParticles;   //!
   TBranch        *b_cHadrons_PdgId_prunedGenParticles;   //!
   TBranch        *b_cHadrons_Pt_prunedGenParticles;   //!
   TBranch        *b_cHadrons_Eta_prunedGenParticles;   //!
   TBranch        *b_cHadrons_Phi_prunedGenParticles;   //!
   TBranch        *b_cHadrons_Energy_prunedGenParticles;   //!
   TBranch        *b_bHadrons_PdgId_prunedGenParticles;   //!
   TBranch        *b_bHadrons_Pt_prunedGenParticles;   //!
   TBranch        *b_bHadrons_Eta_prunedGenParticles;   //!
   TBranch        *b_bHadrons_Phi_prunedGenParticles;   //!
   TBranch        *b_bHadrons_Energy_prunedGenParticles;   //!
   TBranch        *b_cHadrons_PdgId_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_cHadrons_Pt_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_cHadrons_Eta_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_cHadrons_Phi_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_cHadrons_Energy_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_bHadrons_PdgId_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_bHadrons_Pt_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_bHadrons_Eta_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_bHadrons_Phi_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_bHadrons_Energy_slimmedGenJetsFlavourInfos;   //!
   TBranch        *b_GenMuonPt;   //!
   TBranch        *b_GenMuonEta;   //!
   TBranch        *b_GenMuonPhi;   //!
   TBranch        *b_GenMuonEnergy;   //!
   TBranch        *b_GenMuonCharge;   //!
   TBranch        *b_GeneratorWeight;   //!
   TBranch        *b_PSWeights;   //!
   TBranch        *b_QCDScaleWeights;   //!
   TBranch        *b_PDFWeights;   //!
   TBranch        *b_LHEPartPt;   //!
   TBranch        *b_LHEPartEta;   //!
   TBranch        *b_LHEPartPhi;   //!
   TBranch        *b_LHEPartMass;   //!
   TBranch        *b_LHEPartEnergy;   //!
   TBranch        *b_LHEPartIncomingPz;   //!
   TBranch        *b_LHEPartPdgId;   //!
   TBranch        *b_LHEPartStatus;   //!
   TBranch        *b_LHEPartSpin;   //!
   TBranch        *b_GenPartonPt;   //!
   TBranch        *b_PreFiringWeightNm;   //!
   TBranch        *b_PreFiringWeightUp;   //!
   TBranch        *b_PreFiringWeightDn;   //!
   TBranch        *b_RunID;   //!
   TBranch        *b_EventID;   //!
   TBranch        *b_LumiID;   //!
   TBranch        *b_RecoPassDijet;   //!
   TBranch        *b_RecoPassZJet;   //!
   TBranch        *b_NumberPrimaryVertex;   //!
   TBranch        *b_NumberGoodVertex;   //!
   TBranch        *b_CrossSection;   //!
   TBranch        *b_NumberInteractionPileup;   //!
   TBranch        *b_NumberTruePileup;   //!
   TBranch        *b_PileupMaxPtHat;   //!
   TBranch        *b_GenPassDijet;   //!
   TBranch        *b_GenJetMatching;   //!
   TBranch        *b_GenDaughterMatching;   //!
   TBranch        *b_RecoJetMatching;   //!
   TBranch        *b_RecoDaughterMatching;   //!
   TBranch        *b_TotalEventNumber;   //!
   TBranch        *b_NextPassedNumber;   //!

   MCJetsAndDaughters(TTree *tree=0);
   virtual ~MCJetsAndDaughters();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

MCJetsAndDaughters::MCJetsAndDaughters(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK8/QCD_Bin-PT-600to800_pythia8-RunIIISummer24/Chunk0.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/2024datasets/AK8/QCD_Bin-PT-600to800_pythia8-RunIIISummer24/Chunk0.root");
      }
      f->GetObject("MCJetsAndDaughters",tree);

   }
   Init(tree);
}

MCJetsAndDaughters::~MCJetsAndDaughters()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t MCJetsAndDaughters::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t MCJetsAndDaughters::LoadTree(Long64_t entry)
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

void MCJetsAndDaughters::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.

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
   RecoMuondxyPV = 0;
   RecoMuondxyErrPV = 0;
   RecoMuondxyBS = 0;
   RecoMuondxyErrBS = 0;
   RecoMuondzPV = 0;
   RecoMuondzErr = 0;
   RecoMuonip3d = 0;
   RecoMuonsip3d = 0;
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
   GenJetPt = 0;
   GenJetEta = 0;
   GenJetPhi = 0;
   GenJetEnergy = 0;
   GenJetEnergySumDaughters = 0;
   GenJetPartonFlavourHard = 0;
   GenJetPartonFlavourSoft = 0;
   GenJetHadronFlavour = 0;
   GenJetChargedMultiplicity = 0;
   GenJetNeutralMultiplicity = 0;
   GenDaughterPdgId = 0;
   GenDaughterJetId = 0;
   GenDaughterCharge = 0;
   GenDaughterPt = 0;
   GenDaughterEta = 0;
   GenDaughterPhi = 0;
   GenDaughterEnergy = 0;
   Partons_PdgId_prunedGenParticles = 0;
   Partons_Pt_prunedGenParticles = 0;
   Partons_Eta_prunedGenParticles = 0;
   Partons_Phi_prunedGenParticles = 0;
   Partons_Energy_prunedGenParticles = 0;
   cHadrons_PdgId_prunedGenParticles = 0;
   cHadrons_Pt_prunedGenParticles = 0;
   cHadrons_Eta_prunedGenParticles = 0;
   cHadrons_Phi_prunedGenParticles = 0;
   cHadrons_Energy_prunedGenParticles = 0;
   bHadrons_PdgId_prunedGenParticles = 0;
   bHadrons_Pt_prunedGenParticles = 0;
   bHadrons_Eta_prunedGenParticles = 0;
   bHadrons_Phi_prunedGenParticles = 0;
   bHadrons_Energy_prunedGenParticles = 0;
   cHadrons_PdgId_slimmedGenJetsFlavourInfos = 0;
   cHadrons_Pt_slimmedGenJetsFlavourInfos = 0;
   cHadrons_Eta_slimmedGenJetsFlavourInfos = 0;
   cHadrons_Phi_slimmedGenJetsFlavourInfos = 0;
   cHadrons_Energy_slimmedGenJetsFlavourInfos = 0;
   bHadrons_PdgId_slimmedGenJetsFlavourInfos = 0;
   bHadrons_Pt_slimmedGenJetsFlavourInfos = 0;
   bHadrons_Eta_slimmedGenJetsFlavourInfos = 0;
   bHadrons_Phi_slimmedGenJetsFlavourInfos = 0;
   bHadrons_Energy_slimmedGenJetsFlavourInfos = 0;
   GenMuonPt = 0;
   GenMuonEta = 0;
   GenMuonPhi = 0;
   GenMuonEnergy = 0;
   GenMuonCharge = 0;
   PSWeights = 0;
   QCDScaleWeights = 0;
   PDFWeights = 0;
   LHEPartPt = 0;
   LHEPartEta = 0;
   LHEPartPhi = 0;
   LHEPartMass = 0;
   LHEPartEnergy = 0;
   LHEPartIncomingPz = 0;
   LHEPartPdgId = 0;
   LHEPartStatus = 0;
   LHEPartSpin = 0;
   GenPartonPt = 0;
   GenJetMatching = 0;
   GenDaughterMatching = 0;
   RecoJetMatching = 0;
   RecoDaughterMatching = 0;
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
   fChain->SetBranchAddress("RecoMuondxyPV", &RecoMuondxyPV, &b_RecoMuondxyPV);
   fChain->SetBranchAddress("RecoMuondxyErrPV", &RecoMuondxyErrPV, &b_RecoMuondxyErrPV);
   fChain->SetBranchAddress("RecoMuondxyBS", &RecoMuondxyBS, &b_RecoMuondxyBS);
   fChain->SetBranchAddress("RecoMuondxyErrBS", &RecoMuondxyErrBS, &b_RecoMuondxyErrBS);
   fChain->SetBranchAddress("RecoMuondzPV", &RecoMuondzPV, &b_RecoMuondzPV);
   fChain->SetBranchAddress("RecoMuondzErr", &RecoMuondzErr, &b_RecoMuondzErr);
   fChain->SetBranchAddress("RecoMuonip3d", &RecoMuonip3d, &b_RecoMuonip3d);
   fChain->SetBranchAddress("RecoMuonsip3d", &RecoMuonsip3d, &b_RecoMuonsip3d);
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
   fChain->SetBranchAddress("GenJetPt", &GenJetPt, &b_GenJetPt);
   fChain->SetBranchAddress("GenJetEta", &GenJetEta, &b_GenJetEta);
   fChain->SetBranchAddress("GenJetPhi", &GenJetPhi, &b_GenJetPhi);
   fChain->SetBranchAddress("GenJetEnergy", &GenJetEnergy, &b_GenJetEnergy);
   fChain->SetBranchAddress("GenJetEnergySumDaughters", &GenJetEnergySumDaughters, &b_GenJetEnergySumDaughters);
   fChain->SetBranchAddress("GenJetPartonFlavourHard", &GenJetPartonFlavourHard, &b_GenJetPartonFlavourHard);
   fChain->SetBranchAddress("GenJetPartonFlavourSoft", &GenJetPartonFlavourSoft, &b_GenJetPartonFlavourSoft);
   fChain->SetBranchAddress("GenJetHadronFlavour", &GenJetHadronFlavour, &b_GenJetHadronFlavour);
   fChain->SetBranchAddress("GenJetChargedMultiplicity", &GenJetChargedMultiplicity, &b_GenJetChargedMultiplicity);
   fChain->SetBranchAddress("GenJetNeutralMultiplicity", &GenJetNeutralMultiplicity, &b_GenJetNeutralMultiplicity);
   fChain->SetBranchAddress("GenDaughterPdgId", &GenDaughterPdgId, &b_GenDaughterPdgId);
   fChain->SetBranchAddress("GenDaughterJetId", &GenDaughterJetId, &b_GenDaughterJetId);
   fChain->SetBranchAddress("GenDaughterCharge", &GenDaughterCharge, &b_GenDaughterCharge);
   fChain->SetBranchAddress("GenDaughterPt", &GenDaughterPt, &b_GenDaughterPt);
   fChain->SetBranchAddress("GenDaughterEta", &GenDaughterEta, &b_GenDaughterEta);
   fChain->SetBranchAddress("GenDaughterPhi", &GenDaughterPhi, &b_GenDaughterPhi);
   fChain->SetBranchAddress("GenDaughterEnergy", &GenDaughterEnergy, &b_GenDaughterEnergy);
   fChain->SetBranchAddress("Partons_PdgId_prunedGenParticles", &Partons_PdgId_prunedGenParticles, &b_Partons_PdgId_prunedGenParticles);
   fChain->SetBranchAddress("Partons_Pt_prunedGenParticles", &Partons_Pt_prunedGenParticles, &b_Partons_Pt_prunedGenParticles);
   fChain->SetBranchAddress("Partons_Eta_prunedGenParticles", &Partons_Eta_prunedGenParticles, &b_Partons_Eta_prunedGenParticles);
   fChain->SetBranchAddress("Partons_Phi_prunedGenParticles", &Partons_Phi_prunedGenParticles, &b_Partons_Phi_prunedGenParticles);
   fChain->SetBranchAddress("Partons_Energy_prunedGenParticles", &Partons_Energy_prunedGenParticles, &b_Partons_Energy_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_PdgId_prunedGenParticles", &cHadrons_PdgId_prunedGenParticles, &b_cHadrons_PdgId_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_Pt_prunedGenParticles", &cHadrons_Pt_prunedGenParticles, &b_cHadrons_Pt_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_Eta_prunedGenParticles", &cHadrons_Eta_prunedGenParticles, &b_cHadrons_Eta_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_Phi_prunedGenParticles", &cHadrons_Phi_prunedGenParticles, &b_cHadrons_Phi_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_Energy_prunedGenParticles", &cHadrons_Energy_prunedGenParticles, &b_cHadrons_Energy_prunedGenParticles);
   fChain->SetBranchAddress("bHadrons_PdgId_prunedGenParticles", &bHadrons_PdgId_prunedGenParticles, &b_bHadrons_PdgId_prunedGenParticles);
   fChain->SetBranchAddress("bHadrons_Pt_prunedGenParticles", &bHadrons_Pt_prunedGenParticles, &b_bHadrons_Pt_prunedGenParticles);
   fChain->SetBranchAddress("bHadrons_Eta_prunedGenParticles", &bHadrons_Eta_prunedGenParticles, &b_bHadrons_Eta_prunedGenParticles);
   fChain->SetBranchAddress("bHadrons_Phi_prunedGenParticles", &bHadrons_Phi_prunedGenParticles, &b_bHadrons_Phi_prunedGenParticles);
   fChain->SetBranchAddress("bHadrons_Energy_prunedGenParticles", &bHadrons_Energy_prunedGenParticles, &b_bHadrons_Energy_prunedGenParticles);
   fChain->SetBranchAddress("cHadrons_PdgId_slimmedGenJetsFlavourInfos", &cHadrons_PdgId_slimmedGenJetsFlavourInfos, &b_cHadrons_PdgId_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("cHadrons_Pt_slimmedGenJetsFlavourInfos", &cHadrons_Pt_slimmedGenJetsFlavourInfos, &b_cHadrons_Pt_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("cHadrons_Eta_slimmedGenJetsFlavourInfos", &cHadrons_Eta_slimmedGenJetsFlavourInfos, &b_cHadrons_Eta_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("cHadrons_Phi_slimmedGenJetsFlavourInfos", &cHadrons_Phi_slimmedGenJetsFlavourInfos, &b_cHadrons_Phi_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("cHadrons_Energy_slimmedGenJetsFlavourInfos", &cHadrons_Energy_slimmedGenJetsFlavourInfos, &b_cHadrons_Energy_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("bHadrons_PdgId_slimmedGenJetsFlavourInfos", &bHadrons_PdgId_slimmedGenJetsFlavourInfos, &b_bHadrons_PdgId_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("bHadrons_Pt_slimmedGenJetsFlavourInfos", &bHadrons_Pt_slimmedGenJetsFlavourInfos, &b_bHadrons_Pt_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("bHadrons_Eta_slimmedGenJetsFlavourInfos", &bHadrons_Eta_slimmedGenJetsFlavourInfos, &b_bHadrons_Eta_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("bHadrons_Phi_slimmedGenJetsFlavourInfos", &bHadrons_Phi_slimmedGenJetsFlavourInfos, &b_bHadrons_Phi_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("bHadrons_Energy_slimmedGenJetsFlavourInfos", &bHadrons_Energy_slimmedGenJetsFlavourInfos, &b_bHadrons_Energy_slimmedGenJetsFlavourInfos);
   fChain->SetBranchAddress("GenMuonPt", &GenMuonPt, &b_GenMuonPt);
   fChain->SetBranchAddress("GenMuonEta", &GenMuonEta, &b_GenMuonEta);
   fChain->SetBranchAddress("GenMuonPhi", &GenMuonPhi, &b_GenMuonPhi);
   fChain->SetBranchAddress("GenMuonEnergy", &GenMuonEnergy, &b_GenMuonEnergy);
   fChain->SetBranchAddress("GenMuonCharge", &GenMuonCharge, &b_GenMuonCharge);
   fChain->SetBranchAddress("GeneratorWeight", &GeneratorWeight, &b_GeneratorWeight);
   fChain->SetBranchAddress("PSWeights", &PSWeights, &b_PSWeights);
   fChain->SetBranchAddress("QCDScaleWeights", &QCDScaleWeights, &b_QCDScaleWeights);
   fChain->SetBranchAddress("PDFWeights", &PDFWeights, &b_PDFWeights);
   fChain->SetBranchAddress("LHEPartPt", &LHEPartPt, &b_LHEPartPt);
   fChain->SetBranchAddress("LHEPartEta", &LHEPartEta, &b_LHEPartEta);
   fChain->SetBranchAddress("LHEPartPhi", &LHEPartPhi, &b_LHEPartPhi);
   fChain->SetBranchAddress("LHEPartMass", &LHEPartMass, &b_LHEPartMass);
   fChain->SetBranchAddress("LHEPartEnergy", &LHEPartEnergy, &b_LHEPartEnergy);
   fChain->SetBranchAddress("LHEPartIncomingPz", &LHEPartIncomingPz, &b_LHEPartIncomingPz);
   fChain->SetBranchAddress("LHEPartPdgId", &LHEPartPdgId, &b_LHEPartPdgId);
   fChain->SetBranchAddress("LHEPartStatus", &LHEPartStatus, &b_LHEPartStatus);
   fChain->SetBranchAddress("LHEPartSpin", &LHEPartSpin, &b_LHEPartSpin);
   fChain->SetBranchAddress("GenPartonPt", &GenPartonPt, &b_GenPartonPt);
   fChain->SetBranchAddress("PreFiringWeightNm", &PreFiringWeightNm, &b_PreFiringWeightNm);
   fChain->SetBranchAddress("PreFiringWeightUp", &PreFiringWeightUp, &b_PreFiringWeightUp);
   fChain->SetBranchAddress("PreFiringWeightDn", &PreFiringWeightDn, &b_PreFiringWeightDn);
   fChain->SetBranchAddress("RunID", &RunID, &b_RunID);
   fChain->SetBranchAddress("EventID", &EventID, &b_EventID);
   fChain->SetBranchAddress("LumiID", &LumiID, &b_LumiID);
   fChain->SetBranchAddress("RecoPassDijet", &RecoPassDijet, &b_RecoPassDijet);
   fChain->SetBranchAddress("RecoPassZJet", &RecoPassZJet, &b_RecoPassZJet);
   fChain->SetBranchAddress("NumberPrimaryVertex", &NumberPrimaryVertex, &b_NumberPrimaryVertex);
   fChain->SetBranchAddress("NumberGoodVertex", &NumberGoodVertex, &b_NumberGoodVertex);
   fChain->SetBranchAddress("CrossSection", &CrossSection, &b_CrossSection);
   fChain->SetBranchAddress("NumberInteractionPileup", &NumberInteractionPileup, &b_NumberInteractionPileup);
   fChain->SetBranchAddress("NumberTruePileup", &NumberTruePileup, &b_NumberTruePileup);
   fChain->SetBranchAddress("PileupMaxPtHat", &PileupMaxPtHat, &b_PileupMaxPtHat);
   fChain->SetBranchAddress("GenPassDijet", &GenPassDijet, &b_GenPassDijet);
   fChain->SetBranchAddress("GenJetMatching", &GenJetMatching, &b_GenJetMatching);
   fChain->SetBranchAddress("GenDaughterMatching", &GenDaughterMatching, &b_GenDaughterMatching);
   fChain->SetBranchAddress("RecoJetMatching", &RecoJetMatching, &b_RecoJetMatching);
   fChain->SetBranchAddress("RecoDaughterMatching", &RecoDaughterMatching, &b_RecoDaughterMatching);
   fChain->SetBranchAddress("TotalEventNumber", &TotalEventNumber, &b_TotalEventNumber);
   fChain->SetBranchAddress("NextPassedNumber", &NextPassedNumber, &b_NextPassedNumber);
   Notify();
}

bool MCJetsAndDaughters::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be for a new TTree in a TChain. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void MCJetsAndDaughters::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t MCJetsAndDaughters::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
