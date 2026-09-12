#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TString.h"
#include <iostream>
#include <vector>
#include <glob.h>
#include <algorithm>
#include <cmath>

int main(int argc, char *argv[])
{
  TString input_file = "";
  TString output_path = "";
  int chunknum = 2;
  int chunkindex = 0;
  int opt;

  while ((opt = getopt(argc, argv, "i:o:n:e:")) != -1)
  {
    switch (opt)
    {
    case 'i': input_file = optarg; break;
    case 'o': output_path = optarg; break;
    case 'n': chunknum = std::atoi(optarg); break;
    case 'e': chunkindex = std::atoi(optarg); break;
    default:
      std::cerr << "Usage: " << argv[0]
                << " -i <input_dir> -o <output_path> -n <chunk_num> -e <chunk_index>"
                << std::endl;
      return 1;
    }
  }
  if (input_file == "")
  {
    std::cerr << "Error: Input file not specified!" << std::endl;
    return 1;
  }

  // 获取文件列表
  std::string pattern = (input_file + "/*.root").Data();
  glob_t glob_result;
  glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
  std::vector<std::string> fileList;
  for (size_t i = 0; i < glob_result.gl_pathc; ++i)
    fileList.push_back(std::string(glob_result.gl_pathv[i]) + "/JetsAndDaughters");
  globfree(&glob_result);

  if (fileList.empty())
  {
    std::cerr << "No matching files found!" << std::endl;
    return 1;
  }

  std::sort(fileList.begin(), fileList.end());
  int nTotalFiles = fileList.size();
  int filesPerChunk = std::ceil(static_cast<double>(nTotalFiles) / chunknum);
  int fileBegin = chunkindex * filesPerChunk;
  int fileEnd = std::min((chunkindex + 1) * filesPerChunk, nTotalFiles);

  if (fileBegin >= nTotalFiles)
  {
    std::cout << "Chunk index " << chunkindex << " out of range, nothing to do." << std::endl;
    return 0;
  }

  // 累加变量
  double totalEvents = 0.0;
  double generatedEvents = 0.0;

  // 文件循环
  for (int fIdx = fileBegin; fIdx < fileEnd; ++fIdx)
  {
    TChain chain("JetsAndDaughters");
    chain.Add(fileList[fIdx].c_str());

    // 只激活需要的分支
    chain.SetBranchStatus("*", 0);
    chain.SetBranchStatus("TotalEventNumber", 1);
    chain.SetBranchStatus("NextPassedNumber", 1);

    Double_t totalEv = 0.0;
    Int_t nextPassed = 0;
    chain.SetBranchAddress("TotalEventNumber", &totalEv);
    chain.SetBranchAddress("NextPassedNumber", &nextPassed);

    Long64_t nEntries = chain.GetEntries();
    if (nEntries > 0)
    {
      // 读取第一个事件获取文件级 TotalEventNumber
      chain.GetEntry(0);
      totalEvents += totalEv;

      // 遍历所有事件统计 NextPassedNumber == 1 的数量
      for (Long64_t k = 0; k < nEntries; ++k)
      {
        chain.GetEntry(k);
        if (nextPassed == 1)
          generatedEvents += 1.0;   // 如果 NextPassedNumber 可能 >1，可改为 += nextPassed
      }
    }
  }

  // 打印到屏幕
  std::cout << "Chunk " << chunkindex << " summary:" << std::endl;
  std::cout << "  TotalEvents (from TotalEventNumber): " << totalEvents << std::endl;
  std::cout << "  GeneratedEvents (events with NextPassedNumber==1): " << generatedEvents << std::endl;

  // ---------- 写入 ROOT 文件 ----------
  TString outFileName = output_path + TString::Format("_Chunk%d_Summary.root", chunkindex);
  TFile *outFile = TFile::Open(outFileName, "RECREATE");
  if (!outFile || outFile->IsZombie())
  {
    std::cerr << "Error: Cannot create output file " << outFileName << std::endl;
    return 1;
  }

  // 用两个直方图存储数值（每个只有1个bin）
  TH1D *hTotal = new TH1D("TotalEvents", "TotalEvents", 1, 0, 1);
  hTotal->SetBinContent(1, totalEvents);
  hTotal->Write();

  TH1D *hGen = new TH1D("GeneratedEvents", "GeneratedEvents", 1, 0, 1);
  hGen->SetBinContent(1, generatedEvents);
  hGen->Write();

  // 也可以直接写入 TTree（可选），这里用直方图更简单
  outFile->Close();
  delete outFile;

  std::cout << "Summary written to " << outFileName << std::endl;

  return 0;
}
