/* Copyright 2018 HuaTsai */
#include <sstream>
#include <string>
#include "project1.hpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  task1: " << argv[0]
              << " 1 nx,ny,nz,ox,oy,oz,ax,ay,az,px,py,pz" << std::endl;
    std::cout << "  task2: " << argv[0]
              << " 2 theta1,theta2,theta3,theta4,theta5,theta6" << std::endl;
    return 1;
  }
  if (std::stoi(argv[1]) == 1) {
    std::istringstream ss(argv[2]);
    std::string str;
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    int ridx = 0;
    int cidx = 0;
    while (getline(ss, str, ',')) {
      T(ridx, cidx) = std::stod(str);
      if (++ridx == 3) {
        ridx = 0;
        ++cidx;
      }
    }
    std::cout << "Cartesian Point (n, o, a, p):" << std::endl << std::endl
              << T << std::endl << std::endl << std::endl;
    auto answer = DoTask1(T);
    PrintAnswer(answer);
  } else if (std::stoi(argv[1]) == 2) {
    std::vector<double> theta;
    std::istringstream ss(argv[2]);
    std::string str;
    while (getline(ss, str, ',')) {
      theta.push_back(std::stod(str));
    }
    std::cout << "Joint Variables (theta 1 to 6):" << std::endl << std::endl;
    for (const auto &elem : theta) {
      std::cout << elem << " ";
    }
    std::cout << std::endl << std::endl << std::endl;
    auto T = DoTask2(theta);
    std::cout << "Cartesian Point (n, o, a, p):" << std::endl << std::endl
              << T << std::endl << std::endl << std::endl;
    Eigen::Matrix3d rot = T.block(0, 0, 3, 3);
    Eigen::Vector3d angle = rot.eulerAngles(2, 1, 2);
    std::cout << "Cartesian Point (x, y, z, phi, theta, phi):"
              << std::endl << std::endl
              << T.block(0, 3, 3, 1).transpose() << " " << angle.transpose()
              << std::endl << std::endl;
  }
}
