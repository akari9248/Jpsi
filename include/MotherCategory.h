#ifndef JPSI_MOTHER_CATEGORY_H
#define JPSI_MOTHER_CATEGORY_H

#include <array>
#include <cstdlib>
#include <string>

namespace jpsi_origin {

inline constexpr int numberOfCategories() { return 8; }

inline bool isBHadron(int pdgId) {
  const int absoluteId = std::abs(pdgId);
  return absoluteId / 100 % 10 == 5 || absoluteId / 1000 % 10 == 5;
}

inline constexpr std::array<int, 7> charmoniumPdgIds() {
  return {441, 10441, 20443, 445, 10443, 100443, 30443};
}

inline int charmoniumType(int pdgId) {
  const int absoluteId = std::abs(pdgId);
  const auto ids = charmoniumPdgIds();
  for (std::size_t index = 0; index < ids.size(); ++index)
    if (absoluteId == ids[index])
      return static_cast<int>(index);
  return -1;
}

inline constexpr int numberOfCharmoniumComponents() { return 5; }

inline int charmoniumComponent(int motherPdgId, int grandmotherPdgId) {
  if (isBHadron(grandmotherPdgId))
    return 0;
  switch (std::abs(motherPdgId)) {
  case 100443:
    return 1; // psi(2S)
  case 20443:
    return 2; // chi_c1
  case 445:
    return 3; // chi_c2
  default:
    return 4; // remaining charmonium feed-down
  }
}

inline const std::array<const char *, numberOfCharmoniumComponents()> &
charmoniumComponentNames() {
  static const std::array<const char *, numberOfCharmoniumComponents()> values = {
      "from_b_hadron", "psi_2S", "chi_c1", "chi_c2", "others"};
  return values;
}

inline std::string charmoniumComponentDirectoryName(int component) {
  return "PrivateGen_cat2_" +
         std::string(charmoniumComponentNames().at(component));
}

inline int category(int pdgId) {
  const int absoluteId = std::abs(pdgId);
  if (isBHadron(absoluteId))
    return 1; // b-hadron (nonprompt J/psi)
  if (charmoniumType(absoluteId) >= 0)
    return 2; // charmonium feed-down
  if (absoluteId == 21 || absoluteId == 2212)
    return 3; // gluon or proton
  if (absoluteId >= 1 && absoluteId <= 5)
    return 4; // quark
  if (absoluteId == 9940003)
    return 5; // J/psi[3S1(8)]
  if (absoluteId == 9941003)
    return 6; // J/psi[1S0(8)]
  if (absoluteId == 9942003)
    return 7; // J/psi[3PJ(8)]
  return 0;
}

inline const std::array<const char *, numberOfCategories()> &names() {
  static const std::array<const char *, numberOfCategories()> values = {
      "other",          "b_hadron", "charmonium_feeddown",
      "gluon_or_proton", "quark",    "3S1_octet",
      "1S0_octet",      "3PJ_octet"};
  return values;
}

inline std::string directoryName(int value) {
  return "PrivateGen_cat" + std::to_string(value);
}

inline std::string describe() {
  std::string result = "classification=direct_mother_pdgid";
  for (int value = 0; value < numberOfCategories(); ++value)
    result += ";cat" + std::to_string(value) + "=" + names().at(value);
  return result;
}

} // namespace jpsi_origin

#endif
