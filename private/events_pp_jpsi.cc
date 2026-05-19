/*
    top_events.cc
    ~~~~~~~~~~~~~~~~~~~~~
    A tool of generating top quark events using PYTHIA8, and save them to our
    standard data structure.
    :discription:
    An example of generating events using PYTHIA8, contents include: jet
    clustering, jet parton flavour tagging, change parton shower alphaS,
    hadronization cli entrance, transform pythia data to standard data
    structure.
    :note: PYTHIA8 and other libs should be proper installed. Checkout the
    "dependencies.sh" inside this folder.

    :compile: g++ -std=c++17 -g events.cc GeneratorHelper.cc
    JetTupleFactory.cc ./fjcore-3.3.4/fjcore.cc ./pythia8/lib/libpythia8.a -o
    events.o  -I ./pythia8/include -I ./fjcore-3.3.4 `root-config --cflags
    --libs`

    :author: Yulei Ye
    :email: yulei.ye@cern.ch
*/

#include <Pythia8/Pythia.h>
#include <TFile.h>
#include <TTree.h>
#include <unistd.h> // getopt

#include <fjcore/fjcore.hh>
#include <iostream>
#include <string>

#include "/storage/shuangyuan/code/headfile/Process_bar.h"
#include "fastjet/ClusterSequence.hh"
#include "GeneratorHelper.cc"
#include "GlobalHelper.cc"
#include "/storage/shuangyuan/code/QCDAnalysis/OfflineExamples/src/pythia_branch.cc"
#include "TLorentzVector.h"

using namespace Pythia8;
// using namespace fastjet;
// g++ analysis/simulation/events_pp_jet.cc /storage/shuangyuan/.local/lib/libpythia8.a /storage/shuangyuan/.local/include/fjcore/fjcore.cc -o analysis/simulation/events_pp_jet.o -Isrc -std=c++17 -I/usr/local/include -I/storage/shuangyuan/.local/include -L/usr/local/lib -L/storage/shuangyuan/.local/lib -ldl -lstdc++fs $(root-config --cflags --libs) `/storage/shuangyuan/code/CA_cluster/fastjet-install/bin/fastjet-config --cxxflags --libs --plugins`

