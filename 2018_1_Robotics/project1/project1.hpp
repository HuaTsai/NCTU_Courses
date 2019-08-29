/* Copyright 2018 HuaTsai */
#ifndef PROJECT1_PROJECT1_HPP_
#define PROJECT1_PROJECT1_HPP_
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <vector>

constexpr double A1 = 0.12;
constexpr double A2 = 0.25;
constexpr double A3 = 0.26;

bool IsValidRange(const double &theta, const int &idx) {
  if (theta == theta) {
    double theta_deg = theta * 180 / M_PI;
    if ((idx == 1 && theta_deg >= -150 && theta_deg <= 150) ||
        (idx == 2 && theta_deg >= -30 && theta_deg <= 100) ||
        (idx == 3 && theta_deg >= -120 && theta_deg <= 0) ||
        (idx == 4 && theta_deg >= -110 && theta_deg <= 110) ||
        (idx == 5 && theta_deg >= -180 && theta_deg <= 180) ||
        (idx == 6 && theta_deg >= -180 && theta_deg <= 180)) {
      return true;
    }
  }
  return false;
}

Eigen::Matrix<double, 6, 1> MakeVector6(
    double v0, double v1, double v2, double v3, double v4, double v5) {
  Eigen::Matrix<double, 6, 1> ret;
  ret << v0, v1, v2, v3, v4, v5;
  return ret;
}

Eigen::Matrix4d MakeA(double d, double a, double alpha, double theta) {
  // An = Rot(theta, z) * Trans(0, 0, d) * Trans(a, 0, d) * Rot(alpha, x)
  Eigen::AngleAxisd r1(theta * M_PI / 180, Eigen::Vector3d::UnitZ());
  Eigen::Translation3d t1(0, 0, d);
  Eigen::Translation3d t2(a, 0, 0);
  Eigen::AngleAxisd r2(alpha * M_PI / 180, Eigen::Vector3d::UnitX());
  return (r1 * t1 * t2 * r2).matrix();
}

Eigen::Matrix4d DoTask2(const std::vector<double> &theta) {
  auto a1 = MakeA(0, A1, -90, theta.at(0));
  auto a2 = MakeA(0, A2, 0, theta.at(1));
  auto a3 = MakeA(0, A3, 0, theta.at(2));
  auto a4 = MakeA(0, 0, -90, theta.at(3));
  auto a5 = MakeA(0, 0, 90, theta.at(4));
  auto a6 = MakeA(0, 0, 0, theta.at(5));
  auto T = a1 * a2 * a3 * a4 * a5 * a6;
  return T;
}

