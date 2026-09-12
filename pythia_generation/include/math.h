#include <iostream>
#include <vector>
#include <stdexcept> // 包含标准异常类
#include "TLorentzVector.h"
#include "random"

struct spinangles
{
  double phi12, phi13, phi23;
  double z1_1, z2_1, z1_2, z2_2;
  double theta1_1, theta2_1, theta1_2, theta2_2;
};

bool isBetween(double num, std::vector<double> range)
{
  if (range.size() != 2)
    throw std::invalid_argument("Only vectors of length two are supported!");

  // 确保range[0] <= range[1]
  if (range[0] > range[1])
    std::swap(range[0], range[1]);

  // 检查无穷大情况
  if (range[0] == -1) // 仅上界有限
    return num <= range[1];
  else if (range[1] == -1) // 仅下界有限
    return num >= range[0];
  else // 正常情况，两端都有限
    return num >= range[0] && num <= range[1];
}

std::vector<double> computetheta(TLorentzVector vector11, TLorentzVector vector12, TLorentzVector vector2)
{
  TLorentzVector t1, t2, t21, t22, X;
  t1 = vector2;
  t21 = vector11;
  t22 = vector12;
  t2 = t21 + t22;
  X = t1 + t2;
  TVector3 boostp2 = -(t2.BoostVector());
  TVector3 boostX = -(X.BoostVector());
  TLorentzVector t21_p2(t21);
  t21_p2.Boost(boostp2);
  TLorentzVector t22_p2(t22);
  t22_p2.Boost(boostp2);
  TLorentzVector t1_p2(t1);
  t1_p2.Boost(boostp2);
  TLorentzVector t2_pX(t2);
  t2_pX.Boost(boostX);
  TLorentzVector t1_pX(t1);
  t1_pX.Boost(boostX);
  TLorentzVector t21_X(t21);
  t21_X.Boost(boostX);
  TLorentzVector t22_X(t22);
  t22_X.Boost(boostX);
  // theta*

  std::random_device rd;
  std::uniform_int_distribution<int> dist(1, 100000);
  int random_num = dist(rd);
  double theta_, theta2_;
  if (random_num % 2 == 0)
    theta_ = acos(t1_pX.Vect().Dot(X.Vect()) / t1_pX.Vect().Mag() / X.Vect().Mag());
  else
    theta_ = acos(t2_pX.Vect().Dot(X.Vect()) / t2_pX.Vect().Mag() / X.Vect().Mag());
  // theta2
  if (random_num % 2 == 0)
    theta2_ = acos(t1_p2.Vect().Dot(t21_p2.Vect()) / t1_p2.Vect().Mag() / t21_p2.Vect().Mag());
  else
    theta2_ = acos(t1_p2.Vect().Dot(t22_p2.Vect()) / t1_p2.Vect().Mag() / t22_p2.Vect().Mag());

  std::vector<double> temp = {theta_, theta2_};
  return temp;
}

std::vector<double> computephi(TLorentzVector vector11, TLorentzVector vector12, TLorentzVector vector2, bool issubsequent = true)
{
  TLorentzVector vector1 = vector11 + vector12;
  TLorentzVector harder, softer;
  TVector3 plane1, plane2;
  plane1 = vector1.Vect().Cross(vector2.Vect());
  plane2 = vector11.Vect().Cross(vector12.Vect());
  TVector3 plane1_normal(plane1.X() / plane1.Mag(), plane1.Y() / plane1.Mag(),
                         plane1.Z() / plane1.Mag());
  TVector3 plane2_normal(plane2.X() / plane2.Mag(), plane2.Y() / plane2.Mag(),
                         plane2.Z() / plane2.Mag());
  double phicos = abs(plane1_normal.Dot(plane2_normal));
  harder = (vector11.P() > vector12.P()) ? vector11 : vector12;
  softer = (vector11.P() > vector12.P()) ? vector12 : vector11;
  if ((plane1_normal.Cross(plane2_normal)).Dot(harder.Vect()) > 0)
    phicos = -1.0 * phicos;
  double phi = acos(phicos);
  double z1 = vector1.P() / (vector1.P() + vector2.P());
  double z2 = softer.P() / (vector11.P() + vector12.P());
  std::vector<double> temp = {phi, z1, z2};
  return temp;
}

double computephi23(TLorentzVector vector11, TLorentzVector vector12, TLorentzVector vector21, TLorentzVector vector22)
{
  TVector3 plane1, plane2;
  plane1 = vector11.Vect().Cross(vector12.Vect());
  plane2 = vector21.Vect().Cross(vector22.Vect());
  TVector3 plane1_normal(plane1.X() / plane1.Mag(), plane1.Y() / plane1.Mag(),
                         plane1.Z() / plane1.Mag());
  TVector3 plane2_normal(plane2.X() / plane2.Mag(), plane2.Y() / plane2.Mag(),
                         plane2.Z() / plane2.Mag());
  double phicos = abs(plane1_normal.Dot(plane2_normal));
  std::random_device rd;
  std::uniform_int_distribution<int> dist(1, 100000);
  int random_num = dist(rd);
  if (random_num % 2 == 0)
    phicos = -1.0 * phicos;
  double phi = acos(phicos);
  return phi;
}

struct spinangles computeangles(TLorentzVector vector11, TLorentzVector vector12, TLorentzVector vector21, TLorentzVector vector22)
{
  struct spinangles angles;
  TLorentzVector vector1 = vector11 + vector12;
  TLorentzVector vector2 = vector21 + vector22;

  std::vector<double> phis1 = computephi(vector11, vector12, vector2);
  std::vector<double> thetas1 = computetheta(vector11, vector12, vector2);
  std::vector<double> phis2 = computephi(vector21, vector22, vector1);
  std::vector<double> thetas2 = computetheta(vector21, vector22, vector1);
  double phi = computephi23(vector11, vector12, vector21, vector22);

  angles.phi12 = phis1.at(0);
  angles.z1_1 = phis1.at(1);
  angles.z2_1 = phis1.at(2);
  angles.theta1_1 = thetas1.at(0);
  angles.theta2_1 = thetas1.at(1);

  angles.phi13 = phis2.at(0);
  angles.z1_2 = phis2.at(1);
  angles.z2_2 = phis2.at(2);
  angles.theta1_2 = thetas2.at(0);
  angles.theta2_2 = thetas2.at(1);

  angles.phi23 = phi;
  return angles;
}