int main(int argc, char *argv[])
{
  int nEvent = 8000;
  std::string alphaS = "0.118";
  // std::string hadronization = "off";
  std::string mpi = "on";
  std::string seed = "0"; // Random seed according to time
  int njet = 100;
  double R = 0.4;
  double etaMax = 2.1;
  int power = -1;
  int select = 1;
  int massSet = 2;
  int nChunk = 1;
  std::string folder = "/storage/shuangyuan/code/analysis_spin/dataset/";
  std::string prefix = "parton_qq_pythia_jet";
  std::string tune = "CP5";
  std::vector<int> ptRange = {-1, -1};
  std::vector<std::string> spin_correlation = {"off", "on"}; // spin correlation
  std::vector<std::string> decay_flavour = {"Mixed", "Quark", "Gluon"};
  std::vector<std::string> hadronization = {"off", "on"};
  int spin_cor_switch = 1; // spin correlation 0: off 1: on
  int decay_select = 0;    // decay channel 0: mixed jet 1: quark jet 2: gluon quark
  int hadron_switch = 0;   // hadronization 0: off 1: on
  int b_decay = 1;         // bhadron_decay 0: off 1: on
  int opt;                 // read parameters from user inputs

  while ((opt = getopt(argc, argv, "a:n:s:l:r:t:x:y:e:f:c:h:m:b:j:u:o:")) != -1)
  {
    switch (opt)
    {
    case 'a': // prefix of source dataset
      prefix = optarg;
      break;
    case 'o':
      folder = optarg;
      break;
    case 'n': // number of events
      nEvent = std::stoi(optarg);
      break;
    case 's': // sigma value of alphaS, defualt is 0.01
      alphaS = optarg;
      break;
    case 'l':
      ptRange[0] = std::stoi(optarg);
      break;
    case 'r':
      ptRange[1] = std::stoi(optarg);
      break;
    case 't': // only keey leading and sub-leading jet
      njet = std::stoi(optarg);
      break;
    case 'x': // seed number, should be greater than 0
      seed = optarg;
      break;
    case 'h': // hadronization on or off
      hadron_switch = std::stoi(optarg);
      break;
    case 'm': // mpi on or off
      mpi = "off";
      break;
    case 'e': // nChunk
      nChunk = std::stoi(optarg);
      break;
    case 'f': // nChunk
      decay_select = std::stoi(optarg);
      break;
    case 'c':
      spin_cor_switch = std::stoi(optarg);
      break;
    case 'b':
      b_decay = std::stoi(optarg);
      break;
    case 'j': // jet cluster radius
      R = std::stod(optarg);
      break;
    case 'u': // tune
      tune = optarg;
      break;
    }
  }
  double pTMin = 30; // Min jet pT.

  // remove annoying pythia and fastjet hello message and initialize
  // std::cout.setstate(std::ios::failbit);
  Pythia pythia;
  // pythia.readString("Main:writeHepMC = on");
  // todo: figure out how random seed affect result
  pythia.readString("Random:setSeed = on");
  pythia.readString("Random:seed = " + seed);
  if (seed != "0")
    std::cout << "using random seed: " << seed << std::endl;
  std::cout << "using jet cluster radius: " << R << std::endl;
  // std::cout.clear();
  // initialize generator, LHC
  double ecm = 13600;
  // GeneratorHelper::Configee(&pythia, pTMin);
  // GeneratorHelper::ConfigCMSCP5(&pythia);
  GeneratorHelper::ConfigCMSPT(&pythia, ptRange[0], ptRange[1],
                               hadronization[hadron_switch],
                               decay_flavour[decay_select], tune);
  pythia.readString("PDF:lepton = off");
  pythia.readString("Beams:idA =  2212");
  pythia.readString("Beams:idB =  2212");
  pythia.settings.parm("Beams:eCM", ecm);
  // pythia.readString("TimeShower:weightGluonToQuark = 5");
  // pythia.readString("TimeShower:scaleGluonToQuark = 0.25");
  // pythia.readString("PartonShowers:model = 3");

  GeneratorHelper::ShowerAlphaS(&pythia, "Simple", alphaS);
  pythia.readString("PartonLevel:MPI = " + mpi);
  pythia.readString("Next:numberShowEvent = 1");
  pythia.readString("Next:numberCount = 0");
  if (spin_cor_switch == 0)
  {
    pythia.readString("TimeShower:phiPolAsym = off");
  }
  // pythia.readString("SpaceShower:phiPolAsym = off");
  // pythia.readString("TimeShower:globalRecoil = on");
  // pythia.readString("ColourReconnection:mode = 1");
  // pythia.readString("BeamRemnants:remnantMode = 1");

  std::vector<std::string> bbaryon = {
      "5122", "5112", "5212", "5222", "5114", "5214", "5224", "5132", "5232",
      "5312", "5322", "5314", "5324", "5332", "5334", "5142", "5242", "5412",
      "5422", "5414", "5424", "5342", "5432", "5434", "5442", "5444"};
  std::vector<std::string> bmeson = {
      "511", "521", "10511", "10521", "513", "523", "10513", "10523",
      "20513", "20523", "515", "525", "531", "10531", "533", "10533",
      "20533", "535", "541", "10541", "543", "10543", "20543", "545"};
  if (b_decay == 0)
  {
    for (int i = 0; i < bbaryon.size(); i++)
    {
      // cout << bhadron.at(i) + ":onMode = off" << endl;
      pythia.readString(bbaryon.at(i) + ":onMode = off");
      pythia.readString("-" + bbaryon.at(i) + ":onMode = off");
    }
    for (int i = 0; i < bmeson.size(); i++)
    {
      // cout << bhadron.at(i) + ":onMode = off" << endl;
      pythia.readString(bmeson.at(i) + ":onMode = off");
      pythia.readString("-" + bmeson.at(i) + ":onMode = off");
    }
  }

  if (ptRange[0] == 15 && ptRange[1] == 7000)
  {
    pythia.readString("PhaseSpace:bias2Selection = on");
    pythia.readString("PhaseSpace:bias2SelectionPow = 4.5");
    pythia.readString("PhaseSpace:bias2SelectionRef = 15.");
  }

  // J/psi decay
  // pythia.readString("443:onMode = off");
  // pythia.readString("443:onIfMatch = 13 -13");

  // QED
  // pythia.readString("SpaceShower:QEDshowerByQ = off");
  // pythia.readString("SpaceShower:QEDshowerByL = off");
  // pythia.readString("TimeShower:QEDshowerByQ = off");
  // pythia.readString("TimeShower:QEDshowerByL = off");

  // pion
  // pythia.readString("111:onMode = off");

  pythia.settings.list("TimeShower:phiPolAsym");
  pythia.settings.list("TimeShower:globalRecoil");
  pythia.settings.list("HadronLevel:all");
  pythia.init();

  // auto filename = GlobalHelper::GetPythiaFilename(
  //     ptRange, alphaS, "Top", "Mixed", "Simple", hadronization, chunk);
  // auto filename = GlobalHelper::GetPythiaFilename(ptRange, alphaS,"Parton"
  // ,"Mixed",
  //                                              "Simple", hadronization);
  // filename = GlobalHelper::WrapWithDirname(prefix, filename);
  // GlobalHelper::CreateDirIfNotExisted(filename);
  // std::cout << "Output filename: " << filename << std::endl;

  TString filename = folder + prefix +
                     TString::Format("/Chunk%d/Part_pt%d_%d_Ak%d.root", nChunk,
                                     ptRange[0], ptRange[1], int(R * 10.0));
  auto file = new TFile(filename, "RECREATE");
  auto tree =
      new TTree("JetsAndDaughters", "Informations of Jets and Daughters");
  cout << filename << " " << nChunk << endl;
  auto tuples = JetTupleFactory(tree);
  ProcessBar ProcessBar(nEvent);

  int nValidEvent = 0; // 记录攒了多少个包含 J/psi 的有效事件
  int nTotalTried = 0; // 选填：记录总共读取了多少个事件（防止死循环或文件读完）

  // 改用 while 循环，只有当有效事件达到 nEvent 时才停止
  while (nValidEvent < nEvent)
  {
    if (!pythia.next())
    {
      if (pythia.info.atEndOfFile())
      {
        std::cout << "Warning: Reached end of file before finding " << nEvent << " J/psi events!" << std::endl;
        break;
      }
      else
        continue;
    }

    nTotalTried++;
    ProcessBar.show2(nValidEvent);

    auto partons = std::vector<Pythia8::Particle>{};
    auto hadrons = std::vector<Pythia8::Particle>{};
    auto jpsis = std::vector<Pythia8::Particle>{};
    std::vector<int> isprompt;
    std::vector<int> isprompt2;

    for (int i = 0; i < pythia.event.size(); i++)
    {
      if (abs(pythia.event[i].status()) >= 62 && abs(pythia.event[i].status()) <= 69)
      {
        partons.push_back(pythia.event[i]);
      }
      if (pythia.event[i].isFinal())
      {
        hadrons.push_back(pythia.event[i]);
      }
      if (abs(pythia.event[i].id()) == 443)
      {
        jpsis.push_back(pythia.event[i]);
        int mother1 = pythia.event[i].mother1();
        int mother2 = pythia.event[i].mother2();

        int mother_pdgid = pythia.event[mother1].id();
        if (abs(mother_pdgid) == 4)
          isprompt.push_back(1);
        else if (abs(mother_pdgid) / 100 % 10 == 4 || abs(mother_pdgid) / 1000 % 10 == 4)
          isprompt.push_back(2);
        else if (abs(mother_pdgid) / 100 % 10 == 5 || abs(mother_pdgid) / 1000 % 10 == 5)
          isprompt.push_back(3);
        else
          isprompt.push_back(0);

        if (abs(mother_pdgid) / 100 % 10 == 5 || abs(mother_pdgid) / 1000 % 10 == 5)
          isprompt2.push_back(1);
        else if (abs(mother_pdgid) == 441 || abs(mother_pdgid) == 445 || abs(mother_pdgid) == 100443 || abs(mother_pdgid) == 20443 ||
                 abs(mother_pdgid) == 10441 || abs(mother_pdgid) == 30443 || abs(mother_pdgid) == 10443)
          isprompt2.push_back(2);
        else if (abs(mother_pdgid) == 21 || abs(mother_pdgid) == 2212)
          isprompt2.push_back(3);
        else if (abs(mother_pdgid) >= 1 && abs(mother_pdgid) <= 5)
          isprompt2.push_back(4);
        else
          isprompt2.push_back(0);

        auto daughter1 = pythia.event[pythia.event[i].daughter1()];
        auto daughter2 = pythia.event[pythia.event[i].daughter2()];
      }
    }

    if (jpsis.size() == 0)
      continue;

    nValidEvent++;

    tuples.FillParticleInfos(&pythia, partons, hadrons, jpsis, isprompt, isprompt2, pythia.info.weight());
    tree->Fill();
    tuples.ResetToDefaults();
  }
  std::cout << "Tried to read " << nTotalTried << " events to find " << nValidEvent << " valid J/psi events." << std::endl;
  std::cout << "Successfully collected " << nValidEvent << " events containing J/psi." << std::endl;
  // fill cross section and errors
  int nGen = pythia.info.nAccepted();
  double xsecVal =
      pythia.info.sigmaGen() * 1e9; // default units is mb, convert to pb
  // 1b=1e-28m2, milli: 1e-3, micro: 1e-6, nano: 1e-9, pico: 1e-12
  double xsecErr = pythia.info.sigmaErr() * 1e9;
  auto bVal = tree->Branch("CrossSection", &xsecVal);
  auto bErr = tree->Branch("CrossSectionError", &xsecErr);
  auto bGen = tree->Branch("NumberGenerated", &nGen);
  bVal->Fill();
  bErr->Fill();
  bGen->Fill();

  pythia.stat(); // print cross section ant statisticals of pythia results
  tree->Write();
  file->Close();

  std::cout << "Task event generation finish." << std::endl
            << std::endl;
}
