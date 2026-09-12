#pragma once

#include <string>
#include <cstdlib>

// Category determination based on PDG ID
// Same logic as in pythia.cpp
inline int getCategory(int pdgid)
{
  int absid = std::abs(pdgid);
  if (absid / 100 % 10 == 5 || absid / 1000 % 10 == 5)
    return 1; // b hadron
  else if (absid == 441 || absid == 445 || absid == 100443 || absid == 20443 ||
           absid == 10441 || absid == 30443 || absid == 10443)
    return 2; // c hadron feed-down
  else if (absid == 21 || absid == 2212)
    return 3; // gluon or proton
  else if (absid >= 1 && absid <= 5)
    return 4; // quark
  else if (absid == 9940003)
    return 5; // J/psi[3S1(8)]
  else if (absid == 9941003)
    return 6; // J/psi[1S0(8)]
  else if (absid == 9942003)
    return 7; // J/psi[3PJ(8)]
  else
    return 0; // other
}

// Suffix for histogram names, e.g. "_cat0", "_cat1"
inline std::string getCategorySuffix(int cat)
{
  return "_cat" + std::to_string(cat);
}

// Human-readable category name
inline std::string getCategoryName(int cat)
{
  switch (cat)
  {
  case 0:
    return "Other";
  case 1:
    return "bhadron";
  case 2:
    return "chadron";
  case 3:
    return "gluon/proton";
  case 4:
    return "quark";
  case 5:
    return "3S18";
  case 6:
    return "1S08";
  case 7:
    return "3PJ8";
  default:
    return "Unknown";
  }
}

// Number of categories (0 through 7)
inline int nCategories() { return 8; }