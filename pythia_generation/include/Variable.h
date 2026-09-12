#pragma once
#include <vector>
#include <TString.h>
#include <string>
#include "TFile.h"
#include <filesystem>

// namespace fs = std::filesystem;
class VariableInfo
{
public:
  TString variable;
  TString name;
  TString histset;
  int bins;
  double min;
  double max;
  double ymin;
  double ymax;
  VariableInfo(const TString &variable, int bins, double min, double max)
      : variable(variable), bins(bins), min(min), max(max)
  {
    name = variable;
    histset = TString::Format("(%d, %f, %f)", bins, min, max);
  }
  VariableInfo(const TString &variable, int bins, double min, double max, const TString &name)
      : variable(variable), bins(bins), min(min), max(max), name(name)
  {
    histset = TString::Format("(%d, %f, %f)", bins, min, max);
  }
  VariableInfo(const TString &variable, int bins, double min, double max, double ymin, double ymax, const TString &name)
      : variable(variable), bins(bins), min(min), max(max), ymin(ymin), ymax(ymax), name(name)
  {
    histset = TString::Format("(%d, %f, %f)", bins, min, max);
  }
};
class Selection
{
public:
  std::vector<TString> selects;
  std::vector<double> ranges;
  TString variable;
  Selection(TString variable_, std::vector<double> ranges_, int iscut = 0)
      : variable(variable_), ranges(ranges_)
  {
    if (iscut == 1)
    {
      for (int i = 0; i < ranges.size(); i++)
      {
        TString select = "(" + variable + ">" + TString::Format("%f", ranges.at(i)) + ")";
        selects.push_back(select);
      }
    }
    else if (iscut == -1)
    {
      for (int i = 0; i < ranges.size(); i++)
      {
        TString select = "(" + variable + "<" + TString::Format("%f", ranges.at(i)) + ")";
        selects.push_back(select);
      }
    }
    else
    {
      for (int i = 0; i < ranges.size() - 1; i++)
      {
        // if (ranges.at(i) > ranges.at(i + 1))
        //   continue;
        TString select = "(" + variable + ">" + TString::Format("%f", ranges.at(i)) + ")*(" + variable + "<" + TString::Format("%f", ranges.at(i + 1)) + ")";
        selects.push_back(select);
      }
    }
  }
};

TString remove_substring(const TString &original, const TString &substring)
{
  TString modified_string = original;
  int pos;
  while ((pos = modified_string.Index(substring)) != kNPOS)
  {
    modified_string.Remove(pos, substring.Length());
  }
  return modified_string;
}

TString ReplaceAllSubstrings(const TString &original, const TString &toReplace, const TString &replacement)
{
  TString result = original;
  int pos = 0;

  while ((pos = result.Index(toReplace, pos)) != -1)
  {
    result.Remove(pos, toReplace.Length());
    result.Insert(pos, replacement);
    pos += replacement.Length(); // 移动位置到替换后的位置
  }

  return result;
}
// void SafeCreateDirectory(const std::string &path)
// {
//   try
//   {
//     if (fs::exists(path))
//     {
//       if (!fs::is_directory(path))
//       {
//         throw std::runtime_error("路径被文件占用: " + path);
//       }
//       return;
//     }
//     bool created = fs::create_directories(path);

//     if (created)
//     {
//       std::cout << "成功创建: " << path << std::endl;
//     }
//     else
//     {
//       std::cout << "路径已被其他进程创建" << std::endl;
//     }
//   }
//   catch (const std::exception &e)
//   {
//     std::cerr << "[ERROR] " << e.what() << std::endl;
//   }
// }