std::vector<Eigen::Matrix<double, 6, 1>> DoTask1(const Eigen::Matrix4d &T) {
  std::vector<Eigen::Matrix<double, 6, 1>> answer;
  std::vector<Eigen::Matrix<double, 6, 1>> tmp_answer;

  Eigen::Vector3d n = T.block(0, 0, 3, 1);
  Eigen::Vector3d o = T.block(0, 1, 3, 1);
  Eigen::Vector3d a = T.block(0, 2, 3, 1);
  Eigen::Vector3d p = T.block(0, 3, 3, 1);

  // Compute theta1
  std::vector<double> th1;
  th1.push_back(atan2(p.y(), p.x()));
  th1.push_back(atan2(-p.y(), -p.x()));
  for (const auto &theta : th1) {
    if (IsValidRange(theta, 1)) {
      answer.push_back(MakeVector6(theta, 0, 0, 0, 0, 0));
    }
  }

  // Compute theta3
  tmp_answer.clear();
  for (const auto &elem : answer) {
    std::vector<double> th3;
    double th1 = elem(0);
    double num3 = pow(cos(th1) * p.x() + sin(th1) * p.y() - A1, 2) +
                  pow(p.z(), 2) - pow(A2, 2) - pow(A3, 2);
    double den3 = 2 * A2 * A3;
    th3.push_back(acos(num3 / den3));
    th3.push_back(-acos(num3 / den3));
    for (const auto &theta : th3) {
      if (IsValidRange(theta, 3)) {
        tmp_answer.push_back(MakeVector6(th1, 0, theta, 0, 0, 0));
      }
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  // Compute theta2
  for (const auto &elem : answer) {
    std::vector<double> th2;
    double th1 = elem(0);
    double th3 = elem(2);

    double phith2num = -A3 * sin(th3);
    double phith2den = A2 + A3 * cos(th3);
    double phith2_1 = atan2(phith2num, phith2den);
    double phith2_2 = atan2(-phith2num, -phith2den);

    double phinum = p.z();
    double phiden = cos(th1) * p.x() + sin(th1) * p.y() - A1;
    double phi_1 = atan2(phinum, phiden);
    double phi_2 = atan2(-phinum, -phiden);

    th2.push_back(phith2_1 - phi_1);
    th2.push_back(phith2_1 - phi_2);
    // th2.push_back(phith2_2 - phi_1);
    // th2.push_back(phith2_2 - phi_2);
    std::unique(th2.begin(), th2.end());

    for (const auto &theta : th2) {
      if (IsValidRange(theta, 2)) {
        tmp_answer.push_back(MakeVector6(th1, theta, th3, 0, 0, 0));
      }
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  // Compute theta4
  for (const auto &elem : answer) {
    std::vector<double> th4;
    double th1 = elem(0);
    double th2 = elem(1);
    double th3 = elem(2);
    double num4 = a.z() * cos(th2 + th3) +
                  (cos(th1) * a.x() + sin(th1) * a.y()) * sin(th2 + th3);
    double den4 = a.z() * sin(th2 + th3) -
                  (cos(th1) * a.x() + sin(th1) * a.y()) * cos(th2 + th3);
    th4.push_back(atan2(num4, den4));
    th4.push_back(atan2(-num4, -den4));
    for (const auto &theta : th4) {
      if (IsValidRange(theta, 4)) {
        tmp_answer.push_back(MakeVector6(th1, th2, th3, theta, 0, 0));
      }
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  // Compute theta5
  for (const auto &elem : answer) {
    std::vector<double> th5;
    double th1 = elem(0);
    double th2 = elem(1);
    double th3 = elem(2);
    double th4 = elem(3);
    double num5 = (cos(th1) * a.x() + sin(th1) * a.y()) * cos(th2 + th3 + th4) -
                  a.z() * sin(th2 + th3 + th4);
    double den5 = cos(th1) * a.y() - sin(th1) * a.x();
    th5.push_back(atan2(num5, den5));
    th5.push_back(atan2(-num5, -den5));
    for (const auto &theta : th5) {
      if (IsValidRange(theta, 5)) {
        tmp_answer.push_back(MakeVector6(th1, th2, th3, th4, theta, 0));
      }
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  // Compute theta6
  for (const auto &elem : answer) {
    std::vector<double> th6;
    double th1 = elem(0);
    double th2 = elem(1);
    double th3 = elem(2);
    double th4 = elem(3);
    double th5 = elem(4);

    double num6 = sin(th1) * o.x() - cos(th1) * o.y();
    double den6 = -sin(th1) * n.x() + cos(th1) * n.y();
    th6.push_back(atan2(num6, den6));
    th6.push_back(atan2(-num6, -den6));
    for (const auto &theta : th6) {
      if (IsValidRange(theta, 6)) {
        tmp_answer.push_back(MakeVector6(th1, th2, th3, th4, th5, theta));
      }
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  // Check answer validity
  for (const auto &v : answer) {
    auto v_deg = v * 180 / M_PI;
    std::vector<double> v2 = {v_deg(0), v_deg(1), v_deg(2),
                              v_deg(3), v_deg(4), v_deg(5)};
    auto T2 = DoTask2(v2);
    bool valid = true;
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (abs(T(i, j) - T2(i, j)) > 0.005) {
          valid = false;
          break;
        }
      }
    }
    if (valid) {
      tmp_answer.push_back(v);
    }
  }
  answer = tmp_answer;
  tmp_answer.clear();

  return answer;
}

void PrintAnswer(const std::vector<Eigen::Matrix<double, 6, 1>> &ans) {
  int idx = 0;
  std::cout << "Joint Variables (theta 1 to 6):" << std::endl << std::endl;
  for (const auto &elem : ans) {
    auto sol = elem * 180 / M_PI;
    std::cout << "sol" << ++idx << ": " << std::left
              << std::setw(10) << sol(0)
              << std::setw(10) << sol(1)
              << std::setw(10) << sol(2)
              << std::setw(10) << sol(3)
              << std::setw(10) << sol(4)
              << std::setw(10) << sol(5)
              << std::endl;
  }
  std::cout << std::endl;
}

#endif  // PROJECT1_PROJECT1_HPP_
