#ifndef JPSI_MOTHER_CATEGORY_H
#define JPSI_MOTHER_CATEGORY_H

#include <array>
#include <cstdlib>
#include <string>

namespace jpsi_origin {

inline constexpr int numberOfCategories() { return 8; }

inline int category(int pdgId) {
  const int absoluteId = std::abs(pdgId);
  if (absoluteId / 100 % 10 == 5 || absoluteId / 1000 % 10 == 5)
    return 1; // b-hadron (nonprompt J/psi)
  if (absoluteId == 441 || absoluteId == 445 || absoluteId == 100443 ||
      absoluteId == 20443 || absoluteId == 10441 || absoluteId == 30443 ||
      absoluteId == 10443)
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
