//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Jul 24 10:51:14 2026 by ROOT version 6.40.02
// from TTree CharmoniumInfo/CharmoniumInfo
// found on file: root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/Jpsi/HardQCD_Pt15to7000_Oniaoff_CR0/pythia8_jpsi_events_20.root
//////////////////////////////////////////////////////////

#ifndef CharmoniumInfo_h
#define CharmoniumInfo_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"

class CharmoniumInfo {
public :
   TTree          *fChain;   ///<!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; ///<!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        generatorweight;
   vector<double>  *ps_pt;
   vector<double>  *ps_eta;
   vector<double>  *ps_phi;
   vector<double>  *ps_e;
   vector<int>     *ps_pdgid;
   vector<double>  *ps_charge;
   vector<double>  *hadron_pt;
   vector<double>  *hadron_eta;
   vector<double>  *hadron_phi;
   vector<double>  *hadron_e;
   vector<int>     *hadron_pdgid;
   vector<double>  *hadron_charge;
   vector<int>     *hadron_from_jpsi;
   Double_t        jpsi_pt;
   Double_t        jpsi_eta;
   Double_t        jpsi_phi;
   Double_t        jpsi_e;
   Double_t        jpsi_mass;
   Int_t           jpsi_pdgid;
   Bool_t          has_mother;
   Double_t        mother_pt;
   Double_t        mother_eta;
   Double_t        mother_phi;
   Double_t        mother_e;
   Double_t        mother_mass;
   Int_t           mother_pdgid;
   Int_t           grandmother_pdgid;

   // List of branches
   TBranch        *b_generatorweight;   ///<!
   TBranch        *b_ps_pt;   ///<!
   TBranch        *b_ps_eta;   ///<!
   TBranch        *b_ps_phi;   ///<!
   TBranch        *b_ps_e;   ///<!
   TBranch        *b_ps_pdgid;   ///<!
   TBranch        *b_ps_charge;   ///<!
   TBranch        *b_hadron_pt;   ///<!
   TBranch        *b_hadron_eta;   ///<!
   TBranch        *b_hadron_phi;   ///<!
   TBranch        *b_hadron_e;   ///<!
   TBranch        *b_hadron_pdgid;   ///<!
   TBranch        *b_hadron_charge;   ///<!
   TBranch        *b_hadron_from_jpsi;   ///<!
   TBranch        *b_jpsi_pt;   ///<!
   TBranch        *b_jpsi_eta;   ///<!
   TBranch        *b_jpsi_phi;   ///<!
   TBranch        *b_jpsi_e;   ///<!
   TBranch        *b_jpsi_mass;   ///<!
   TBranch        *b_jpsi_pdgid;   ///<!
   TBranch        *b_has_mother;   ///<!
   TBranch        *b_mother_pt;   ///<!
   TBranch        *b_mother_eta;   ///<!
   TBranch        *b_mother_phi;   ///<!
   TBranch        *b_mother_e;   ///<!
   TBranch        *b_mother_mass;   ///<!
   TBranch        *b_mother_pdgid;   ///<!
   TBranch        *b_grandmother_pdgid;   ///<!

   CharmoniumInfo(TTree *tree=0);
   virtual ~CharmoniumInfo();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef CharmoniumInfo_cxx
CharmoniumInfo::CharmoniumInfo(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/Jpsi/HardQCD_Pt15to7000_Oniaoff_CR0/pythia8_jpsi_events_20.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("root://eoscms.cern.ch//eos/cms/store/group/phys_smp/ec/shuangyu/Jpsi/HardQCD_Pt15to7000_Oniaoff_CR0/pythia8_jpsi_events_20.root");
      }
      f->GetObject("CharmoniumInfo",tree);

   }
   Init(tree);
}

CharmoniumInfo::~CharmoniumInfo()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t CharmoniumInfo::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t CharmoniumInfo::LoadTree(Long64_t entry)
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

void CharmoniumInfo::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.

   // Set object pointer
   ps_pt = 0;
   ps_eta = 0;
   ps_phi = 0;
   ps_e = 0;
   ps_pdgid = 0;
   ps_charge = 0;
   hadron_pt = 0;
   hadron_eta = 0;
   hadron_phi = 0;
   hadron_e = 0;
   hadron_pdgid = 0;
   hadron_charge = 0;
   hadron_from_jpsi = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("generatorweight", &generatorweight, &b_generatorweight);
   fChain->SetBranchAddress("ps_pt", &ps_pt, &b_ps_pt);
   fChain->SetBranchAddress("ps_eta", &ps_eta, &b_ps_eta);
   fChain->SetBranchAddress("ps_phi", &ps_phi, &b_ps_phi);
   fChain->SetBranchAddress("ps_e", &ps_e, &b_ps_e);
   fChain->SetBranchAddress("ps_pdgid", &ps_pdgid, &b_ps_pdgid);
   fChain->SetBranchAddress("ps_charge", &ps_charge, &b_ps_charge);
   fChain->SetBranchAddress("hadron_pt", &hadron_pt, &b_hadron_pt);
   fChain->SetBranchAddress("hadron_eta", &hadron_eta, &b_hadron_eta);
   fChain->SetBranchAddress("hadron_phi", &hadron_phi, &b_hadron_phi);
   fChain->SetBranchAddress("hadron_e", &hadron_e, &b_hadron_e);
   fChain->SetBranchAddress("hadron_pdgid", &hadron_pdgid, &b_hadron_pdgid);
   fChain->SetBranchAddress("hadron_charge", &hadron_charge, &b_hadron_charge);
   fChain->SetBranchAddress("hadron_from_jpsi", &hadron_from_jpsi, &b_hadron_from_jpsi);
   fChain->SetBranchAddress("jpsi_pt", &jpsi_pt, &b_jpsi_pt);
   fChain->SetBranchAddress("jpsi_eta", &jpsi_eta, &b_jpsi_eta);
   fChain->SetBranchAddress("jpsi_phi", &jpsi_phi, &b_jpsi_phi);
   fChain->SetBranchAddress("jpsi_e", &jpsi_e, &b_jpsi_e);
   fChain->SetBranchAddress("jpsi_mass", &jpsi_mass, &b_jpsi_mass);
   fChain->SetBranchAddress("jpsi_pdgid", &jpsi_pdgid, &b_jpsi_pdgid);
   fChain->SetBranchAddress("has_mother", &has_mother, &b_has_mother);
   fChain->SetBranchAddress("mother_pt", &mother_pt, &b_mother_pt);
   fChain->SetBranchAddress("mother_eta", &mother_eta, &b_mother_eta);
   fChain->SetBranchAddress("mother_phi", &mother_phi, &b_mother_phi);
   fChain->SetBranchAddress("mother_e", &mother_e, &b_mother_e);
   fChain->SetBranchAddress("mother_mass", &mother_mass, &b_mother_mass);
   fChain->SetBranchAddress("mother_pdgid", &mother_pdgid, &b_mother_pdgid);
   fChain->SetBranchAddress("grandmother_pdgid", &grandmother_pdgid, &b_grandmother_pdgid);
   Notify();
}

bool CharmoniumInfo::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be for a new TTree in a TChain. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void CharmoniumInfo::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t CharmoniumInfo::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef CharmoniumInfo_cxx
